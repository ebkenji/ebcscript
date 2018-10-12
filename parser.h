/*******************************************************************************
   Project : C script
   File    : parser.h
   Date    : 2018.8.17
   Note    : 構文解析時の作業用
*******************************************************************************/
#ifndef EBCSCRIPT_PARSER
#define EBCSCRIPT_PARSER

#include <setjmp.h>
#include "trnsunit.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"
#include "name.h"
#include "code.h"
#include "slist.h"
#include "hashmap.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                                 構造体宣言 */
/* -------------------------------------------------------------------------- */
enum ebcscript_parser_kind {
	EBCSCRIPT_PARSER_KIND_TRNSUNIT,
	EBCSCRIPT_PARSER_KIND_FUNCTION,
	EBCSCRIPT_PARSER_KIND_BLOCK,
	EBCSCRIPT_PARSER_KIND_IF,
	EBCSCRIPT_PARSER_KIND_SWITCH,
	EBCSCRIPT_PARSER_KIND_LOOP,
};

/* 解析時の作業用 */
typedef struct ebcscript_parser ebcscript_parser;
struct ebcscript_parser {
/*
	ebcscript_parser *Prev;
	enum ebcscript_parser_kind Kind;

	union {
	  ebcscript_parser_trnsunit Trnsunit;
	  ebcscript_parser_block Block;
	  ebcscript_parser_function Function;
	  ebcscript_parser_if If;
	  ebcscript_parser_Switch Switch;
	  ebcscript_parser_Loop Loop;
	} As;
*/
	ebcscript_trnsunit *TU;	/* 現翻訳単位 */
	ebcscript_parser_blockstack *BS;

	int Nest;	/* 0:外部, 0<:ブロック内部 */
	int AnonymousNum;
	int EnumNum;
	int Line;
	char *Filename;

	/* スタック *//* 固定サイズ。new時に引数でサイズを指定 */
	ebcscript_stack *Stack;

	/* エラー処理 */
	jmp_buf CheckPoint;
};

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
extern FILE *Ebcscript_Parser_Fplog;
extern size_t Ebcscript_Parser_MallocTotal;
extern void *(*Ebcscript_Parser_malloc)(size_t Size);
extern void (*Ebcscript_Parser_free)(void *);
extern void (*Ebcscript_Parser_exit)(int);
extern void (*Ebcscript_Parser_log)(const char *Fmt, ...);

/*                                                                   関数宣言 */
/* -------------------------------------------------------------------------- */
void Ebcscript_Parser_onexit(void);

ebcscript_parser *Ebcscript_newParser(char *Filename);
void Ebcscript_deleteParser(ebcscript_parser *Prs);

void Ebcscript_Parser_pushBlockstack(
                             ebcscript_parser *, ebcscript_parser_blockstack *);
ebcscript_parser_blockstack *Ebcscript_Parser_popBlockstack(ebcscript_parser *);

void Ebcscript_Parser_push_char(ebcscript_parser *Prs, char C);
void Ebcscript_Parser_push_short(ebcscript_parser *Prs, short S);
void Ebcscript_Parser_push_int(ebcscript_parser *Prs, int I);
void Ebcscript_Parser_push_long(ebcscript_parser *Prs, long L);
void Ebcscript_Parser_push_uchar(ebcscript_parser *Prs, unsigned char UC);
void Ebcscript_Parser_push_ushort(ebcscript_parser *Prs, unsigned short US);
void Ebcscript_Parser_push_uint(ebcscript_parser *Prs, unsigned int UI);
void Ebcscript_Parser_push_ulong(ebcscript_parser *Prs, unsigned long UL);
void Ebcscript_Parser_push_float(ebcscript_parser *Prs, float F);
void Ebcscript_Parser_push_double(ebcscript_parser *Prs, double D);
void Ebcscript_Parser_push_address(ebcscript_parser *Prs, void *P);
void Ebcscript_Parser_pop_char(ebcscript_parser *Prs, char *C);
void Ebcscript_Parser_pop_short(ebcscript_parser *Prs, short *S);
void Ebcscript_Parser_pop_int(ebcscript_parser *Prs, int *I);
void Ebcscript_Parser_pop_long(ebcscript_parser *Prs, long *L);
void Ebcscript_Parser_pop_uchar(ebcscript_parser *Prs, unsigned char *UC);
void Ebcscript_Parser_pop_ushort(ebcscript_parser *Prs, unsigned short *US);
void Ebcscript_Parser_pop_uint(ebcscript_parser *Prs, unsigned int *UI);
void Ebcscript_Parser_pop_ulong(ebcscript_parser *Prs, unsigned long *UL);
void Ebcscript_Parser_pop_float(ebcscript_parser *Prs, float *F);
void Ebcscript_Parser_pop_double(ebcscript_parser *Prs, double *D);
void Ebcscript_Parser_pop_address(ebcscript_parser *Prs, void **P);

