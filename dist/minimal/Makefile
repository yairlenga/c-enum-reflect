default: all

ENUMDESC_DIR = .
B = .
TESTS = test_color test_country

PROGRAMS = $(TESTS:%=$B/%.exe)

setup:

all: $(PROGRAMS)

CFLAGS = -g -I $(ENUMDESC_DIR) -Wall -Werror
OBJS = $(SRCS:%.c=$(B)/%.o)

$(B):
	mkdir -p $(B)

$(B)/%: | $(B)

$(B)/gen_%.o: $(B)/%.o
	$(ENUMDESC_DIR)/enum_dwarf_query --format=c $< > $(B)/gen_$*.c
	$(COMPILE.c) -o $@ $(B)/gen_$*.c

$(B)/enum_desc.o: $(ENUMDESC_DIR)/enum_desc.c
	$(COMPILE.c) -o $@ $^

$(OBJS) $(ENUMDESC_OBJS): $(ENUMDESC_DIR)/enum_desc.h

test_color.exe: $(addprefix $(B)/, test_color.o enum_desc.o gen_test_color.o)
	$(LINK.c) -o $@ $(filter %.o, $^)

test_country.exe: $(addprefix $(B)/, test_country.o enum_desc.o gen_test_country.o)
	$(LINK.c) -o $@ $(filter %.o, $^)


clean:
	rm -f $(B)/*.o $(PROGRAMS) $(B)/gen_*.c $(B)/*.exe test.result

test.result: $(PROGRAMS)
	echo "=== test_color" > $@.new
	./test_color.exe >> $@.new
	echo "=== test_country" >> $@.new
	./test_country.exe 840 826 392 250 276 >> $@.new
	mv $@.new $@

test: test.result test.gold
	diff test.result test.gold