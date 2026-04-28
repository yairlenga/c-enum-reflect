default: all

SUBDIR = api  dwarf-query dist/minimal

setup:
	for d in $(SUBDIR) ; do make -C $$d $@ ; done

all:
	for d in $(SUBDIR) ; do make -C $$d $@ ; done

clean:
	for d in $(SUBDIR) ; do make -C $$d $@ ; done

realclean:
	for d in $(SUBDIR) ; do make -C $$d $@ ; done

test:
	for d in $(SUBDIR) ; do make -C $$d $@ ; done
