CC = gcc
CFLAGS = -std=gnu99 -Wall -g -pthread

OBJS = list.o threadpool.o

.PHONY: all clean test

GIT_HOOKS := .git/hooks/pre-commit

all: $(GIT_HOOKS) sort sort_autotest

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

deps := $(OBJS:%.o=.%.o.d)
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -MMD -MF .$@.d -c $<

sort: $(OBJS) main.c mergesort.c
	$(CC) $(CFLAGS) -o $@ $^ -rdynamic

sort_autotest: $(OBJS) main.c mergesort.c
	$(CC) $(CFLAGS) -DBENCH -o $@ $^ -rdynamic

clean:
	rm -f *.o sort sort_autotest
	@rm -rf $(deps)

-include $(deps)
