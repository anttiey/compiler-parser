
all: hw2.exe 

hw2.exe : hw2.c hw2.l header.h
	flex -o hw2.yy.c hw2.l
	gcc -o $@ hw2.c hw2.yy.c -lfl 

.PHONY : clean
clean :
	rm -rf *.yy.c *.tab.c *.exe *.o 