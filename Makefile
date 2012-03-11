include $(CONFIG_MK)

CC = cc
CFLAGS = -g -Wall
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


_OBJ = hashtable
OBJ = $(addprefix $(OBJDIR)/,$(addsuffix .o,$(_OBJ)))

all : archive


$(OBJDIR) :
	@$(MKDIR) $(OBJDIR)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(ECHO) CC -c $<
	@$(CC) $(CFLAGS) -c -o $@ $<


archive : $(DESTDIR)/datastructs.a

$(DESTDIR)/datastructs.a : $(OBJDIR) $(OBJ)
	@$(AR) $(ARFLAGS) $@ $(OBJ)


httest : $(OBJDIR) $(OBJ)
	@$(ECHO) CC -o $@
	@$(CC) $(CFLAGS) -o httest $(SRCDIR)/httest.c $(OBJ) $(LDFLAGS)

doc : src/hashtable.h doxyfile
	@$(MKDIR) $(DOCDIR) 2> /dev/null || true
	@$(ECHO) generating documenation in $(DOCDIR)/
	@$(SED) -e s:DOXYGEN_OUTPUT_DIR:$(DOCDIR): doxyfile | $(DOXYGEN) - 2>&1 > /dev/null

clean :
	@$(ECHO) cleaning
	@$(RM) -f httest
	@$(RM) -rf doc
	@$(RM) -rf $(OBJDIR)

.PHONY: clean doc
