all:	parser.out

parser_202020813.out:	parser.c lexer.l header.h
	flex -o lexer.yy.c lexer.l
	gcc -o $@ parser.c lexer.yy.c -lfl 

.PHONY:	clean
clean	:
	rm -rf *.yy.c *.tab.c *.exe *.o *.out
