CC = cc
CPPFLAGS = -DPGROUP_COUNT=8 -DPGROUP_ELEMENTS=10
CFLAGS = -g -Wall
LDFLAGS =

SRCDIR=.
OBJDIR=obj

_OBJ = hashtable.o test.o
OBJ = $(addprefix $(OBJDIR)/,$(_OBJ))

default : httest

$(OBJDIR) :
	@mkdir $(OBJDIR)

$(OBJDIR)/%.o : $(SRCDIR)/%.c pgroups.h
	@echo CC -c $<
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

httest : $(OBJDIR) $(OBJ)
	@echo CC -o $@
	@$(CC) $(CFLAGS) $(LDFLAGS) -o httest $(OBJ)

doc : hashtable.h doxyfile
	@echo generating doc
	@doxygen doxyfile >/dev/null

pgroups.h : genpgroups
	@echo generating pgroups.h
	@./genpgroups > $@

genpgroups: genpgroups.c
	@echo CC $<
	@$(CC) $(CPPFLAGS) $(CFLAGS) $< -o genpgroups
clean :
	@echo cleaning
	@rm -f httest
	@rm -rf $(OBJDIR)

.PHONY: clean
