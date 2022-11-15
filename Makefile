all: hw2.exe 

hw2.exe : parser.c lexer.l header.h
	flex -o lexer.yy.c lexer.l
	gcc -o $@ parser.c lexer.yy.c -lfl 

.PHONY : clean
clean :
	rm -rf *.yy.c *.tab.c *.exe *.o 