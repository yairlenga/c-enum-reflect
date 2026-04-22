#!/usr/bin/env python3
from __future__ import annotations

import argparse
import io
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Set, Tuple

from elftools.elf.elffile import ELFFile
from elftools.dwarf.die import DIE

REQ_PREFIX = "enum_req_"
TYPE_PREFIX = "enum_type_"

@dataclass
class EnumItem:
    name: str
    value: int


@dataclass
class EnumMeta:
    symbol: Optional[str] = None
    anchor: Optional[str] = None
    name: Optional[str] = None
    byte_size: Optional[int] = None
    count: Optional[int] = None
    decl_file: Optional[str] = None
    decl_line: Optional[int] = None


@dataclass
class EnumBlock:
    section_id: str
    items: List[EnumItem] = field(default_factory=list)
    meta: EnumMeta = field(default_factory=EnumMeta)


class DwarfEnumExportError(Exception):
    pass


def decode_attr(value) -> Optional[str]:
    if value is None:
        return None
    if isinstance(value, bytes):
        return value.decode("utf-8", errors="replace")
    return str(value)


def toml_quote(text: str) -> str:
    escaped = (
        text.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\b", "\\b")
        .replace("\t", "\\t")
        .replace("\n", "\\n")
        .replace("\f", "\\f")
        .replace("\r", "\\r")
    )
    return f'"{escaped}"'


def is_identifier(text: str) -> bool:
    return re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", text) is not None


