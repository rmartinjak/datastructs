CC = cc
CPPFLAGS = -ansi -pedantic -Wall -g

TESTS = test_ht test_bst

all : clean $(TESTS)

test_ht : test_ht.c test_ht_init.c test_ht_simple.c test_ht_args.c
	@$(CC) -I../src $(CPPFLAGS) $(CFLAGS) -o $@ $? -lcheck ../datastructs.a
	@./$@
	@rm $@
	@echo

test_bst : test_bst.c
	@$(CC) -I../src $(CPPFLAGS) $(CFLAGS) -o $@ $? -lcheck ../datastructs.a
	@./$@
	@rm $@
	@echo

clean:
	@rm -f $(TESTS)
