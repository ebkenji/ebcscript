/*******************************************************************************
   Project : C script
   File    : stmt.h
   Date    : 2018.12.5-
   Note    : 制御構文のコード生成のための補助処理
*******************************************************************************/
#ifndef EBCSCRIPT_PARSER_STATEMENT
#define EBCSCRIPT_PARSER_STATEMENT

#include "trnsunit.h"
#include "name.h"
#include "slist.h"
#include "hashmap.h"
#include "boolean.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                                 構造体宣言 */
/* -------------------------------------------------------------------------- */
enum ebcscript_parser_blockstack_kind {
	EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT,
	EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK,
	EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION,
	EBCSCRIPT_PARSER_BLOCKSTACK_KIND_IF,
	EBCSCRIPT_PARSER_BLOCKSTACK_KIND_SWITCH,
	EBCSCRIPT_PARSER_BLOCKSTACK_KIND_LOOP,
};

/* 翻訳単位ブロック */
typedef struct ebcscript_parser_blockstack_trnsunit
                                           ebcscript_parser_blockstack_trnsunit;
struct ebcscript_parser_blockstack_trnsunit {
	ebcscript_trnsunit *TU;
};

/* ブロック */
typedef struct ebcscript_parser_blockstack_block
                                              ebcscript_parser_blockstack_block;
struct ebcscript_parser_blockstack_block {
	/* ローカル変数割付け用 */
	void *DP;

	/* バックパッチリスト：フレームサイズ *//* 実際1箇所だけ */
	int FrameSize;
	slist/*<void *>*/ *UnresolvedFrameSizeList;

	/* ブロックスコープ */
	hashmap *NSVarFuncTypeEnum;
	hashmap *NSTag;
/*	slist<ebcscript_unresolved *> *UnresolvedList;*/
};

/* 関数ブロック */
typedef struct ebcscript_parser_blockstack_function
                                           ebcscript_parser_blockstack_function;
struct ebcscript_parser_blockstack_function {
	ebcscript_type *ReturnValue;
	size_t ReturnValueSize;
	size_t ParametersSize;

	/* バックパッチリスト：return */
	ebcscript_name *LReturn;
	slist/*<ebcscript_unresolved *>*/ *UnresolvedReturnList;

	/* goto用 */
	hashmap *LGoto;
	slist/*<ebcscript_unresolved *>*/ *UnresolvedGotoList;
};

/* ifブロック */
typedef struct ebcscript_parser_blockstack_if ebcscript_parser_blockstack_if;
struct ebcscript_parser_blockstack_if {
	/* バックパッチリスト：endif, else */
	ebcscript_name *LEndif;
	ebcscript_name *LElse;
	slist/*<ebcscript_unresolved *>*/ *UnresolvedList;
};

/* switchブロック */
typedef struct ebcscript_parser_blockstack_switch
                                             ebcscript_parser_blockstack_switch;
struct ebcscript_parser_blockstack_switch {
	/* バックパッチリスト：break, switchの分岐 */
	ebcscript_name *LBreak;
	ebcscript_name *LBegin;
	ebcscript_name *LDefault;	/* defaultラベル */
	slist/*<ebcscript_name *>*/ *LCase;

	ebcscript_name *NSelector;/* 式の値を保存するローカル変数のアドレス */

	slist/*<ebcscript_unresolved *>*/ *UnresolvedList;
};

/* while, do-while, forブロック */
typedef struct ebcscript_parser_blockstack_loop
                                               ebcscript_parser_blockstack_loop;
struct ebcscript_parser_blockstack_loop {
	/* バックパッチリスト：break, continue, forの先頭 */
	ebcscript_name *LBreak;
	ebcscript_name *LContinue;
	ebcscript_name *LBegin;
	slist/*<ebcscript_unresolved *>*/ *UnresolvedList;
};

/* ブロックスタック */
typedef struct ebcscript_parser_blockstack ebcscript_parser_blockstack;
struct ebcscript_parser_blockstack {
	ebcscript_parser_blockstack *Prev;
	enum ebcscript_parser_blockstack_kind Kind;
	int Nest;

