default: all

all: $(EXE)

B = ../build/dwarf-query

EXE = $B/enum_dwarf_query
CFLAGS = -g
CPPFLAGS = -I../api/include

$B:
	mkdir -p $B

$B/%.o: %.c
	$(COMPILE.c) -o $@ $<

$B/%: %.py
	python3 -m py_compile $<
	install -m 0755 $< $@

$B/test1.exe: test1.c $B/test_enum_country2.o $B/test_enum_country3.o $B/test_enum_currency.o
	$(LINK.c) -o $@ $^

test1.txt: $B/test1.exe $(EXE)
	$(EXE) $< > $@.tmp
	mv $@.tmp $@

.PHONY: setup

setup: | $B 
	# python3 -m pip install pyelftools

all: $(EXE) $B/test1.exe

test: test1.txt test1.gold
	diff test1.txt test1.gold

clean:
	rm -f *.tmp *.enum.txt $B/*.o $B/*.exe

realclean: clean
	rm -f __pycache__
	@case "/$B/" in */build/*) sh -x -c "rm -rf $B" ;; esac
