default: all

all: $(EXE)

B = ../build/dwarf-query

EXE = ./enum_dwarf_query
CFLAGS = -g
CPPFLAGS = -I../api/include

$B:
	mkdir -p $B

$B/%.o: %.c
	$(COMPILE.c) -o $@ $<

%: %.py
	python3 -m py_compile $<
	install -m 0755 $< $@


$B/test1.exe: test1.c $B/test_enum_country2.o $B/test_enum_country3.o $B/test_enum_currency.o
	$(LINK.c) -o $@ $^

test1.txt: $B/test1.exe $(EXE)
	$(EXE) $< > $@.tmp
	mv $@.tmp $@

.PHONY: setup
setup: | $B 

all: $(EXE) $B/test1.exe

test: test1.txt

clean:
	rm -f *.tmp *.enum.txt $B/*.o $B/*.exe

realclean: clean
	@case "/$B/" in */build/*) sh -x -c "rm -rf $B" ;; esac
