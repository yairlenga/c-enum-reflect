default: all

B = build
T = tests
BIN = $B/enum_reflect.exe
LIB = $B/enum_reflect.a
TESTS = test1 t_enum_desc t_enum_refl

vpath %.h include
vpath %.c src tests

setup:
	mkdir -p $B

CC = gcc -std=c2x
CFLAGS = -g -Og -Werror -Wall -Wextra
CPPFLAGS = -Iinclude -D_DEFAULT_SOURCE

SRCS = enum_reflect.c
OBJS = $(SRCS:%.c=$B/%.o)

LIB_SRCS = enum_desc_reader.c enum_desc_repo.c enum_refl.c enum_desc.c
LIB_OBJS = $(LIB_SRCS:%.c=$B/%.o)

$B/%.o: %.c
	$(COMPILE.c) -o $@ $<

$B/%.exe: $B/%.o $(LIB)
	$(LINK.c) -o $@ $< $(LIB)

$(LIB): $(LIB_OBJS)
	rm -f $@
	ar cq $@ $^

$(BIN): $(OBJS) $(LIB)
	$(LINK.c) -o $@ $(OBJS) $(LIB)

all: $(BIN)

test: $(TESTS:%=%.t)

$T/test1.result: ARG=$T/sample.enum.txt
$T/test1.result: $(BIN) $T/sample.enum.txt
	$(BIN) < $(ARG) > $@

$(TESTS:%=$T/%.result):

$T/%.result: $B/%.exe
	$< > $@

%.t: $T/%.result $T/%.gold
	diff $T/$*.result $T/$*.gold

$B/t_enum_desc.exe: $B/t_enum_desc.o $(LIB)
	$(LINK.c) -o $@ $^ $(LIB)

$B/t_enum_refl.exe: $B/t_enum_refl.o $(LIB)
	gcc $(CFLAGS) -o $@ $^ $(LIB)

$B/t_enum_desc.o: enum_desc.h enum_refl.h enum_desc_def.h
$B/t_enum_refl.o: enum_desc.h enum_refl.h enum_desc_def.h

clean:
	rm -f $B/*.o $B/*.exe $(LIB) $T/*.result

realclean: clean
	@case "/$B/" in */build/*) sh -x -c "rm -rf $B" ;; esac
