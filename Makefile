CC = gcc
CFLAGS = -std=gnu99 -Wall -g -pthread

OBJS = list.o threadpool.o

.PHONY: all clean test

GIT_HOOKS := .git/hooks/pre-commit

all: $(GIT_HOOKS) sort sort_bench

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

deps := $(OBJS:%.o=.%.o.d)
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -MMD -MF .$@.d -c $<

sort: $(OBJS) main.c mergesort.c
	$(CC) $(CFLAGS) -o $@ $^ -rdynamic

sort_bench: $(OBJS) main.c mergesort.c
	$(CC) $(CFLAGS) -DBENCH -o $@ $^ -rdynamic

nums=100000
gencsv: sort_bench
	printf ",2-threads,4-threads,8-threads,16-threads\n" > result_clock_gettime.csv
	for i in `seq 1 1 100`; do \
	    (for j in {1..$(nums)}; do echo $$RANDOM; done > numbers.txt); \
	    printf "%d," $$i >> result_clock_gettime.csv; \
	    ./sort_bench 2 $(nums) < numbers.txt >> result_clock_gettime.csv; \
	    ./sort_bench 4 $(nums) < numbers.txt >> result_clock_gettime.csv; \
	    ./sort_bench 8 $(nums) < numbers.txt >> result_clock_gettime.csv; \
	    ./sort_bench 16 $(nums) < numbers.txt >> result_clock_gettime.csv; \
	    printf "\n" >> result_clock_gettime.csv; \
	done

clean:
	rm -f *.o sort sort_bench result_clock_gettime.csv numbers.txt
	@rm -rf $(deps)

-include $(deps)