class DwarfEnumExporter:
    def __init__(self, path: Path, strict: bool = False, include_decl: bool = False):
        self.path = path
        self.strict = strict
        self.include_decl = include_decl
        self._warnings: List[str] = []

    def warn(self, msg: str) -> None:
        self._warnings.append(msg)

    def load(self) -> List[EnumBlock]:
        with self.path.open("rb") as fp:
            elf = ELFFile(fp)
            if not elf.has_dwarf_info():
                raise DwarfEnumExportError(f"{self.path}: no DWARF information found")

            dwarfinfo = elf.get_dwarf_info()
            exported_descs = self._find_export_requests(elf)
            if not exported_descs:
                return []

            variable_dies = self._index_variable_dies(dwarfinfo)

            blocks: List[EnumBlock] = []
            for desc_symbol in sorted(exported_descs):
                suffix = desc_symbol[len(REQ_PREFIX):]
                anchor_symbol = TYPE_PREFIX + suffix

                if anchor_symbol not in variable_dies:
                    msg = (
                        f"{self.path}: found {desc_symbol}, but no DWARF variable DIE "
                        f"for matching anchor {anchor_symbol}"
                    )
                    if self.strict:
                        raise DwarfEnumExportError(msg)
                    self.warn(msg)
                    continue

                anchor_die, cu = variable_dies[anchor_symbol]
                enum_die = self._resolve_variable_to_enum(anchor_die)
                if enum_die is None:
                    msg = (
                        f"{self.path}: {anchor_symbol} does not resolve to an "
                        f"enumeration type"
                    )
                    if self.strict:
                        raise DwarfEnumExportError(msg)
                    self.warn(msg)
                    continue

                block = self._extract_enum_block(
                    desc_symbol=desc_symbol,
                    anchor_symbol=anchor_symbol,
                    suffix=suffix,
                    enum_die=enum_die,
                    cu=cu,
                )
                blocks.append(block)

            return blocks

    def _find_export_requests(self, elf: ELFFile) -> Set[str]:
        """
        Find global or external symbols named enum_desc_*.
        Works for object files and linked ELF images.
        """
        results: Set[str] = set()

        for sec_name in (".symtab", ".dynsym"):
            section = elf.get_section_by_name(sec_name)
            if section is None:
                continue

            for sym in section.iter_symbols():
                name = sym.name
                if not name or not name.startswith(REQ_PREFIX):
                    continue

                info = sym["st_info"]
                bind = info["bind"]
                typ = info["type"]

                # Only consider object-ish globals.
                if bind not in ("STB_LOCAL"):
                    continue
                if typ not in ("STT_OBJECT", "STT_NOTYPE"):
                    continue

                suffix = name[len(REQ_PREFIX):]
                if not suffix:
                    continue
                results.add(name)

        return results

    def _index_variable_dies(self, dwarfinfo) -> Dict[str, Tuple[DIE, object]]:
        """
        Build name -> (variable DIE, CU) for global/static variables.
        """
        index: Dict[str, Tuple[DIE, object]] = {}

        for cu in dwarfinfo.iter_CUs():
            top = cu.get_top_DIE()
            for die in self._walk_die_tree(top):
                if die.tag != "DW_TAG_variable":
                    continue

                name_attr = die.attributes.get("DW_AT_name")
                if name_attr is None:
                    continue

                name = decode_attr(name_attr.value)
                if not name:
                    continue

                if name in index:
                    # Keep first, warn on duplicates.
                    self.warn(
                        f"{self.path}: duplicate variable DIE for {name}; using first match"
                    )
                    continue

                index[name] = (die, cu)

        return index

    def _walk_die_tree(self, die: DIE) -> Iterable[DIE]:
        yield die
        for child in die.iter_children():
            yield from self._walk_die_tree(child)

    def _follow_type_ref(self, die: DIE) -> Optional[DIE]:
        """
        Follow DW_AT_type from a DIE to the referenced DIE.
        """
        type_attr = die.attributes.get("DW_AT_type")
        if type_attr is None:
            return None
        return die.get_DIE_from_attribute("DW_AT_type")

    def _resolve_variable_to_enum(self, var_die: DIE) -> Optional[DIE]:
        """
        From DW_TAG_variable, unwrap qualifiers/typedefs until we hit
        DW_TAG_enumeration_type or something else.
        """
        die = self._follow_type_ref(var_die)
        seen: Set[int] = set()

        passthrough_tags = {
            "DW_TAG_const_type",
            "DW_TAG_volatile_type",
            "DW_TAG_restrict_type",
            "DW_TAG_typedef",
            "DW_TAG_atomic_type",
        }

        while die is not None:
            offset = die.offset
            if offset in seen:
                break
            seen.add(offset)

            if die.tag == "DW_TAG_enumeration_type":
                return die

            if die.tag in passthrough_tags:
                die = self._follow_type_ref(die)
                continue

            return None

        return None

    def _extract_enum_block(
        self,
        desc_symbol: str,
        anchor_symbol: str,
        suffix: str,
        enum_die: DIE,
        cu,
    ) -> EnumBlock:
        section_id = suffix
        meta = EnumMeta(
            symbol=desc_symbol,
            anchor=anchor_symbol,
            name=decode_attr(enum_die.attributes.get("DW_AT_name").value)
            if enum_die.attributes.get("DW_AT_name") is not None
            else None,
            byte_size=self._as_int(enum_die.attributes.get("DW_AT_byte_size")),
        )

        if self.include_decl:
            meta.decl_file = self._resolve_decl_file(cu, enum_die)
            meta.decl_line = self._as_int(enum_die.attributes.get("DW_AT_decl_line"))

        items: List[EnumItem] = []
        for child in enum_die.iter_children():
            if child.tag != "DW_TAG_enumerator":
                continue

            name_attr = child.attributes.get("DW_AT_name")
            value_attr = child.attributes.get("DW_AT_const_value")
            if name_attr is None or value_attr is None:
                continue

            item_name = decode_attr(name_attr.value)
            if not item_name:
                continue

            if not is_identifier(item_name):
                msg = (
                    f"{self.path}: enumerator {item_name!r} in {anchor_symbol} is not "
                    f"a C identifier; cannot emit bare-key enum-desc-v1"
                )
                if self.strict:
                    raise DwarfEnumExportError(msg)
                self.warn(msg)
                continue

            value = self._as_int(value_attr)
            if value is None:
                continue

            items.append(EnumItem(name=item_name, value=value))

        meta.count = len(items)
        return EnumBlock(section_id=section_id, items=items, meta=meta)

    def _as_int(self, attr) -> Optional[int]:
        if attr is None:
            return None
        return int(attr.value)

    def _resolve_decl_file(self, cu, die: DIE) -> Optional[str]:
        file_attr = die.attributes.get("DW_AT_decl_file")
        if file_attr is None:
            return None

        try:
            lp = cu.dwarfinfo.line_program_for_CU(cu)
            if lp is None:
                return None
            file_entries = lp.header.file_entry
            idx = int(file_attr.value) - 1
            if idx < 0 or idx >= len(file_entries):
                return None
            return decode_attr(file_entries[idx].name)
        except Exception:
            return None

    def print_warnings(self, err: io.TextIOBase) -> None:
        for msg in self._warnings:
            print(f"warning: {msg}", file=err)


