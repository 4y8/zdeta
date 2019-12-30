brute_sort: zdc.c
	musl-gcc -o zdc zdc.c -I. && ./zdc Examples/brute_sort.zd && gcc -o test testhello.c && ./test
euclid: zdc.c
	musl-gcc -o zdc zdc.c -I. && ./zdc Examples/euclid.zd && gcc -o test testhello.c && ./test
insertion_sort: zdc.c
	musl-gcc -o zdc zdc.c -I. && ./zdc Examples/insert_sort.zd && gcc -o test testhello.c && ./test
test: zdc.c
	clang -o zdc zdc.c -I. && ./zdc Examples/test.zd
hello: zdc.c
	clang -o zdc zdc.c -I. && ./zdc Examples/hello_world.zd
