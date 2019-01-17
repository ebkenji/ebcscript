/*******************************************************************************
   Project : C script
   File    : expr.h
   Date    : 2018.6.4-
   Note    : 式の解析のための補助処理
*******************************************************************************/
#ifndef EBCSCRIPT_PARSER_EXPRESSION
#define EBCSCRIPT_PARSER_EXPRESSION

#include "boolean.h"
#include "name.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                                 構造体宣言 */
/* -------------------------------------------------------------------------- */
enum ebcscript_parser_expression_kind {
	EBCSCRIPT_PARSER_EXPRESSION_KIND_ID,	/* 識別子 */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_CONST,	/* 定数 */

	EBCSCRIPT_PARSER_EXPRESSION_KIND_FUNC,	/* () */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_INDEX,	/* [] */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_DOT,	/* . */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_ARROW,	/* -> */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_POST,	/* 後置++ */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_POST,	/* 後置-- */

	EBCSCRIPT_PARSER_EXPRESSION_KIND_ADDR,	/* & */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_PTR,	/* * */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_UPLUS,	/* + */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_UMINUS,	/* - */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_NOT,	/* ~ */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_NOT,	/* ! */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_PRE,	/* 前置++ */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_PRE,	/* 前置-- */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_EXPR,	/* sizeof expr */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_TYPE,	/* sizeof(type) */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_CAST,	/* 型変換 */

	EBCSCRIPT_PARSER_EXPRESSION_KIND_MUL,	/* * */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_DIV,	/* / */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_MOD,	/* % */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_ADD,	/* + */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_SUB,	/* - */

	EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_L,	/* << */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_R,	/* >> */

	EBCSCRIPT_PARSER_EXPRESSION_KIND_LT,	/* < */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_GT,	/* > */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_LE,	/* <= */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_GE,	/* >= */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_EQ,	/* == */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_NE,	/* != */

	EBCSCRIPT_PARSER_EXPRESSION_KIND_AND,	/* & */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_XOR,	/* ^ */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_OR,	/* | */

	EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_AND,	/* && */
	EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_OR,	/* || */

	EBCSCRIPT_PARSER_EXPRESSION_KIND_COND,	/* ? */

	EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN,		/* = */
/*	EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_MUL,*/		/* *= */
/*	EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_DIV,*/		/* /= */
/*	EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_MOD,*/		/* %= */
/*	EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_ADD,*/		/* += */
/*	EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_SUB,*/		/* -= */
/*	EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_SHFT_L,*/	/* <<= */
/*	EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_SHFT_R,*/	/* >>= */
/*	EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_AND,*/		/* &= */
/*	EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_OR,*/		/* |= */
/*	EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_XOR,*/		/* ^= */

	EBCSCRIPT_PARSER_EXPRESSION_KIND_COMMA,	/* , */
};

typedef struct ebcscript_parser_expression ebcscript_parser_expression;
struct ebcscript_parser_expression {
	enum ebcscript_parser_expression_kind Kind;
	ebcscript_type *TypeTree;
	boolean IsConstant;

	union {
	  struct {
	    ebcscript_name *Name;
	  } Identifier;

	  struct {
	    ebcscript_literal *Literal;
	  } Constant;

	  struct {
	    ebcscript_parser_expression *Child;
	    btree/*<ebcscript_parser_expression *>*/ *Arguments;
	  } Func;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Index;

	  struct {
	    ebcscript_parser_expression *Child;
	    int Offset;
	  } Dot;

	  struct {
	    ebcscript_parser_expression *Child;
	    int Offset;
	  } Arrow;

	  struct {
	    ebcscript_parser_expression *Child;
	  } IncPost;

	  struct {
	    ebcscript_parser_expression *Child;
	  } DecPost;

	  struct {
	    ebcscript_parser_expression *Child;
	  } Addr;

	  struct {
	    ebcscript_parser_expression *Child;
	  } Ptr;

	  struct {
	    ebcscript_parser_expression *Child;
	  } UPlus;

	  struct {
	    ebcscript_parser_expression *Child;
	  } UMinus;

	  struct {
	    ebcscript_parser_expression *Child;
	  } Not;

	  struct {
	    ebcscript_parser_expression *Child;
	  } LogNot;

	  struct {
	    ebcscript_parser_expression *Child;
	  } IncPre;

	  struct {
	    ebcscript_parser_expression *Child;
	  } DecPre;

	  struct {
	    ebcscript_parser_expression *Child;
	    int Size;
	  } SizeofExpr;

	  struct {
	    ebcscript_type *TypeTree;
	    int Size;
	  } SizeofType;

	  struct {
	    ebcscript_parser_expression *Child;
	  } Cast;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Mul;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Div;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Mod;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Add;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Sub;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } ShftL;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } ShftR;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Lt;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Gt;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Le;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Ge;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Eq;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Ne;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } And;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Xor;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Or;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } LogAnd;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } LogOr;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Center;
	    ebcscript_parser_expression *Right;
	  } Cond;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Assign;
/*
	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } AssignMul;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } AssignDiv;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } AssignMod;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } AssignAdd;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } AssignSub;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } AssignShftL;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } AssignShftR;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } AssignAnd;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } AssignOr;

	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } AssignXor;
*/
	  struct {
	    ebcscript_parser_expression *Left;
	    ebcscript_parser_expression *Right;
	  } Comma;

	} As;

};

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
extern FILE *Ebcscript_Parser_Expression_Fplog;
extern size_t Ebcscript_Parser_Expression_MallocTotal;
extern void *(*Ebcscript_Parser_Expression_malloc)(size_t Size);
extern void (*Ebcscript_Parser_Expression_free)(void *);
extern void (*Ebcscript_Parser_Expression_exit)(int);
extern void (*Ebcscript_Parser_Expression_log)(const char *Fmt, ...);

/*                                                                   関数宣言 */
/* -------------------------------------------------------------------------- */
void Ebcscript_Parser_Expression_onexit(void);

/* 整数で0のときtrueを返す */
boolean Ebcscript_Parser_Expression_isZero(ebcscript_parser_expression *);

/* 左辺値を持つか？ */
boolean Ebcscript_Parser_Expression_isLvalue(ebcscript_parser_expression *);

/* 左辺値は相対アドレスか絶対アドレスか？ */
int Ebcscript_Parser_Expression_addressing(ebcscript_parser_expression *);

void Ebcscript_Parser_Expression_dump(ebcscript_parser_expression *);

ebcscript_parser_expression *Ebcscript_Parser_Expression_dup(
                                                 ebcscript_parser_expression *);

ebcscript_parser_expression *Ebcscript_Parser_newExpression(void);
void Ebcscript_Parser_deleteExpression(ebcscript_parser_expression *);

ebcscript_parser_expression *Ebcscript_Parser_newExpression_cast(
                             ebcscript_type *, ebcscript_parser_expression *);

#endif
