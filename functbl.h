/*******************************************************************************
   Project : C script
   File    : functbl.h
   Date    : 2019.1.26-
   Note    : 関数表とダミー関数の管理
*******************************************************************************/
#ifndef EBCSCRIPT_FUNCTIONMAP
#define EBCSCRIPT_FUNCTIONMAP

#include "boolean.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                                 構造体宣言 */
/* -------------------------------------------------------------------------- */
/* ダミー関数構造体 */
typedef struct ebcscript_dummyfunction ebcscript_dummyfunction;
struct ebcscript_dummyfunction {
	boolean InUse;
	void *FunctionID;
};

/* 関数表 */
typedef struct ebcscript_functionmap ebcscript_functionmap;
struct ebcscript_functionmap_entry {
	void *FunctionID;
	void *CodeAddress;
	boolean IsNative;
};

typedef struct ebcscript_functionmap_entry ebcscript_functionmap_entry;
struct ebcscript_functionmap {
	int TableSize;
	ebcscript_functionmap_entry *Table;
	int N;
};

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
extern FILE *Ebcscript_Functionmap_Fplog;
extern size_t Ebcscript_Functionmap_MallocTotal;
extern void *(*Ebcscript_Functionmap_malloc)(size_t Size);
extern void (*Ebcscript_Functionmap_free)(void *);
extern void (*Ebcscript_Functionmap_exit)(int);
extern void (*Ebcscript_Functionmap_log)(const char *Fmt, ...);

/*                                                                   関数宣言 */
/* -------------------------------------------------------------------------- */
void Ebcscript_Functionmap_onexit(void);

/* FunctionIDはコードセグメント上の絶対アドレスを表し、
   1つの関数に対して一意になる。関数を識別する目的で使用する */

void *Ebcscript_Dummyfunction_getDummy(void);
void Ebcscript_Dummyfunction_releaseDummy(void *FunctionID);
boolean Ebcscript_Dummyfunction_isDummy(void *FunctionID);

ebcscript_functionmap *Ebcscript_newFunctionmap(void);
void Ebcscript_deleteFunctionmap(ebcscript_functionmap *);

ebcscript_functionmap_entry *Ebcscript_Functionmap_find(
                                 ebcscript_functionmap *FMap, void *FunctionID);
boolean Ebcscript_Functionmap_add(
 ebcscript_functionmap *FMap,
 void *FunctionID, void *CodeAddress, boolean IsNative
);
void Ebcscript_Functionmap_remove(
                                 ebcscript_functionmap *FMap, void *FunctionID);

#endif
