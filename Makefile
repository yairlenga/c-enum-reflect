default: t

CFLAGS = -g -Wall -Werror

TESTS = t_enum_refl t_enum_desc
PLUGINS = gcc_enum_reflect.so
MKFILE = Makefile


.PHONY: all clean test plugins
all: $(PLUGINS) $(TESTS)
test: $(TESTS)
plugins: $(PLUGINS)
t: t_gcc

enum_reflect.o: enum_desc.h enum_refl.h enum_desc_def.h $(MKFILE)

t_enum_desc.o: enum_desc.h enum_refl.h enum_desc_def.h $(MKFILE)
t_enum_refl.o: enum_desc.h enum_refl.h enum_desc_def.h $(MKFILE)

t_enum_desc: enum_reflect.o

t_enum_refl: enum_reflect.o

# Build GCC Plugin
gcc_enum_reflect.so: gcc_enum_reflect.cc $(MKFILE)
	gcc $(CFLAGS) -fno-rtti -fno-exceptions -shared -fPIC -o $@ $< -I$$(gcc -print-file-name=plugin)/include

t_gcc.o: t_gcc.c enum_desc_def.h $(PLUGINS)
	gcc $(CFLAGS) -c -fplugin=./gcc_enum_reflect.so $< -o $@

t_gcc: t_gcc.o enum_reflect.o $(PLUGINS) $(MKFILE)
	gcc $(CFLAGS) -fplugin=./gcc_enum_reflect.so $< enum_reflect.o -o $@

clean:
	rm -f *.o $(TESTS) $(PLUGINS)
