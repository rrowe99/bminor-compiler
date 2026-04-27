CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Werror -O2 -D_XOPEN_SOURCE=700
CFLAGS_NO_WERROR := $(filter-out -Werror,$(CFLAGS))
LEX     := flex
YACC    := bison
YFLAGS  := -Wall -v -d

BIN     := bminor

SRCS = main.c decl.c expr.c stmt.c type.c param_list.c stack.c scope.c symbol.c scratch.c label.c symbol_codegen.c expr_codegen.c stmt_codegen.c decl_codegen.c

OBJS = $(SRCS:.c=.o) hash_table.o

all: $(BIN)

parser.c parser.h: parser.y
	$(YACC) $(YFLAGS) -o parser.c parser.y

lex.yy.c: scanner.l parser.h tokens.h
	$(LEX) -o $@ $<

parser.o: parser.c
	$(CC) $(CFLAGS) -c $<

lex.yy.o: lex.yy.c
	$(CC) $(CFLAGS) -c $<

hash_table.o: hash_table.c
	$(CC) $(CFLAGS_NO_WERROR) -c $<

$(SRCS:.c=.o): %.o: %.c
	$(CC) $(CFLAGS) -c $<

$(OBJS): parser.h

$(BIN): parser.o lex.yy.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lfl

.PHONY: test
test: $(BIN)
	@for f in tests/*.bminor; do \
		echo "TEST $$f"; \
		./$(BIN) $$f; \
		echo ""; \
	done

.PHONY: clean
clean:
	rm -f $(BIN) *.o parser.c parser.h parser.output lex.yy.c