class BaseEmitter:
    def __init__(self, args):
        self.args = args

    def emit(self, blocks: List[EnumBlock], out: io.TextIOBase) -> None:
        raise NotImplementedError
   
    def _build_c_str_parts(self, block: EnumBlock) -> tuple[list[str], list[int]]:
        """
        Build:
        parts   = [enum_name, label1, label2, ...]
        offsets = offsets in strs blob for each label (not enum name)

        For empty enums:
        parts   = [enum_name, ""]
        offsets = [len(enum_name) + 1]
        """
        enum_name = block.section_id
        parts = [enum_name]
        offsets = []

        cur = len(enum_name) + 1   # skip enum_name + '\0'

        if not block.items:
            parts.append("")
            offsets.append(cur)
            return parts, offsets

        for item in block.items:
            offsets.append(cur)
            parts.append(item.name)
            cur += len(item.name) + 1

        return parts, offsets

class TextEmitter(BaseEmitter):
    def _str_blob_size(self, block: EnumBlock) -> int:
        parts, _ = self._build_c_str_parts(block)
        return sum(len(p) + 1 for p in parts) + 8

    def _wrap_enum_items(
        self,
        items: List[EnumItem],
        indent: str = "    ",
        width: int = 72,
    ) -> List[str]:
        """
        Format:
            NAME = value, NAME2 = value2, ...
        wrapped to roughly `width` chars total per line.
        """
        if not items:
            return [indent + "(no members)"]

        tokens = [f"{item.name} = {item.value}" for item in items]
        lines: List[str] = []
        cur = indent

        for tok in tokens:
            sep = "" if cur == indent else ", "
            if len(cur) + len(sep) + len(tok) > width and cur != indent:
                lines.append(cur)
                cur = indent + tok
            else:
                cur += sep + tok

        if cur != indent:
            lines.append(cur)

        return lines

    def emit(self, blocks: List[EnumBlock], out: io.TextIOBase) -> None:
        """
        Emit enums in a simple human-readable text format.
        """
        for i, block in enumerate(blocks):
            values = [item.value for item in block.items]
            count = len(block.items)
            min_value = min(values) if values else None
            max_value = max(values) if values else None
            blob_size = self._str_blob_size(block)

            attrs: List[str] = [f"count={count}"]

            if block.meta.symbol:
                attrs.append(f"symbol={block.meta.symbol}")
            if block.meta.anchor:
                attrs.append(f"anchor={block.meta.anchor}")
            if min_value is not None:
                attrs.append(f"min={min_value}")
            if max_value is not None:
                attrs.append(f"max={max_value}")
            attrs.append(f"bytes={blob_size}")

            out.write(f"Enum: {block.section_id} ({', '.join(attrs)})\n")

            for line in self._wrap_enum_items(block.items, indent="    ", width=72):
                out.write(line + "\n")

            if i + 1 != len(blocks):
                out.write("\n")
        ...

class TomlEmitter(BaseEmitter):




    def emit(self, blocks: List[EnumBlock], out: io.TextIOBase) -> None:
        out.write('format = "enum-desc-v1"\n')

        include_decl = self.args.include_decl

        if blocks:
            out.write("\n")

        for i, block in enumerate(blocks):
            out.write(f"[enum.{block.section_id}]\n")

            # Required / useful metadata. Meta stays optional by line, but count is helpful.
            meta_lines: List[Tuple[str, object]] = []
            if block.meta.count is not None:
                meta_lines.append(("count", block.meta.count))
            if block.meta.symbol:
                meta_lines.append(("symbol", block.meta.symbol))
            if block.meta.anchor:
                meta_lines.append(("anchor", block.meta.anchor))
            if block.meta.name:
                meta_lines.append(("name", block.meta.name))
            if block.meta.byte_size is not None:
                meta_lines.append(("byte_size", block.meta.byte_size))
            if include_decl and block.meta.decl_file:
                meta_lines.append(("decl_file", block.meta.decl_file))
            if include_decl and block.meta.decl_line is not None:
                meta_lines.append(("decl_line", block.meta.decl_line))

            for key, value in meta_lines:
                if isinstance(value, str):
                    out.write(f"_.{key} = {toml_quote(value)}\n")
                else:
                    out.write(f"_.{key} = {value}\n")

            if meta_lines and block.items:
                out.write("\n")

            for item in block.items:
                out.write(f"{item.name} = {item.value}\n")

            if i + 1 != len(blocks):
                out.write("\n")


