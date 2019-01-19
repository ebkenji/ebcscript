/*******************************************************************************
   Project : C script
   File    : ebcscrip.h
   Date    : 2018.4.4
   Note    : 
*******************************************************************************/
#ifndef EBCSCRIPT
#define EBCSCRIPT

#include "trnsunit.h"
#include "parser.h"
#include "boolean.h"
#include "hashmap.h"
#include "slist.h"
#include "name.h"
#include "code.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                                 構造体宣言 */
/* -------------------------------------------------------------------------- */
/* 実行環境 */
typedef struct ebcscript ebcscript;
struct ebcscript {
	slist/*<ebcscript_transunit *>*/ *Trnsunits;
/*	trnsunit *Native;*/
	boolean IsResolved;	/* 全翻訳単位の名前解決済み */

	/* 実行時の作業用 */
	void *CP;
	void *BP;
	ebcscript_stack *Stack;	/* 固定サイズ。new時に引数でサイズを指定 */
};

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
extern FILE *Ebcscript_Fplog;
extern size_t Ebcscript_MallocTotal;
extern void *(*Ebcscript_malloc)(size_t Size);
extern void (*Ebcscript_free)(void *);
extern void (*Ebcscript_exit)(int);
extern void (*Ebcscript_log)(const char *Fmt, ...);

/*                                                                   関数宣言 */
/* -------------------------------------------------------------------------- */
ebcscript *newEbcscript(int StackSize);
void deleteEbcscript(ebcscript *);

boolean Ebcscript_addTrnsunit(ebcscript *Env, char *Filename);
boolean Ebcscript_removeTrnsunit(ebcscript *Env, char *Filename);

/* ログファイル */
void Ebcscript_setFplog(FILE *Fp);

/* 全翻訳単位の記号表、中間コードを表示 */
void Ebcscript_dump(ebcscript *Env);

/* IsResolvedをtrueにする唯一の手段 */
boolean Ebcscript_resolve(ebcscript *Env);

/* 実行 */
boolean Ebcscript_call(ebcscript *Env, char *FuncName);

/* 変数参照 */
void *Ebcscript_address(ebcscript *Env, char *VarName);

/* スタック操作 */
void Ebcscript_push_char(ebcscript *Env, char C);
void Ebcscript_push_short(ebcscript *Env, short S);
void Ebcscript_push_int(ebcscript *Env, int I);
void Ebcscript_push_long(ebcscript *Env, long L);
void Ebcscript_push_uchar(ebcscript *Env, unsigned char UC);
void Ebcscript_push_ushort(ebcscript *Env, unsigned short US);
void Ebcscript_push_uint(ebcscript *Env, unsigned int UI);
void Ebcscript_push_ulong(ebcscript *Env, unsigned long UL);
void Ebcscript_push_float(ebcscript *Env, float F);
void Ebcscript_push_double(ebcscript *Env, double D);
void Ebcscript_push_address(ebcscript *Env, void *P);
void Ebcscript_pop_char(ebcscript *Env, char *C);
void Ebcscript_pop_short(ebcscript *Env, short *S);
void Ebcscript_pop_int(ebcscript *Env, int *I);
void Ebcscript_pop_long(ebcscript *Env, long *L);
void Ebcscript_pop_uchar(ebcscript *Env, unsigned char *UC);
void Ebcscript_pop_ushort(ebcscript *Env, unsigned short *US);
void Ebcscript_pop_uint(ebcscript *Env, unsigned int *UI);
void Ebcscript_pop_ulong(ebcscript *Env, unsigned long *UL);
void Ebcscript_pop_float(ebcscript *Env, float *F);
void Ebcscript_pop_double(ebcscript *Env, double *D);
void Ebcscript_pop_address(ebcscript *Env, void **P);
void Ebcscript_add_sp(ebcscript *Env, int I);
void Ebcscript_sub_sp(ebcscript *Env, int I);

#endif
