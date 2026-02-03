default: all

CFLAGS = -g -Wall -Werror -Iinclude

B = build
S = src
T = tests
TESTS = t_enum_refl t_enum_desc t_gcc1
PLUGINS = $B/gcc_enum_reflect.so
LIBRARY = $B/libenum_reflect.a

vpath %.c src
vpath %.cc src
vpath %.h include
vpath t_%.c tests

.PHONY: all clean test plugins
all: $(PLUGINS) $(LIBRARY)
TESTS_EXE = $(TESTS:%=build/%.exe)
plugins: $(PLUGINS)

world:
	$(MAKE) setup
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) test

setup:
	mkdir -p $B

test: $(TESTS_EXE)
test: $B/result.txt $T/result.gold
	diff $B/result.txt $T/result.gold
	@echo "Passed all tests."

$B/result.txt: $(TESTS_EXE)
	rm -f $@.new
	$B/t_enum_desc.exe >> $@.new
	$B/t_enum_refl.exe >> $@.new
	$B/t_gcc1.exe >> $@.new
	mv $@.new $@

$B/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIBRARY): enum_reflect.o
	rm -f $@.new
	ar rcs $@.new $^
	mv $@.new $@

$B/gcc_enum_reflect.so: gcc_enum_reflect.cc
	gcc $(CFLAGS) -fno-rtti -fno-exceptions -shared -fPIC -o $@ $< -I$$(gcc -print-file-name=plugin)/include

$B/t_gcc1.exe: t_gcc1.c $(LIBRARY) $(PLUGINS)
	gcc $(CFLAGS) -fplugin=$(PLUGINS) $< -o $@ $(LIBRARY)

$B/t_enum_desc.exe: t_enum_desc.o $(LIBRARY)
	gcc $(CFLAGS) -o $@ $^ $(LIBRARY)

$B/t_enum_refl.exe: t_enum_refl.o $(LIBRARY)
	gcc $(CFLAGS) -o $@ $^ $(LIBRARY)

$B/t_enum_desc.o: enum_desc.h enum_refl.h enum_desc_def.h
$B/t_enum_refl.o: enum_desc.h enum_refl.h enum_desc_def.h
$B/t_gcc1.o: enum_desc_def.h

clean:
	rm -f $B/*

