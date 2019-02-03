/*******************************************************************************
   Project : C script
   File    : trnsunit.h
   Date    : 2018.6.26-
   Note    : 翻訳単位
*******************************************************************************/
#ifndef EBCSCRIPT_TRANSUNIT
#define EBCSCRIPT_TRANSUNIT

#include "name.h"
#include "code.h"
#include "slist.h"
#include "hashmap.h"
#include "boolean.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */
#define EBCSCRIPT_TRNSUNIT_CODESIZE_INITIAL 256

/*                                                                 構造体宣言 */
/* -------------------------------------------------------------------------- */
/* 翻訳単位 */
typedef struct ebcscript_trnsunit ebcscript_trnsunit;
struct ebcscript_trnsunit {
	char *Filename;

	/* 中間コード格納領域 */
	void *Code;	/* 可変サイズ */
	int CodeSize;
	void *CP;	/* コード生成時の作業用 */

	/* ファイルスコープ */
	hashmap *NSVarFuncTypeEnum;
	hashmap *NSTag;

	/* 定数表 */
	/* "123", "12.34e10", "Hello" */
	hashmap/*<ebcscript_literal *>*/ *ConstTbl;

	/* アドレス未解決リスト */
	slist/*<ebcscript_unresolved *>*/ *UnresolvedList;

	/* メモリ割付けの全記録 */
	slist/*<void *>*/ *AllocatedList;
};

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
extern FILE *Ebcscript_Trnsunit_Fplog;
extern size_t Ebcscript_Trnsunit_MallocTotal;
extern void *(*Ebcscript_Trnsunit_malloc)(size_t Size);
extern void (*Ebcscript_Trnsunit_free)(void *);
extern void (*Ebcscript_Trnsunit_exit)(int);
extern void (*Ebcscript_Trnsunit_log)(const char *Fmt, ...);

/*                                                                   関数宣言 */
/* -------------------------------------------------------------------------- */
void Ebcscript_Trnsunit_onexit(void);

ebcscript_trnsunit *Ebcscript_newTrnsunit(char *Filename);
void Ebcscript_deleteTrnsunit(ebcscript_trnsunit *);

void *Ebcscript_Trnsunit_mallocG(ebcscript_trnsunit *, size_t);

/* 中間コード領域を1024バイト増やす */
void Ebcscript_Trnsunit_resizeCode(ebcscript_trnsunit *);
void Ebcscript_Trnsunit_dumpCode(ebcscript_trnsunit *);

void Ebcscript_Trnsunit_dumpUnresolvedList(ebcscript_trnsunit *);
void Ebcscript_Trnsunit_dumpConstTbl(ebcscript_trnsunit *);
void Ebcscript_Trnsunit_dumpNS(ebcscript_trnsunit *);
void Ebcscript_Trnsunit_dump(ebcscript_trnsunit *);

/* static変数・関数、及びextern変数・関数は無視する。
   見つからなければNULLを返す。 */
ebcscript_name **Ebcscript_Trnsunit_findVarFuncTypeEnum_global(
                                             ebcscript_trnsunit *TU, char *Key);

/* コード領域オーバーフローのチェックが主な目的。
   オーバーフローの場合は、コード領域を拡張する（未実装）。 
  (1)命令を格納するもの。
  (2)アドレスを格納するもの。
  (3)各型ごとの即値を格納するもの。 */
void Ebcscript_Trnsunit_store_instruction(ebcscript_trnsunit *TU,
                                                    ebcscript_instruction Inst);
void Ebcscript_Trnsunit_store_address(ebcscript_trnsunit *TU, void *P);
void Ebcscript_Trnsunit_store_address_n(ebcscript_trnsunit *TU,
                                                             ebcscript_name *N);
void Ebcscript_Trnsunit_store_char(ebcscript_trnsunit *TU, char C);
void Ebcscript_Trnsunit_store_short(ebcscript_trnsunit *TU, short S);
void Ebcscript_Trnsunit_store_int(ebcscript_trnsunit *TU, int I);
void Ebcscript_Trnsunit_store_long(ebcscript_trnsunit *TU, long L);
void Ebcscript_Trnsunit_store_float(ebcscript_trnsunit *TU, float F);
void Ebcscript_Trnsunit_store_double(ebcscript_trnsunit *TU, double D);

boolean Ebcscript_Trnsunit_resolve(ebcscript_trnsunit *TU);

#endif
