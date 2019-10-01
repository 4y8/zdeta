brute_sort: zdc.c
	musl-gcc -o zdc zdc.c -I. && ./zdc Examples/brute_sort.zd && gcc -o test testhello.c && ./test

euclid: zdc.c
	musl-gcc -o zdc zdc.c -I. && ./zdc Examples/euclid.zd && gcc -o test testhello.c && ./test
insertion_sort: zdc.c
	musl-gcc -o zdc zdc.c -I. && ./zdc Examples/insert_sort.zd && gcc -o test testhello.c && ./test
