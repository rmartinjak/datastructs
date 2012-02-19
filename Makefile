CC = cc
CFLAGS = -g -Wall
LDFLAGS =

SRCDIR=.
OBJDIR=obj

_OBJ = hashtable.o test.o
OBJ = $(addprefix $(OBJDIR)/,$(_OBJ))

default : httest

$(OBJDIR) :
	@mkdir $(OBJDIR)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@echo CC -c $<
	@$(CC) $(CFLAGS) -c -o $@ $<

httest : $(OBJDIR) $(OBJ)
	@echo CC -o $@
	@$(CC) $(LDFLAGS) -o httest $(OBJ)

clean :
	@echo cleaning
	@rm -f httest
	@rm -rf $(OBJDIR)

.PHONY: clean