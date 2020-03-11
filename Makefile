all: zdc.c
	tcc -o zdc zdc.c -w
brute_sort: zdc.c
	tcc -o zdc zdc.c -w && ./zdc Examples/brute_sort.zd  && ./out
test: zdc.c
	tcc -o zdc zdc.c -w && ./zdc Examples/test.zd        && ./out
hello: zdc.c
	tcc -o zdc zdc.c -w && ./zdc Examples/hello_world.zd && ./out
fibonacci: zdc.c
	tcc -o zdc zdc.c -w && ./zdc Examples/fibonacci.zd   && ./out
leak_test: zdc.c
	gcc -o zdc -g zdc.c && valgrind -v --tool=memcheck --leak-check=full --show-reachable=yes --num-callers=20 --track-fds=yes ./zdc Examples/test.zd
