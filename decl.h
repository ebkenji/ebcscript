/*******************************************************************************
   Project : C script
   File    : decl.h
   Date    : 2018.6.4-
   Note    : 宣言の解析のための補助処理
*******************************************************************************/
#ifndef EBCSCRIPT_PARSER_DECLARATION
#define EBCSCRIPT_PARSER_DECLARATION

#include "boolean.h"
#include "name.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                                 構造体宣言 */
/* -------------------------------------------------------------------------- */
enum ebcscript_parser_declaration_specifier {
	EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_TYPEDEF = 1,
	EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_EXTERN,
	EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_STATIC,

	EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_VOID,
	EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_CHAR,
	EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_INT,
	EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_FLOAT,
	EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_DOUBLE,

	EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_SHORT,
	EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_LONG,

	EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_SIGNED,
	EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_UNSIGNED,
};

typedef struct ebcscript_parser_declaration ebcscript_parser_declaration;
struct ebcscript_parser_declaration {
	int StorageClass;	/* {0, TYPEDEF, EXTERN, STATIC} */
	int PrimitiveType;	/* {0, VOID, CHAR, INT, FLOAT, DOUBLE} */
	int ShortOrLong;	/* {0, SHORT, LONG} */
	int SignedOrUnsigned;	/* {0, SIGNED, UNSIGNED} */
	ebcscript_type *Type;	/* {NULL, &typetree} */
	boolean IsConst, IsVolatile;
};

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
extern FILE *Ebcscript_Parser_Declaration_Fplog;
extern size_t Ebcscript_Parser_Declaration_MallocTotal;
extern void *(*Ebcscript_Parser_Declaration_malloc)(size_t Size);
extern void (*Ebcscript_Parser_Declaration_free)(void *);
extern void (*Ebcscript_Parser_Declaration_exit)(int);
extern void (*Ebcscript_Parser_Declaration_log)(const char *Fmt, ...);

/*                                                                   関数宣言 */
/* -------------------------------------------------------------------------- */
void Ebcscript_Parser_Declaration_onexit(void);

ebcscript_parser_declaration *Ebcscript_Parser_newDeclaration(void);
void Ebcscript_Parser_deleteDeclaration(ebcscript_parser_declaration *);

/* 宣言指定子についての情報Dから型表現木を作る */
ebcscript_type *Ebcscript_Parser_Declaration_toTypeTree(
                                             ebcscript_parser_declaration *D);

#endif
