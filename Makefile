CC = cc
CPPFLAGS = -DPGROUP_COUNT=20 -DPGROUP_ELEMENTS=10
CFLAGS = -g -ansi -pedantic -Wall
LDFLAGS =
LIBS =

AR = ar
ARFLAGS = rs
ECHO = echo
MKDIR = mkdir
SED = sed
DOXYGEN = doxygen

SRCDIR = src
OBJDIR = obj
DOCDIR = doc
DESTDIR = .
ARCHIVENAME = datastructs.a


_OBJ = hashtable queue bst
OBJ = $(addprefix $(OBJDIR)/,$(addsuffix .o,$(_OBJ)))

all : archive


$(OBJDIR) :
	@$(MKDIR) $(OBJDIR)

$(OBJDIR)/%.o : $(SRCDIR)/%.c $(SRCDIR)/pgroups.h
	@$(ECHO) CC -c $<
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

archive : $(DESTDIR)/$(ARCHIVENAME)

$(DESTDIR)/datastructs.a : $(OBJDIR) $(OBJ)
	@$(AR) $(ARFLAGS) $@ $(OBJ)


httest : $(OBJDIR) $(OBJ)
	@$(ECHO) CC -o $@
	@$(CC) $(CFLAGS) -o httest $(SRCDIR)/httest.c $(OBJ) $(LDFLAGS)

doc : src/hashtable.h doxyfile
	@$(MKDIR) $(DOCDIR) 2> /dev/null || true
	@$(ECHO) generating documenation in $(DOCDIR)/
	@$(SED) -e s:DOXYGEN_OUTPUT_DIR:$(DOCDIR): doxyfile | $(DOXYGEN) - 2>&1 > /dev/null

$(SRCDIR)/pgroups.h : genpgroups
	@echo generating pgroups.h
	@./$< > $@

genpgroups: $(SRCDIR)/genpgroups.c
	@echo CC $<
	@$(CC) $(CPPFLAGS) $(CFLAGS) $< -o genpgroups
clean :
	@$(ECHO) cleaning
	@$(RM) $(SRCDIR)/pgroups.h
	@$(RM) -f httest
	@$(RM) -rf doc
	@$(RM) -rf $(OBJDIR)

.PHONY: clean doc
