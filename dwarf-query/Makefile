default: all

all: test1_enum.exe

CFLAGS = -g
CPPFLAGS = -I../api/include

test1_enum.o: test1_enum_currency.c

test1.enum.txt: test1_enum_currency.o enum_dwarf_query.py
	./enum_dwarf_query.py $< > $@.tmp
	mv $@.tmp $@

test1_enum.exe: test1_enum_currency.o
	$(LINK.c) $^ -o $@ $(LDFLAGS)

test: test1.enum.txt

clean:
	rm -f *.tmp *.enum.txt *.o *.exe