class CEmitter(BaseEmitter):


    def _emit_c_block(self, block: EnumBlock, out: io.TextIOBase) -> None:
        """
        Emit one enum block as C globals + exported accessor function.
        Assumes block.section_id and item names are valid C identifiers / safe text.
        """
        sym = f"enum_desc_{block.section_id}"
        parts, offsets = self._build_c_str_parts(block)
        count = len(block.items)

        out.write(f"static const enum_desc_val {sym}_values[] = {{\n")
        if count:
            for item in block.items:
                out.write(f"    {item.value}, /* {item.name} */\n")
        else:
            out.write("    0 /* empty */\n")
        out.write("};\n\n")

        out.write(f"static const uint16_t {sym}_lbl_off[] = {{\n")
        if count:
            for off, item in zip(offsets, block.items):
                out.write(f"    {off}, /* {item.name} */\n")
        else:
            out.write(f"    {offsets[0]}, /* empty */\n")
        out.write("};\n\n")

        out.write(f"static const char {sym}_strs[] =\n")
        for p in parts:
            out.write(f'    "{p}\\0"\n')
        out.write('    "\\0\\0\\0\\0\\0\\0\\0\\0";\n\n')

        out.write(f"static const struct enum_desc {sym}_obj = {{\n")
        out.write(f"    .value_count = {count},\n")
        out.write("    .flags = 0,\n")
        out.write(f"    .values = {sym}_values,\n")
        out.write(f"    .lbl_off = {sym}_lbl_off,\n")
        out.write("    .meta = NULL,\n")
        out.write("    .ext = NULL,\n")
        out.write(f"    .strs = {sym}_strs,\n")
        out.write("};\n\n")

        out.write(f"const struct enum_desc *{sym}(void)\n")
        out.write("{\n")
        out.write(f"    return &{sym}_obj;\n")
        out.write("}\n\n")

        out.write(f"const char *enum_desc_label_of_{block.section_id}(int value)\n")
        out.write("{\n")
        out.write(f"    return enum_desc_label_of(&{sym}_obj, value) ;\n")
        out.write("}\n\n")



    def emit(self, blocks: List[EnumBlock], out: io.TextIOBase) -> None:
        """
        Emit all blocks as a C source file.
        """
        out.write("/* auto-generated by enum_dwarf_query.py */\n")
        out.write("#include <stdint.h>\n")
        out.write('#include "enum_desc_def.h"\n\n')

        for i, block in enumerate(blocks):
            self._emit_c_block(block, out)
            if i + 1 != len(blocks):
                out.write("\n")


def build_arg_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        description=(
            "Extract opted-in enum metadata from DWARF and emit enum-desc-v1 TOML."
        )
    )
    p.add_argument(
        "elf",
        type=Path,
        help="ELF object, executable, or shared library compiled with DWARF info",
    )
    p.add_argument(
        "--format",
        choices=("text", "toml", "c"),
        default="text",
        help="Output format",
    )   
    p.add_argument(
        "--strict",
        action="store_true",
        help="Treat missing anchors / bad types / invalid enumerators as errors",
    )
    p.add_argument(
        "--include-decl",
        action="store_true",
        help="Emit optional _.decl_file / _.decl_line when available",
    )
    p.add_argument(
        "--quiet-warnings",
        action="store_true",
        help="Suppress non-fatal warnings on stderr",
    )
    return p


def main(argv: Optional[List[str]] = None) -> int:
    args = build_arg_parser().parse_args(argv)

    try:
        exporter = DwarfEnumExporter(
            path=args.elf,
            strict=args.strict,
            include_decl=args.include_decl,
        )
        blocks = exporter.load()
        if args.format == "c":
            emitter = CEmitter(args)
        elif args.format == "toml":
            emitter = TomlEmitter(args)
        else:
            emitter = TextEmitter(args)

        emitter.emit(blocks, sys.stdout)        

        if not args.quiet_warnings:
            exporter.print_warnings(sys.stderr)

        return 0

    except BrokenPipeError:
        return 0
    except DwarfEnumExportError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    except FileNotFoundError:
        print(f"error: file not found: {args.elf}", file=sys.stderr)
        return 2
    except Exception as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2

if __name__ == "__main__":
    raise SystemExit(main())