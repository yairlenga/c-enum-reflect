default: all

ENUMDESC_DIR = .
OBJDIR = .
PROGRAMS = test_color test_country

setup:

all: $(PROGRAMS)

CFLAGS = -g -I $(ENUMDESC_DIR) -Wall -Werror
OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%: | $(OBJDIR)

$(OBJDIR)/gen_%.o: $(OBJDIR)/%.o
	$(ENUMDESC_DIR)/enum_dwarf_query --format=c $< > $(OBJDIR)/gen_$*.c
	$(COMPILE.c) -o $@ $(OBJDIR)/gen_$*.c

$(OBJDIR)/enum_desc.o: $(ENUMDESC_DIR)/enum_desc.c
	$(COMPILE.c) -o $@ $^

$(OBJS) $(ENUMDESC_OBJS): $(ENUMDESC_DIR)/enum_desc.h

test_color: $(addprefix $(OBJDIR)/, test_color.o enum_desc.o gen_test_color.o)
	$(LINK.c) -o $@ $(filter %.o, $^)

test_country: $(addprefix $(OBJDIR)/, test_country.o enum_desc.o gen_test_country.o)
	$(LINK.c) -o $@ $(filter %.o, $^)


clean:
	rm -f $(OBJDIR)/*.o $(PROGRAMS) $(OBJDIR)/gen_*.c

test: test_color test_country
	./test_color
	./test_country 840 826 392 250 276
