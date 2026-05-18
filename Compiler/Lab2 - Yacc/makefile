all:	clean y.tab.c lex.yy.c
	gcc lex.yy.c y.tab.c -lfl -o compiler
	@echo "-------------------------------------"

y.tab.c:
	bison -y -d -Wno-yacc yacc.y

lex.yy.c:
	flex lex.l

clean:
	rm -f compiler lex.yy.c y.tab.c y.tab.h
