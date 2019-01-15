/*******************************************************************************
   Project : C script
   File    : init.h
   Date    : 2018.7.19-
   Note    : 初期化子のための補助処理
*******************************************************************************/
#ifndef EBCSCRIPT_PARSER_INITIALIZER
#define EBCSCRIPT_PARSER_INITIALIZER

#include "name.h"
#include "btree.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                                 構造体宣言 */
/* -------------------------------------------------------------------------- */

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
extern FILE *Ebcscript_Parser_Initializer_Fplog;
extern size_t Ebcscript_Parser_Initializer_MallocTotal;
extern void *(*Ebcscript_Parser_Initializer_malloc)(size_t Size);
extern void (*Ebcscript_Parser_Initializer_free)(void *);
extern void (*Ebcscript_Parser_Initializer_exit)(int);
extern void (*Ebcscript_Parser_Initializer_log)(const char *Fmt, ...);

/*                                                                   関数宣言 */
/* -------------------------------------------------------------------------- */
void Ebcscript_Parser_Initializer_onexit(void);

/* 初期化子リストは、btree.Rightで伸ばす。
   入れ子の{}が現れたら、btree.DatumをNULLにして、btree.Leftに分岐する。 */

void Ebcscript_Parser_deleteInitializer(btree *Initializer);

#endif