/*                                                           関数宣言[decl.c] */
/* -------------------------------------------------------------------------- */
/* 宣言指定子についての情報D1をD2に統合し、D2を返す */
ebcscript_parser_declaration *Ebcscript_Parser_mergeDeclaration(
 ebcscript_parser *Prs,
 ebcscript_parser_declaration *D1,
 ebcscript_parser_declaration *D2
);

/* 宣言で名前が重複している場合の処理。エラーを出すか何もしない。 */
void Ebcscript_Parser_checkRepetation_varfunc(
 ebcscript_parser *Prs,
 ebcscript_name *N0,
 ebcscript_name *N,
 int Nest
);
void Ebcscript_Parser_checkRepetation_tag(
 ebcscript_parser *Prs,
 ebcscript_name *N0,
 ebcscript_name *N
);
void Ebcscript_Parser_checkRepetation_label(
 ebcscript_parser *Prs,
 ebcscript_name *N0,
 ebcscript_name *N
);

/*                                                           関数宣言[expr.c] */
/* -------------------------------------------------------------------------- */
void Ebcscript_Parser_gencode_expr_rv(
 ebcscript_parser *Prs,
 ebcscript_parser_expression *E
);
void Ebcscript_Parser_gencode_expr_lv(
 ebcscript_parser *Prs,
 ebcscript_parser_expression *E
);
void Ebcscript_Parser_eval_expr_rv(
 ebcscript_parser *Prs,
 ebcscript_parser_expression *E
);
void Ebcscript_Parser_eval_expr_lv(
 ebcscript_parser *Prs,
 ebcscript_parser_expression *E
);

/*                                                           関数宣言[init.c] */
/* -------------------------------------------------------------------------- */
/* 初期化子リストは、btree.Rightで伸ばす。
   入れ子の{}が現れたら、btree.DatumをNULLにして、btree.Leftに分岐する。 */

/* 型の情報Typeに従って初期化子Initializerをなぞる。
   初期化子の内容が、型の情報に含まれる配列の要素数、構造体のメンバ数、
   と一致していなければエラーを出す。
   不定サイズ（Length = 0）の配列の場合、初期化子リストの数を数えて、
   サイズ（Length）を設定する。 */
void Ebcscript_Parser_walkInitializerAlongType(
 ebcscript_parser *Prs,
 btree *Initializer,	/* P->Left */
 ebcscript_type *Type	/* P->Datum->Info.Variable.TypeTree */
);

/* 初期化子を評価して代入 */
void Ebcscript_Parser_eval_initializer(
 ebcscript_parser *Prs,
 btree *Initializer,
 ebcscript_type *Type,
 void *Address
);

/* リテラル化。代入コードの生成 */
void Ebcscript_Parser_gencode_initializer(
 ebcscript_parser *Prs,
 btree *Initializer,
 ebcscript_type *Type,
 void *Address
);

/*                                                           関数宣言[stmt.c] */
/* -------------------------------------------------------------------------- */
void Ebcscript_Parser_resolve(
                        ebcscript_parser *Prs, ebcscript_parser_blockstack *BS);
void Ebcscript_Parser_resolve_if(
                        ebcscript_parser *Prs, ebcscript_parser_blockstack *BS);
void Ebcscript_Parser_resolve_loop(
                        ebcscript_parser *Prs, ebcscript_parser_blockstack *BS);
void Ebcscript_Parser_resolve_switch(
                        ebcscript_parser *Prs, ebcscript_parser_blockstack *BS);
void Ebcscript_Parser_resolve_function(
                        ebcscript_parser *Prs, ebcscript_parser_blockstack *BS);
void Ebcscript_Parser_resolve_block(
                        ebcscript_parser *Prs, ebcscript_parser_blockstack *BS);
void Ebcscript_Parser_resolve_trnsunit(
                        ebcscript_parser *Prs, ebcscript_parser_blockstack *BS);

void Ebcscript_Parser_store_address_n(
     ebcscript_parser *Prs, ebcscript_parser_blockstack *BS, ebcscript_name *N);
void Ebcscript_Parser_store_address_n_if(
     ebcscript_parser *Prs, ebcscript_parser_blockstack *BS, ebcscript_name *N);
void Ebcscript_Parser_store_address_n_loop(
     ebcscript_parser *Prs, ebcscript_parser_blockstack *BS, ebcscript_name *N);
void Ebcscript_Parser_store_address_n_switch(
     ebcscript_parser *Prs, ebcscript_parser_blockstack *BS, ebcscript_name *N);
void Ebcscript_Parser_store_address_n_function(
     ebcscript_parser *Prs, ebcscript_parser_blockstack *BS, ebcscript_name *N);
void Ebcscript_Parser_store_address_n_block(
     ebcscript_parser *Prs, ebcscript_parser_blockstack *BS, ebcscript_name *N);
void Ebcscript_Parser_store_address_n_trnsunit(
     ebcscript_parser *Prs, ebcscript_parser_blockstack *BS, ebcscript_name *N);

#endif
