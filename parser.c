/*******************************************************************************
   Project : C script
   File    : parser.c
   Date    : 2018.8.17-
   Note    : 構文解析時の作業用
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include "parser.h"
#include "trnsunit.h"
#include "name.h"
#include "code.h"
#include "slist.h"
#include "hashmap.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Parser_malloc(size_t Size);
void _Ebcscript_Parser_free(void *P);
void _Ebcscript_Parser_exit(int Status);
void _Ebcscript_Parser_log(const char *Fmt, ...);

FILE *Ebcscript_Parser_Fplog/* = stdout*/;
size_t Ebcscript_Parser_MallocTotal = 0;
void *(*Ebcscript_Parser_malloc)(size_t Size) = &_Ebcscript_Parser_malloc;
void (*Ebcscript_Parser_free)(void *) = &_Ebcscript_Parser_free;
void (*Ebcscript_Parser_exit)(int) = &_Ebcscript_Parser_exit;
void (*Ebcscript_Parser_log)(const char *Fmt, ...) = &_Ebcscript_Parser_log;

/*                                                                   関数定義 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Parser_malloc(size_t Size)
{
	void *P;

	if ((P = malloc(Size + sizeof(size_t))) == NULL){
	  fprintf(Ebcscript_Parser_Fplog,
	   "Ebcscript_Parser: error: memory allocation error(total=%d bytes)\n",
	   Ebcscript_Parser_MallocTotal);
	  Ebcscript_Parser_exit(1);
	}
	*(size_t *)P = Size;
	Ebcscript_Parser_MallocTotal += Size;
	return (size_t *)P + 1;
}

void _Ebcscript_Parser_free(void *P)
{
	P = (size_t *)P - 1;
	Ebcscript_Parser_MallocTotal -= *(size_t *)P;
	free(P);
}

void Ebcscript_Parser_onexit(void)
{
	fclose(Ebcscript_Parser_Fplog);
}

void _Ebcscript_Parser_exit(int Status)
{
	Ebcscript_Parser_onexit();
	exit(Status);
}

void _Ebcscript_Parser_log(const char *Fmt, ...)
{
	va_list Args;

	fprintf(Ebcscript_Parser_Fplog, "ebcscript_parser: ");

	va_start(Args, Fmt);
	vfprintf(Ebcscript_Parser_Fplog, Fmt, Args);
	va_end(Args);
}
/* -------------------------------------------------------------------------- */
void Ebcscript_Parser_pushBlockstack(
                         ebcscript_parser *Prs, ebcscript_parser_blockstack *BS)
{
	BS->Prev = Prs->BS; Prs->BS = BS;

	BS->Nest = Prs->Nest;
}

ebcscript_parser_blockstack *Ebcscript_Parser_popBlockstack(
                                                          ebcscript_parser *Prs)
{
	ebcscript_parser_blockstack *BS;

	BS = Prs->BS; Prs->BS = BS->Prev;
	return BS;
}
/* -------------------------------------------------------------------------- */
void Ebcscript_Parser_push_char(ebcscript_parser *Prs, char C)
{
	Ebcscript_Stack_push_char(Prs->Stack, C);
	return;
}

void Ebcscript_Parser_push_short(ebcscript_parser *Prs, short S)
{
	Ebcscript_Stack_push_short(Prs->Stack, S);
	return;
}

void Ebcscript_Parser_push_int(ebcscript_parser *Prs, int I)
{
	Ebcscript_Stack_push_int(Prs->Stack, I);
	return;
}

void Ebcscript_Parser_push_long(ebcscript_parser *Prs, long L)
{
	Ebcscript_Stack_push_long(Prs->Stack, L);
	return;
}

void Ebcscript_Parser_push_uchar(ebcscript_parser *Prs, unsigned char C)
{
	Ebcscript_Stack_push_uchar(Prs->Stack, C);
	return;
}

void Ebcscript_Parser_push_ushort(ebcscript_parser *Prs, unsigned short S)
{
	Ebcscript_Stack_push_ushort(Prs->Stack, S);
	return;
}

