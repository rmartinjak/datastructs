include $(CONFIG_MK)

CC = cc
CFLAGS = -g -Wall
LDFLAGS =
LIBS =

AR = ar
ARFLAGS = rs

SRCDIR = src
OBJDIR = obj
DESTDIR = .


_OBJ = hashtable
OBJ = $(addprefix $(OBJDIR)/,$(addsuffix .o,$(_OBJ)))

all : archive


$(OBJDIR) :
	@mkdir $(OBJDIR)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@echo CC -c $<
	@$(CC) $(CFLAGS) -c -o $@ $<


archive : $(DESTDIR)/datastructs.a

$(DESTDIR)/datastructs.a : $(OBJDIR) $(OBJ)
	@$(AR) $(ARFLAGS) $@ $(OBJ)


httest : $(OBJDIR) $(OBJ)
	@echo CC -o $@
	@$(CC) $(CFLAGS) -o httest $(SRCDIR)/httest.c $(OBJ) $(LDFLAGS)


doc : src/hashtable.h doxyfile
	@echo generating doc
	@doxygen doxyfile >/dev/null

clean :
	@echo cleaning
	@rm -f httest
	@rm -rf doc
	@rm -rf $(OBJDIR)

.PHONY: clean
