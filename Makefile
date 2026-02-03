default: t

CFLAGS = -g -Wall -Werror

B = build
TESTS = $B/t_enum_refl $B/t_enum_desc
PLUGINS = $B/gcc_enum_reflect.so
MKFILE = Makefile

vpath %.o $B
vpath %.c .
vpath %.cc .

.PHONY: all clean test plugins
all: $B $(PLUGINS) $(TESTS)
test: $B $(TESTS)
plugins: $(PLUGINS)
t: $B $B/t_gcc

%.o: %.c

$B/enum_reflect.o: enum_reflect.c enum_desc.h enum_refl.h enum_desc_def.h $(MKFILE)
	$(COMPILE.c) $< -o $@

$B/t_enum_desc.o: enum_desc.h enum_refl.h enum_desc_def.h $(MKFILE)
$B/t_enum_refl.o: enum_desc.h enum_refl.h enum_desc_def.h $(MKFILE)

$B/t_enum_desc: enum_reflect.o

$B/t_enum_refl: enum_reflect.o

$B:
	mkdir -p $B

# Build GCC Plugin
$B/gcc_enum_reflect.so: gcc_enum_reflect.cc $(MKFILE)
	gcc $(CFLAGS) -fno-rtti -fno-exceptions -shared -fPIC -o $@ $< -I$$(gcc -print-file-name=plugin)/include

$B/t_gcc.o: t_gcc.c enum_desc_def.h $(PLUGINS)
	gcc $(CFLAGS) -c -fplugin=$B/gcc_enum_reflect.so $< -o $@

$B/t_gcc: $B/t_gcc.o $B/enum_reflect.o $(PLUGINS) $(MKFILE)
	gcc $(CFLAGS) $< $B/enum_reflect.o -o $@

$B/%.o: %.c $(MKFILE)
	gcc $(CFLAGS) -c -o $@ $<

$B/t_enum_desc: $B/t_enum_desc.o $B/enum_reflect.o $(MKFILE)
	gcc $(CFLAGS) $< $B/enum_reflect.o -o $@

$B/t_enum_refl: $B/t_enum_refl.o $B/enum_reflect.o $(MKFILE)
	gcc $(CFLAGS) $< $B/enum_reflect.o -o $@

clean:
	rm -f *.o $(TESTS) $(PLUGINS)

