brute_sort: zdc.c
	tcc -o zdc zdc.c && ./zdc Examples/brute_sort.zd && ./out
euclid: zdc.c
	tcc -o zdc zdc.c -I. && ./zdc Examples/euclid.zd && gcc -o test testhello.c && ./test
insertion_sort: zdc.c
	tcc -o zdc zdc.c -I. && ./zdc Examples/insert_sort.zd && gcc -o test testhello.c && ./test
test: zdc.c
	tcc -o zdc zdc.c -I. && ./zdc Examples/test.zd && ./out
hello: zdc.c
	tcc -o zdc zdc.c && ./zdc Examples/hello_world.zd && ./out
leak_test: zdc.c
	gcc -o zdc -g zdc.c && valgrind -v --tool=memcheck --leak-check=full --show-reachable=yes --num-callers=20 --track-fds=yes ./zdc Examples/test.zd
