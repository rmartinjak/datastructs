CC = cc
CPPFLAGS = -Wall

TESTS = test1

all : clean $(TESTS)

% : %.c
	$(CC) -I../src $(CPPFLAGS) $(CFLAGS) -o $@ $^ -lcheck ../datastructs.a
	./$@
	rm $@

clean:
	rm -f $(TESTS)