	union {
	  ebcscript_parser_blockstack_trnsunit Trnsunit;
	  ebcscript_parser_blockstack_function Function;
	  ebcscript_parser_blockstack_block Block;
	  ebcscript_parser_blockstack_if If;
	  ebcscript_parser_blockstack_switch Switch;
	  ebcscript_parser_blockstack_loop Loop;
	} As;
/*
	void (*resolve)(ebcscript_parser *, ebcscript_parser_blockstack *);
	void (*store_address_n)
	  (ebcscript_parser *, ebcscript_parser_blockstack *, ebcscript_name *);
	ebcscript_name (**findVarFuncTypeEnum)
	                           (ebcscript_parser_blockstack *, char *);
	ebcscript_name (**addTag)  (ebcscript_parser_blockstack *, char *);
	ebcscript_name (**addLabel)(ebcscript_parser_blockstack *, char *);

	boolean (*addVarFuncTypeEnum)
	                   (ebcscript_parser_blockstack *, ebcscript_name *);
	boolean (*addTag)  (ebcscript_parser_blockstack *, ebcscript_name *);
	boolean (*addLabel)(ebcscript_parser_blockstack *, ebcscript_name *);
*/
};

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
extern FILE *Ebcscript_Parser_Statement_Fplog;
extern size_t Ebcscript_Parser_Statement_MallocTotal;
extern void *(*Ebcscript_Parser_Statement_malloc)(size_t Size);
extern void (*Ebcscript_Parser_Statement_free)(void *);
extern void (*Ebcscript_Parser_Statement_exit)(int);
extern void (*Ebcscript_Parser_Statement_log)(const char *Fmt, ...);

/*                                                                   関数宣言 */
/* -------------------------------------------------------------------------- */
void Ebcscript_Parser_Statement_onexit(void);

ebcscript_parser_blockstack *Ebcscript_Parser_newBlockstack(void);
ebcscript_parser_blockstack *Ebcscript_Parser_newBlockstack_if(void);
ebcscript_parser_blockstack *Ebcscript_Parser_newBlockstack_loop(void);
ebcscript_parser_blockstack *Ebcscript_Parser_newBlockstack_switch(void);
ebcscript_parser_blockstack *Ebcscript_Parser_newBlockstack_function(void);
ebcscript_parser_blockstack *Ebcscript_Parser_newBlockstack_block(void);
ebcscript_parser_blockstack *Ebcscript_Parser_newBlockstack_trnsunit(void);
void Ebcscript_Parser_deleteBlockstack(ebcscript_parser_blockstack *);
void Ebcscript_Parser_deleteBlockstack_if(ebcscript_parser_blockstack *);
void Ebcscript_Parser_deleteBlockstack_loop(ebcscript_parser_blockstack *);
void Ebcscript_Parser_deleteBlockstack_switch(ebcscript_parser_blockstack *);
void Ebcscript_Parser_deleteBlockstack_function(ebcscript_parser_blockstack *);
void Ebcscript_Parser_deleteBlockstack_block(ebcscript_parser_blockstack *);
void Ebcscript_Parser_deleteBlockstack_trnsunit(ebcscript_parser_blockstack *);

void Ebcscript_Parser_Blockstack_store_int_block(
                                       ebcscript_parser_blockstack *BS, int *V);

void Ebcscript_Parser_Blockstack_store_address_n_trnsunit(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);

void *Ebcscript_Parser_Blockstack_mallocL(
                                  ebcscript_parser_blockstack *BS, size_t Size);

ebcscript_name **Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_if(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_loop(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_switch(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_function(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_block(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_trnsunit(
                                    ebcscript_parser_blockstack *BS, char *Key);

ebcscript_name **Ebcscript_Parser_Blockstack_findTag(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findTag_if(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findTag_loop(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findTag_switch(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findTag_function(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findTag_block(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findTag_trnsunit(
                                    ebcscript_parser_blockstack *BS, char *Key);

ebcscript_name **Ebcscript_Parser_Blockstack_findLabel(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findLabel_if(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findLabel_loop(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findLabel_switch(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findLabel_function(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findLabel_block(
                                    ebcscript_parser_blockstack *BS, char *Key);
ebcscript_name **Ebcscript_Parser_Blockstack_findLabel_trnsunit(
                                    ebcscript_parser_blockstack *BS, char *Key);

boolean Ebcscript_Parser_Blockstack_addVarFuncTypeEnum(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_if(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_loop(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_switch(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_function(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_block(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_trnsunit(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);

boolean Ebcscript_Parser_Blockstack_addTag(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addTag_if(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addTag_loop(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addTag_switch(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addTag_function(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addTag_block(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addTag_trnsunit(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);

boolean Ebcscript_Parser_Blockstack_addLabel(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addLabel_if(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addLabel_loop(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addLabel_switch(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addLabel_function(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addLabel_block(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);
boolean Ebcscript_Parser_Blockstack_addLabel_trnsunit(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N);

#endif
