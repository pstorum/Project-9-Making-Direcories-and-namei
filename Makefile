CCOPTS=-Wall -Wextra

simfs: simfs_test.c image.c
	gcc $(CCOPTS) -o $@ $^

.PHONY: test clean

test: simfs_test.c image.c block.c free.c inode.c mkfs.c  pack.c ls.c
	@gcc $(CCOPTS) -DCTEST_ENABLE -o simfstest $^
	@./simfstest
	@rm -f simfstest
	@rm -f test_file

clean:
	rm -f simfs simfstest inode.o free.o mkfs.o image.o block.o  pack.o ls.o dirbasename.o test_file simfs_test.osimfs.a

simfs.a: image.o block.o inode.o free.o mkfs.o pack.o ls.o dirbasename.o
	ar rcs $@ $^

simfsexe: simfs_test.o simfs.a
	gcc -o $@ $^

mkfs.o: mkfs.c mkfs.h
	gcc -Wall -Wextra -c $<

image.o: image.c image.h
	gcc -Wall -Wextra -c $<

block.o: block.c block.h
	gcc -Wall -Wextra -c $<

inode.o: inode.c inode.h
	gcc -Wall -Wextra -c $<

free.o: free.c free.h
	gcc -Wall -Wextra -c $<

pack.o: pack.c pack.h
	gcc -Wall -Wextra -c $<

ls.o: ls.c ls.h
	gcc -Wall -Wextra -c $<

dirbasename.o: dirbasename.c
	gcc -Wall -Wextra -c $<

simfs_test.o: simfs_test.c
	gcc -Wall -Wextra -c $<
