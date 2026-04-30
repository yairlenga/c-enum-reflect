default: all

CFLAGS = -g -Wall -Werror -I$I
CXXFLAGS = -g -Wall -Werror -Iinclude -std=c++11


B = build
S = ../api/src
I = ../api/include
T = tests
TESTS = t_gcc1  # t_gpp2
PLUGINS = $B/gcc_enum_reflect.so
LIBRARY = $B/libenum_reflect.a

vpath t_%.cc $T
vpath t_%.c $T
vpath %.c $S
vpath %.cc $S
vpath %.h $I

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
	$B/t_gcc1.exe >> $@.new
	mv $@.new $@

$B/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$B/%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(PLUGINS): gcc_enum_reflect.cc
	gcc $(CFLAGS) -fno-rtti -fno-exceptions -shared -fPIC -o $@ $< -I$$(gcc -print-file-name=plugin)/include

$(LIBRARY): $B/enum_desc.o $B/enum_refl.o
	rm -f $@.new
	ar rcs $@.new $^
	mv $@.new $@

$B/t_gcc1.exe: t_gcc1.c $(LIBRARY) $(PLUGINS)
	gcc $(CFLAGS) -fplugin=$(PLUGINS) $< -o $@ $(LIBRARY)

$B/t_gpp2.exe: t_gpp2.o $(LIBRARY) $(PLUGINS)
	$(CXX) $(CXXFLAGS) -fplugin=$(PLUGINS) $< -o $@ $(LIBRARY)


$B/t_gcc1.o: enum_desc_def.h
$B/t_gpp2.o: enum_desc_def.h

clean:
	rm -f $B/*.exe $B/*.o $B/*.so $T/*.result

realclean: clean