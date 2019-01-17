AR      := ar
ARFLAGS := -rv
CC      := gcc
CFLAGS  := -c
RM      := rm -f
TARGET  := ebcscrip.a
OBJS    := ebcscrip.o trnsunit.o parser.o decl.o expr.o init.o stmt.o \
	parse.tab.o lex.yy.o name.o code.o hashmap.o btree.o slist.o

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $?

ebcscrip.o: ebcscrip.c ebcscrip.h trnsunit.h parser.h decl.h expr.h init.h stmt.h name.h code.h hashmap.h slist.h btree.h boolean.h
	$(CC) $(CFLAGS) $<

trnsunit.o: trnsunit.c trnsunit.h name.h code.h slist.h hashmap.h boolean.h
	$(CC) $(CFLAGS) $<

parser.o: parser.c parser.h trnsunit.h stmt.h name.h code.h slist.h hashmap.h
	$(CC) $(CFLAGS) $<

decl.o: decl.c decl.h parser.h name.h
	$(CC) $(CFLAGS) $<

expr.o: expr.c expr.h stmt.h parser.h trnsunit.h name.h code.h btree.h boolean.h
	$(CC) $(CFLAGS) $<

init.o: init.c init.h expr.h parser.h trnsunit.h name.h code.h btree.h slist.h
	$(CC) $(CFLAGS) $<

stmt.o: stmt.c stmt.h parser.h trnsunit.h name.h slist.h hashmap.h boolean.h
	$(CC) $(CFLAGS) $<

parse.tab.o: parse.tab.c trnsunit.h parser.h decl.h expr.h init.h stmt.h name.h code.h btree.h slist.h hashmap.h
	$(CC) $(CFLAGS) $<

lex.yy.o: lex.yy.c parse.tab.h name.h decl.h expr.h btree.h hashmap.h
	$(CC) $(CFLAGS) $<

parse.tab.c: parse.y
	bison -d parse.y

lex.yy.c: lex.l parse.tab.h
	flex lex.l

name.o: name.c name.h hashmap.h slist.h boolean.h
	$(CC) $(CFLAGS) $<

code.o: code.c code.h boolean.h
	$(CC) $(CFLAGS) $<

hashmap.o: hashmap.c hashmap.h boolean.h
	$(CC) $(CFLAGS) $<

btree.o: btree.c btree.h
	$(CC) $(CFLAGS) $<

slist.o: slist.c slist.h boolean.h
	$(CC) $(CFLAGS) $<

.PHONY : clean
clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)
	$(RM) lex.yy.c parse.tab.c
