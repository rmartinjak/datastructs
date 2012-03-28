CC = cc
CPPFLAGS = -ansi -pedantic -Wall -g

TESTS = test_ht

all : clean $(TESTS)

test_ht : test_ht.c test_ht_init.c test_ht_simple.c test_ht_args.c
	@echo
	@$(CC) -I../src $(CPPFLAGS) $(CFLAGS) -o $@ $? -lcheck ../datastructs.a
	@./$@
	@rm $@

clean:
	@rm -f $(TESTS)
