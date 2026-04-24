default: all

B = build
T = tests
BIN = $B/enum_reflect.exe
LIB = $B/enum_reflect.a

setup:
	mkdir -p $B

CC = gcc -std=c2x
CFLAGS = -g -Og -Werror -Wall -Wextra
CPPFLAGS = -Iinclude -D_DEFAULT_SOURCE

SRCS = enum_reflect.c
OBJS = $(SRCS:%.c=$B/%.o)

LIB_SRCS = enum_desc_reader.c enum_desc_repo.c enum_refl.c enum_desc.c
LIB_OBJS = $(LIB_SRCS:%.c=$B/%.o)

$B/%.o: src/%.c
	$(COMPILE.c) -o $@ $<

$(LIB): $(LIB_OBJS)
	rm -f $@
	ar cq $@ $^

$(BIN): $(OBJS) $(LIB)
	$(LINK.c) -o $@ $(OBJS) $(LIB)

all: $(BIN)

test: test1.t

$T/test1.result: $T/sample.enum.txt $(BIN)
	$(BIN) < $< > $@

%.t: $T/%.result $T/%.gold
	diff $< $T/$*.gold


clean:
	rm -f $B/*.o $(BIN) $(LIB) $T/*.result

realclean: clean
	@case "/$B/" in */build/*) sh -x -c "rm -rf $B" ;; esac
