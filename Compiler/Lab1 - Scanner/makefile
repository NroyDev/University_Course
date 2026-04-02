FILE_lex=	lex.l
PROG_lex=	lex.yy.c
all:	$(PROG_lex)
	gcc $(PROG_lex) -lfl -o scanner

$(PROG_lex):	$(FILE_lex)
	flex $(FILE_lex)

clean:
	rm scanner $(PROG_lex)
