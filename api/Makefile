default: all

B = build
BIN = $B/enum_reflect.exe
LIB = $B/enum_reflect.a

all: $(BIN)

setup:
	mkdir -p $B

CC = gcc -std=c2x
CFLAGS = -g -Og -Werror -Wall -Wextra
CPPFLAGS = -Iinclude -D_DEFAULT_SOURCE

SRCS = enum_reflect.c
OBJS = $(SRCS:%.c=$B/%.o)

LIB_SRCS = enum_desc_reader.c enum_desc_repo.c enum_refl.c
LIB_OBJS = $(LIB_SRCS:%.c=$B/%.o)

$B/%.o: src/%.c
	$(COMPILE.c) -o $@ $<

$(LIB): $(LIB_OBJS)
	rm -f $@
	ar cq $@ $^

$(BIN): $(OBJS) $(LIB)
	$(LINK.c) -o $@ $(OBJS) $(LIB)

clean:
	rm -f $B/*.o $(BIN) $(LIB)

realclean: clean
	@case "/$B/" in */build/*) sh -x -c "rm -rf $B" ;; esac
