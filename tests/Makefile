CC = cc
CPPFLAGS = -Wall -g

TESTS = test_ht_init

all : clean $(TESTS)

% : %.c
	@$(CC) -I../src $(CPPFLAGS) $(CFLAGS) -o $@ $^ -lcheck ../datastructs.a
	@./$@
	@rm $@

clean:
	@rm -f $(TESTS)