void Ebcscript_Parser_push_uint(ebcscript_parser *Prs, unsigned int I)
{
	Ebcscript_Stack_push_uint(Prs->Stack, I);
	return;
}

void Ebcscript_Parser_push_ulong(ebcscript_parser *Prs, unsigned long L)
{
	Ebcscript_Stack_push_ulong(Prs->Stack, L);
	return;
}

void Ebcscript_Parser_push_float(ebcscript_parser *Prs, float F)
{
	Ebcscript_Stack_push_float(Prs->Stack, F);
	return;
}

void Ebcscript_Parser_push_double(ebcscript_parser *Prs, double D)
{
	Ebcscript_Stack_push_double(Prs->Stack, D);
	return;
}

void Ebcscript_Parser_push_address(ebcscript_parser *Prs, void *P)
{
	Ebcscript_Stack_push_address(Prs->Stack, P);
	return;
}

void Ebcscript_Parser_pop_char(ebcscript_parser *Prs, char *C)
{
	Ebcscript_Stack_pop_char(Prs->Stack, C);
	return;
}

void Ebcscript_Parser_pop_short(ebcscript_parser *Prs, short *S)
{
	Ebcscript_Stack_pop_short(Prs->Stack, S);
	return;
}

void Ebcscript_Parser_pop_int(ebcscript_parser *Prs, int *I)
{
	Ebcscript_Stack_pop_int(Prs->Stack, I);
	return;
}

void Ebcscript_Parser_pop_long(ebcscript_parser *Prs, long *L)
{
	Ebcscript_Stack_pop_long(Prs->Stack, L);
	return;
}

void Ebcscript_Parser_pop_uchar(ebcscript_parser *Prs, unsigned char *C)
{
	Ebcscript_Stack_pop_uchar(Prs->Stack, C);
	return;
}

void Ebcscript_Parser_pop_ushort(ebcscript_parser *Prs, unsigned short *S)
{
	Ebcscript_Stack_pop_ushort(Prs->Stack, S);
	return;
}

void Ebcscript_Parser_pop_uint(ebcscript_parser *Prs, unsigned int *I)
{
	Ebcscript_Stack_pop_uint(Prs->Stack, I);
	return;
}

void Ebcscript_Parser_pop_ulong(ebcscript_parser *Prs, unsigned long *L)
{
	Ebcscript_Stack_pop_ulong(Prs->Stack, L);
	return;
}

void Ebcscript_Parser_pop_float(ebcscript_parser *Prs, float *F)
{
	Ebcscript_Stack_pop_float(Prs->Stack, F);
	return;
}

void Ebcscript_Parser_pop_double(ebcscript_parser *Prs, double *D)
{
	Ebcscript_Stack_pop_double(Prs->Stack, D);
	return;
}

void Ebcscript_Parser_pop_address(ebcscript_parser *Prs, void **P)
{
	Ebcscript_Stack_pop_address(Prs->Stack, P);
	return;
}

/* -------------------------------------------------------------------------- */
ebcscript_parser *Ebcscript_newParser(char *Filename)
{
	ebcscript_parser *Parser;

	Parser = Ebcscript_Parser_malloc(sizeof(ebcscript_parser));
	Parser->Filename = Ebcscript_Parser_malloc(strlen(Filename) + 1);
	strcpy(Parser->Filename, Filename);
	Parser->Line = 1;
	Parser->EnumNum = 0;

	return Parser;
}

void Ebcscript_deleteParser(ebcscript_parser *Parser)
{
	ebcscript_parser_blockstack *BS;

	while (Parser->BS != NULL) {
	  BS = Ebcscript_Parser_popBlockstack(Parser);
	  Ebcscript_Parser_deleteBlockstack(BS);
	}
	Ebcscript_Parser_free(Parser->Filename);
	Ebcscript_Parser_free(Parser);
	return;
}

/* -------------------------------------------------------------------------- */
#ifdef DEBUG
int main(int argc, char *argv[])
{
	return 0;
}
#endif
