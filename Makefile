all: hw2.exe 

hw2.exe : parser.c lexer.l header.h
	flex -o parser.yy.c parser.l
	gcc -o $@ parser.c parser.yy.c -lfl 

.PHONY : clean
clean :
	rm -rf *.yy.c *.tab.c *.exe *.o 