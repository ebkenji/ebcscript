/*******************************************************************************
   Project : C script
   File    : expr.c
   Date    : 2018.6.4-
   Note    : 式の解析のための補助処理
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include "parser.h"
#include "trnsunit.h"
#include "stmt.h"
#include "expr.h"
#include "name.h"
#include "code.h"
#include "btree.h"
#include "boolean.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Parser_Expression_malloc(size_t Size);
void _Ebcscript_Parser_Expression_free(void *P);
void _Ebcscript_Parser_Expression_exit(int Status);
void _Ebcscript_Parser_Expression_log(const char *Fmt, ...);

FILE *Ebcscript_Parser_Expression_Fplog/* = stdout*/;
size_t Ebcscript_Parser_Expression_MallocTotal = 0;
void *(*Ebcscript_Parser_Expression_malloc)(size_t Size) =
                                           &_Ebcscript_Parser_Expression_malloc;
void (*Ebcscript_Parser_Expression_free)(void *) =
                                             &_Ebcscript_Parser_Expression_free;
void (*Ebcscript_Parser_Expression_exit)(int) =
                                             &_Ebcscript_Parser_Expression_exit;
void (*Ebcscript_Parser_Expression_log)(const char *Fmt, ...) =
                                              &_Ebcscript_Parser_Expression_log;

/*                                                                   関数定義 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Parser_Expression_malloc(size_t Size)
{
	void *P;

	if ((P = malloc(Size + sizeof(size_t))) == NULL){
	  Ebcscript_Parser_Expression_log(
	   "Ebcscript_Parser_Expression_malloc(): "
	   " error: memory allocation error(total=%d bytes)\n",
	   Ebcscript_Parser_Expression_MallocTotal);
	  Ebcscript_Parser_Expression_exit(1);
	}
	*(size_t *)P = Size;
	Ebcscript_Parser_Expression_MallocTotal += Size;
	return (size_t *)P + 1;
}

void _Ebcscript_Parser_Expression_free(void *P)
{
	P = (size_t *)P - 1;
	Ebcscript_Parser_Expression_MallocTotal -= *(size_t *)P;
	free(P);
}

void Ebcscript_Parser_Expression_onexit(void)
{
	fclose(Ebcscript_Parser_Expression_Fplog);
}

void _Ebcscript_Parser_Expression_exit(int Status)
{
	Ebcscript_Parser_Expression_onexit();
	exit(Status);
}

void _Ebcscript_Parser_Expression_log(const char *Fmt, ...)
{
	va_list Args;

	fprintf(Ebcscript_Parser_Expression_Fplog,
	                                       "ebcscript_parser_expression: ");

	va_start(Args, Fmt);
	vfprintf(Ebcscript_Parser_Expression_Fplog, Fmt, Args);
	va_end(Args);
}
/* -------------------------------------------------------------------------- */
#define log_debug0(Message)						\
{									\
	Ebcscript_Parser_Expression_log(				\
	 "%s:%d: debug: %s:%d: " Message,				\
	 Prs->Filename, Prs->Line, __FILE__, __LINE__);			\
}

#define log_debug1(Message, Param1)					\
{									\
	Ebcscript_Parser_Expression_log(				\
	 "%s:%d: debug: %s:%d: " Message,				\
	 Prs->Filename, Prs->Line, __FILE__, __LINE__,			\
	 Param1);							\
}

#define log_error0(Message)						\
{									\
	Ebcscript_Parser_Expression_log(				\
	 "%s:%d: error: " Message,					\
	 Prs->Filename, Prs->Line);					\
}

#define log_error1(Message, Param1)					\
{									\
	Ebcscript_Parser_Expression_log(				\
	 "%s:%d: error: " Message,					\
	 Prs->Filename, Prs->Line,					\
	 Param1);							\
}

#define log_warning0(Message)						\
{									\
	Ebcscript_Parser_Expression_log(				\
	 "%s:%d: warning: " Message,					\
	 Prs->Filename, Prs->Line);					\
}

boolean Ebcscript_Parser_Expression_isZero(ebcscript_parser_expression *E)
{
	if (Ebcscript_Type_isInteger(E->TypeTree)
	 && E->Kind == EBCSCRIPT_PARSER_EXPRESSION_KIND_CONST
	 && E->As.Constant.Literal->Kind == EBCSCRIPT_LITERAL_KIND_INTEGER
	 && E->As.Constant.Literal->As.Integer == 0) {
	  return true;
	}
	return false;
}

boolean Ebcscript_Parser_Expression_isLvalue(ebcscript_parser_expression *E)
{
	switch (E->Kind) {
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ID:	return true;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CONST:		return false;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_FUNC:		return false;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INDEX:	return true;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DOT:	return true;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ARROW:	return true;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_POST:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_POST:	return false;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADDR:		return false;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_PTR:	return true;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UPLUS:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UMINUS:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NOT:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_NOT:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_PRE:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_PRE:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_EXPR:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_TYPE:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CAST:		return false;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MUL:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DIV:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MOD:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADD:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SUB:		return false;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_L:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_R:		return false;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LT:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GT:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LE:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GE:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_EQ:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NE:		return false;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_AND:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_XOR:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_OR:		return false;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_AND:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_OR:		return false;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COND:		return false;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN:		return false;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COMMA:		return false;
	  default:
	    break;
	}
	return false;
}

#define store_instruction(Inst)						\
	Ebcscript_Trnsunit_store_instruction(Prs->TU, Inst);

#define store_address(P)						\
	Ebcscript_Trnsunit_store_address(Prs->TU, P);

#define store_address_n(BS, N)						\
	Ebcscript_Parser_store_address_n(Prs, BS, N);

#define store_char(C)							\
	Ebcscript_Trnsunit_store_char(Prs->TU, C);

#define store_short(S)							\
	Ebcscript_Trnsunit_store_short(Prs->TU, S);

#define store_int(I)							\
	Ebcscript_Trnsunit_store_int(Prs->TU, I);

#define store_long(L)							\
	Ebcscript_Trnsunit_store_long( Prs->TU, L);

#define store_float(F)							\
	Ebcscript_Trnsunit_store_float(Prs->TU, F);

#define store_double(D)							\
	Ebcscript_Trnsunit_store_double(Prs->TU, D);

/* 型による場合分けを吸収することが目的。
   マクロ内において、store_instruction()関数のための引数（命令を表す列挙子）を
   構成する。
   マクロパラメータとして与えられた識別子（列挙子の一部, MULなど）に、
   型に応じた接尾辞を付加する。 */
#define gencode_op_storage(Op, TypeTree)				\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _C);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _S);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _I);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _L);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_FLOAT:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _F);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _D);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_POINTER:				\
	  case EBCSCRIPT_TYPE_KIND_ARRAY:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _P);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_STRUCT:				\
	  case EBCSCRIPT_TYPE_KIND_UNION:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _OBJECT);	\
	    store_int((int)Ebcscript_Type_getSize(TypeTree));		\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

#define gencode_op_integer(Op, TypeTree)				\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _C);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _UC);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _S);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _US);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _I);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _UI);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _L);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _UL);	\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

#define gencode_op_numeric(Op, TypeTree)				\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _C);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _UC);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _S);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _US);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _I);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _UI);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _L);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _UL);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_FLOAT:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _F);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _D);	\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

#define gencode_op_numeric_ptr(Op, TypeTree)				\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _C);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _UC);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _S);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _US);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _I);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _UI);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _L);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _UL);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_FLOAT:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _F);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _D);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_POINTER:				\
	  case EBCSCRIPT_TYPE_KIND_ARRAY:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _P);	\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

#define gencode_op_bit(Op, TypeTree)	\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _C);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _S);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _I);	\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    store_instruction(EBCSCRIPT_INSTRUCTION_ ## Op ## _L);	\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

#define gencode_zero_numeric_ptr(TypeTree)				\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_VOID:				\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    store_char(0);						\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    store_short(0);						\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    store_int(0);						\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    store_long(0);						\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_FLOAT:				\
	    store_float(0.0);						\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:				\
	    store_double(0.0);						\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_POINTER:				\
	  case EBCSCRIPT_TYPE_KIND_ARRAY:				\
	    store_address(NULL);					\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

void Ebcscript_Parser_gencode_expr_rv(
 ebcscript_parser *Prs,
 ebcscript_parser_expression *E
)
{
	ebcscript_name *N;
	ebcscript_parser_blockstack *BS;
	int I;
	int Addressing;
	int Nest;
	long Size;
	void *P, *P1, *P2;
	btree *BT;

	switch (E->Kind) {
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ID:
	    N = E->As.Identifier.Name;

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(
	                                                 BS, N->Identifier)) {
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_debug0("")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    switch (N->Kind) {
	      case EBCSCRIPT_NAME_KIND_TYPEDEF:
	        /* [parse.y]primary_expressionで除外される */
	        log_debug0("an invalid kind of name\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;

	      case EBCSCRIPT_NAME_KIND_VARIABLE:
	        Addressing = N->As.Variable.Addressing;
	        Nest       = N->As.Variable.Nest;
	        switch (N->As.Variable.TypeTree->Kind) {
	          case EBCSCRIPT_TYPE_KIND_VOID:
	            log_debug0("the type of a variable is never void\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;
	          case EBCSCRIPT_TYPE_KIND_CHAR:
	          case EBCSCRIPT_TYPE_KIND_UCHAR:
	          case EBCSCRIPT_TYPE_KIND_SHORT:
	          case EBCSCRIPT_TYPE_KIND_USHORT:
	          case EBCSCRIPT_TYPE_KIND_INT:
	          case EBCSCRIPT_TYPE_KIND_UINT:
	          case EBCSCRIPT_TYPE_KIND_LONG:
	          case EBCSCRIPT_TYPE_KIND_ULONG:
	          case EBCSCRIPT_TYPE_KIND_FLOAT:
	          case EBCSCRIPT_TYPE_KIND_DOUBLE:
	          case EBCSCRIPT_TYPE_KIND_POINTER:
	          case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ONSTACKFRAME) {
	              if (Prs->Nest - Nest == 0) {
	                gencode_op_storage(PUSH_LMEM, N->As.Variable.TypeTree)
	                store_address_n(BS, N);
	              }
	              if (Prs->Nest - Nest >= 1) {
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_BP)
	                for (; Nest < Prs->Nest; Nest++) {
	                  store_instruction(EBCSCRIPT_INSTRUCTION_LOAD_P)
	                }
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	                store_address_n(BS, N);
	                store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	                gencode_op_storage(LOAD, N->As.Variable.TypeTree)
	              }
	            }
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE) {
	              gencode_op_storage(PUSH_GMEM, N->As.Variable.TypeTree)
	              store_address_n(BS, N);
	            }
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_UNDEFINED) {
	              gencode_op_storage(PUSH_GMEM, N->As.Variable.TypeTree)
	              store_address_n(BS, N);
	            }
	            break;
	          case EBCSCRIPT_TYPE_KIND_ARRAY:/* 左辺値 */
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ONSTACKFRAME) {
	              if (Prs->Nest - Nest == 0) {
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_BP)
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	                store_address_n(BS, N);
	                store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	              }
	              if (Prs->Nest - Nest >= 1) {
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_BP)
	                for (; Nest < Prs->Nest; Nest++) {
	                  store_instruction(EBCSCRIPT_INSTRUCTION_LOAD_P)
	                }
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	                store_address_n(BS, N);
	                store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	              }
	            }
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE) {
	              store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_P)
	              store_address_n(BS, N);
	            }
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_UNDEFINED) {
	              store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_P)
	              store_address_n(BS, N);
	            }
	            break;
	          case EBCSCRIPT_TYPE_KIND_STRUCT:
	          case EBCSCRIPT_TYPE_KIND_UNION:
 	            Size = Ebcscript_Type_getSize(N->As.Variable.TypeTree);
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ONSTACKFRAME) {
	              if (Prs->Nest - Nest == 0) {
	                store_instruction(
	                                 EBCSCRIPT_INSTRUCTION_PUSH_LMEM_OBJECT)
	                store_int(Size)	/* オペランドはサイズとアドレス */
	                store_address_n(BS, N)
	              }
	              if (Prs->Nest - Nest >= 1) {
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_BP)
	                for (; Nest < Prs->Nest; Nest++) {
	                  store_instruction(EBCSCRIPT_INSTRUCTION_LOAD_P)
	                }
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	                store_address_n(BS, N);
	                store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	                store_instruction(EBCSCRIPT_INSTRUCTION_LOAD_OBJECT)
	                store_int(Size);
	              }
	            }
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE) {
	              store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_GMEM_OBJECT)
	              store_int(Size);
	              store_address_n(BS, N);
	            }
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_UNDEFINED) {
	              store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_GMEM_OBJECT)
	              store_int(Size);
	              store_address_n(BS, N);
	            }
	            break;
	          case EBCSCRIPT_TYPE_KIND_FUNCTION:
	            log_debug0("the type of a variable is never function\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;
	          case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	            log_debug0("the type of a variable is never enumerator\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;
	          default:
	            log_debug0("an invalid type of a variable\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;
	        }
	        break;

	      case EBCSCRIPT_NAME_KIND_FUNCTION:
	        switch (N->As.Function.TypeTree->Kind) {
	          case EBCSCRIPT_TYPE_KIND_FUNCTION:
	            store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_P)
	            store_address_n(BS, N)
	            break;
	          default:
	            log_debug0("an invalid type of a function\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;
	        }
	        break;

	      case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	        switch (N->As.Enumerator.TypeTree->Kind) {
	          case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	            store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	            store_int(N->As.Enumerator.Value)
	            break;
	          default:
	            log_debug0("an invalid type of a enumerator\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;
	        }
	        break;

	      default:
	        log_debug0("an invalid kind of name\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CONST:
	    switch (E->As.Constant.Literal->Kind) {
	      case EBCSCRIPT_LITERAL_KIND_INTEGER:
	        store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_L)
	        store_long(E->As.Constant.Literal->As.Integer)
	        break;
	      case EBCSCRIPT_LITERAL_KIND_FLOATING:
	        store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_D)
	        store_double(E->As.Constant.Literal->As.Floating)
	        break;
	      case EBCSCRIPT_LITERAL_KIND_CHARACTER:
	        store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_C)
	        store_char(E->As.Constant.Literal->As.Character)
	        break;
	      case EBCSCRIPT_LITERAL_KIND_STRING:
	        store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_P)
	        store_address(E->As.Constant.Literal->As.String)
	        break;
	      default:
	        log_debug0("an invalid kind of literal\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_FUNC:
	    /* 戻り値領域の確保 */
	    Size = Ebcscript_Type_getSize(
	                   E->As.Func.Child->TypeTree->As.Function.ReturnValue);
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	    store_int(Size)
	    store_instruction(EBCSCRIPT_INSTRUCTION_SUB_SP)

	    /* 実引数プッシュ */
	    for (BT = E->As.Func.Arguments; BT != NULL; BT = BT->Right) {
	      Ebcscript_Parser_gencode_expr_rv(Prs,
	                            (ebcscript_parser_expression *)BT->Datum);
	    }

	    /* 関数アドレスプッシュ */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Func.Child);

	    /* コール */
	    store_instruction(EBCSCRIPT_INSTRUCTION_CALL_PTR)

	    /* SP調整 */
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	    store_int(E->As.Func.Child->TypeTree->As.Function.ParametersSize)
	    store_instruction(EBCSCRIPT_INSTRUCTION_ADD_SP)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INDEX:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Index.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Index.Right);
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_L)
	    Size = Ebcscript_Type_getSize(E->TypeTree);
	    store_long(Size)
	    store_instruction(EBCSCRIPT_INSTRUCTION_MUL_L)
	    store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	    switch (E->TypeTree->Kind) {
	      case EBCSCRIPT_TYPE_KIND_VOID:
	        log_debug0("the type of a variable is never void\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_CHAR:
	      case EBCSCRIPT_TYPE_KIND_UCHAR:
	      case EBCSCRIPT_TYPE_KIND_SHORT:
	      case EBCSCRIPT_TYPE_KIND_USHORT:
	      case EBCSCRIPT_TYPE_KIND_INT:
	      case EBCSCRIPT_TYPE_KIND_UINT:
	      case EBCSCRIPT_TYPE_KIND_LONG:
	      case EBCSCRIPT_TYPE_KIND_ULONG:
	      case EBCSCRIPT_TYPE_KIND_FLOAT:
	      case EBCSCRIPT_TYPE_KIND_DOUBLE:
	      case EBCSCRIPT_TYPE_KIND_POINTER:
	      case EBCSCRIPT_TYPE_KIND_ARRAY:
	      case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	        gencode_op_storage(LOAD, E->TypeTree)
	        			/* localかglobalかはLeft側で処理済み */
	        break;
	      case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	        log_debug0("the type of a variable is never enumerator\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_STRUCT:
	      case EBCSCRIPT_TYPE_KIND_UNION:
	        store_instruction(EBCSCRIPT_INSTRUCTION_LOAD_OBJECT)
	        store_long(Size)
	        break;
	      case EBCSCRIPT_TYPE_KIND_FUNCTION:
	        log_debug0("the type of a variable is never function\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      default:
	        log_debug0("an invalid type of a variable\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DOT:
	    Ebcscript_Parser_gencode_expr_lv(Prs, E->As.Dot.Child);
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	    store_int(E->As.Dot.Offset)
	    store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	    switch (E->TypeTree->Kind) {
	      case EBCSCRIPT_TYPE_KIND_VOID:
	        log_debug0("the type of a variable is never void\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_CHAR:
	      case EBCSCRIPT_TYPE_KIND_UCHAR:
	      case EBCSCRIPT_TYPE_KIND_SHORT:
	      case EBCSCRIPT_TYPE_KIND_USHORT:
	      case EBCSCRIPT_TYPE_KIND_INT:
	      case EBCSCRIPT_TYPE_KIND_UINT:
	      case EBCSCRIPT_TYPE_KIND_LONG:
	      case EBCSCRIPT_TYPE_KIND_ULONG:
	      case EBCSCRIPT_TYPE_KIND_FLOAT:
	      case EBCSCRIPT_TYPE_KIND_DOUBLE:
	      case EBCSCRIPT_TYPE_KIND_POINTER:
	      case EBCSCRIPT_TYPE_KIND_ARRAY:
	      case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	        gencode_op_storage(LOAD, E->TypeTree)
	        break;
	      case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	        log_debug0("the type of a variable is never enumerator\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_STRUCT:
	      case EBCSCRIPT_TYPE_KIND_UNION:
	        Size = Ebcscript_Type_getSize(E->TypeTree);
	        store_instruction(EBCSCRIPT_INSTRUCTION_LOAD_OBJECT)
	        store_long(Size)
	        break;
	      case EBCSCRIPT_TYPE_KIND_FUNCTION:
	        log_debug0("the type of a variable is never function\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      default:
	        log_debug0("an invalid type of a variable\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ARROW:	/* -> */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Arrow.Child);
								/*右辺値*/
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	    store_int(E->As.Arrow.Offset)
	    store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	    switch (E->TypeTree->Kind) {
	      case EBCSCRIPT_TYPE_KIND_VOID:
	        log_debug0("the type of a variable is never void\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_CHAR:
	      case EBCSCRIPT_TYPE_KIND_UCHAR:
	      case EBCSCRIPT_TYPE_KIND_SHORT:
	      case EBCSCRIPT_TYPE_KIND_USHORT:
	      case EBCSCRIPT_TYPE_KIND_INT:
	      case EBCSCRIPT_TYPE_KIND_UINT:
	      case EBCSCRIPT_TYPE_KIND_LONG:
	      case EBCSCRIPT_TYPE_KIND_ULONG:
	      case EBCSCRIPT_TYPE_KIND_FLOAT:
	      case EBCSCRIPT_TYPE_KIND_DOUBLE:
	      case EBCSCRIPT_TYPE_KIND_POINTER:
	      case EBCSCRIPT_TYPE_KIND_ARRAY:
	      case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	        gencode_op_storage(LOAD, E->TypeTree)
	        break;
	      case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	        log_debug0("the type of a variable is never enumerator\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_STRUCT:
	      case EBCSCRIPT_TYPE_KIND_UNION:
	        Size = Ebcscript_Type_getSize(E->TypeTree);
	        store_instruction(EBCSCRIPT_INSTRUCTION_LOAD_OBJECT)
	        store_long(Size)
	        break;
	      case EBCSCRIPT_TYPE_KIND_FUNCTION:
	        log_debug0("the type of a variable is never function\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      default:
	        log_debug0("an invalid type of a variable\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_POST:	/* 後置++ */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.IncPost.Child);
	    gencode_op_storage(DUP, E->TypeTree)

	    if (Ebcscript_Type_isPointer(E->TypeTree)) {
	      ebcscript_type *Value;

	      switch (E->TypeTree->Kind) {
	        case EBCSCRIPT_TYPE_KIND_ARRAY:
	          Value = E->TypeTree->As.Array.Value;
	          break;
	        case EBCSCRIPT_TYPE_KIND_POINTER:
	          Value = E->TypeTree->As.Pointer.Value;
	          break;
	      }
	      Size = Ebcscript_Type_getSize(Value);
	      store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_L)
	      store_long(Size)
	      store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	    } else {
	      gencode_op_numeric(INC, E->TypeTree)
	    }

	    Ebcscript_Parser_gencode_expr_lv(Prs, E->As.IncPost.Child);
	    gencode_op_storage(STORE, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_POST:	/* 後置-- */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.DecPost.Child);
	    gencode_op_storage(DUP, E->TypeTree)

	    if (Ebcscript_Type_isPointer(E->TypeTree)) {
	      ebcscript_type *Value;

	      switch (E->TypeTree->Kind) {
	        case EBCSCRIPT_TYPE_KIND_ARRAY:
	          Value = E->TypeTree->As.Array.Value;
	          break;
	        case EBCSCRIPT_TYPE_KIND_POINTER:
	          Value = E->TypeTree->As.Pointer.Value;
	          break;
	      }
	      Size = Ebcscript_Type_getSize(Value);
	      store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_L)
	      store_long(Size)
	      store_instruction(EBCSCRIPT_INSTRUCTION_SUB_P)
	    } else {
	      gencode_op_numeric(DEC, E->TypeTree)
	    }

	    Ebcscript_Parser_gencode_expr_lv(Prs, E->As.DecPost.Child);
	    gencode_op_storage(STORE, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADDR:	/* & */
	    Ebcscript_Parser_gencode_expr_lv(Prs, E->As.Addr.Child);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_PTR:	/* * */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Ptr.Child);
	    switch (E->TypeTree->Kind) {
	      case EBCSCRIPT_TYPE_KIND_VOID:
	        log_debug0("the type of a variable is never void\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_CHAR:
	      case EBCSCRIPT_TYPE_KIND_UCHAR:
	      case EBCSCRIPT_TYPE_KIND_SHORT:
	      case EBCSCRIPT_TYPE_KIND_USHORT:
	      case EBCSCRIPT_TYPE_KIND_INT:
	      case EBCSCRIPT_TYPE_KIND_UINT:
	      case EBCSCRIPT_TYPE_KIND_LONG:
	      case EBCSCRIPT_TYPE_KIND_ULONG:
	      case EBCSCRIPT_TYPE_KIND_FLOAT:
	      case EBCSCRIPT_TYPE_KIND_DOUBLE:
	      case EBCSCRIPT_TYPE_KIND_POINTER:
	      case EBCSCRIPT_TYPE_KIND_ARRAY:
	      case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	        gencode_op_storage(LOAD, E->TypeTree)
	        break;
	      case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	        log_debug0("the type of a variable is never enumerator\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_STRUCT:
	      case EBCSCRIPT_TYPE_KIND_UNION:
	        Size = Ebcscript_Type_getSize(E->TypeTree);
	        store_instruction(EBCSCRIPT_INSTRUCTION_LOAD_OBJECT)
	        store_long(Size)
	        break;
	      case EBCSCRIPT_TYPE_KIND_FUNCTION:
	        /* 関数ポインタ。Childの右辺値そのもの */
	        break;
	      default:
	        log_debug0("an invalid type of a variable\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UPLUS:
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UMINUS:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.UMinus.Child);
	    gencode_op_numeric(NEG, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NOT:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Not.Child);
	    gencode_op_bit(NOT, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_NOT:	/* ! */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.LogNot.Child);
	    gencode_op_storage(PUSH_IMM, E->As.LogNot.Child->TypeTree)
	    gencode_zero_numeric_ptr(E->As.LogNot.Child->TypeTree)
	    gencode_op_numeric_ptr(NE, E->As.LogNot.Child->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_PRE:	/* 前置++ */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.IncPre.Child);

	    if (Ebcscript_Type_isPointer(E->TypeTree)) {
	      ebcscript_type *Value;

	      switch (E->TypeTree->Kind) {
	        case EBCSCRIPT_TYPE_KIND_ARRAY:
	          Value = E->TypeTree->As.Array.Value;
	          break;
	        case EBCSCRIPT_TYPE_KIND_POINTER:
	          Value = E->TypeTree->As.Pointer.Value;
	          break;
	      }
	      Size = Ebcscript_Type_getSize(Value);
	      store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_L)
	      store_long(Size)
	      store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	    } else {
	      gencode_op_numeric(INC, E->TypeTree)
	    }

	    Ebcscript_Parser_gencode_expr_lv(Prs, E->As.IncPre.Child);
	    gencode_op_storage(ASSGN, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_PRE:	/* 前置-- */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.DecPre.Child);

	    if (Ebcscript_Type_isPointer(E->TypeTree)) {
	      ebcscript_type *Value;

	      switch (E->TypeTree->Kind) {
	        case EBCSCRIPT_TYPE_KIND_ARRAY:
	          Value = E->TypeTree->As.Array.Value;
	          break;
	        case EBCSCRIPT_TYPE_KIND_POINTER:
	          Value = E->TypeTree->As.Pointer.Value;
	          break;
	      }
	      Size = Ebcscript_Type_getSize(Value);
	      store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_L)
	      store_long(Size)
	      store_instruction(EBCSCRIPT_INSTRUCTION_SUB_P)
	    } else {
	      gencode_op_numeric(DEC, E->TypeTree)
	    }

	    Ebcscript_Parser_gencode_expr_lv(Prs, E->As.DecPre.Child);
	    gencode_op_storage(ASSGN, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_EXPR:/* sizeof expr */
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	    store_int(E->As.SizeofExpr.Size)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_TYPE:/* sizeof(type) */
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	    store_int(E->As.SizeofType.Size)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CAST:	/* 型変換 */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Cast.Child);
	    switch (E->As.Cast.Child->TypeTree->Kind) {
	      case EBCSCRIPT_TYPE_KIND_CHAR:
	        gencode_op_numeric_ptr(C_TO, E->TypeTree)
	        break;
	      case EBCSCRIPT_TYPE_KIND_UCHAR:
	        gencode_op_numeric_ptr(UC_TO, E->TypeTree)
	        break;
	      case EBCSCRIPT_TYPE_KIND_SHORT:
	        gencode_op_numeric_ptr(S_TO, E->TypeTree)
	        break;
	      case EBCSCRIPT_TYPE_KIND_USHORT:
	        gencode_op_numeric_ptr(US_TO, E->TypeTree)
	        break;
	      case EBCSCRIPT_TYPE_KIND_INT:
	      case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	      case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	        gencode_op_numeric_ptr(I_TO, E->TypeTree)
	        break;
	      case EBCSCRIPT_TYPE_KIND_UINT:
	        gencode_op_numeric_ptr(UI_TO, E->TypeTree)
	        break;
	      case EBCSCRIPT_TYPE_KIND_LONG:
	        gencode_op_numeric_ptr(L_TO, E->TypeTree)
	        break;
	      case EBCSCRIPT_TYPE_KIND_ULONG:
	        gencode_op_numeric_ptr(UL_TO, E->TypeTree)
	        break;
	      case EBCSCRIPT_TYPE_KIND_FLOAT:
	        gencode_op_numeric_ptr(F_TO, E->TypeTree)
	        break;
	      case EBCSCRIPT_TYPE_KIND_DOUBLE:
	        gencode_op_numeric_ptr(D_TO, E->TypeTree)
	        break;
	      case EBCSCRIPT_TYPE_KIND_POINTER:
	        gencode_op_numeric_ptr(P_TO, E->TypeTree)
	        break;
	      default:
	        log_debug0("an invalid type of a variable\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MUL:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Mul.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Mul.Right);
	    gencode_op_numeric(MUL, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DIV:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Div.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Div.Right);
	    gencode_op_numeric(DIV, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MOD:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Mod.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Mod.Right);
	    gencode_op_integer(MOD, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADD:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Add.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Add.Right);

	    if (Ebcscript_Type_isPointer(E->TypeTree)) {
	      ebcscript_type *Value;

	      switch (E->TypeTree->Kind) {
	        case EBCSCRIPT_TYPE_KIND_POINTER:
	          Value = E->TypeTree->As.Pointer.Value;
	          break;
	        case EBCSCRIPT_TYPE_KIND_ARRAY:
	          Value = E->TypeTree->As.Array.Value;
	          break;
	      }
	      store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_L)
	      Size = Ebcscript_Type_getSize(Value);
	      store_long(Size)
	      store_instruction(EBCSCRIPT_INSTRUCTION_MUL_L)
	    }

	    gencode_op_numeric_ptr(ADD, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SUB:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Sub.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Sub.Right);

	    if (Ebcscript_Type_isPointer(E->TypeTree)) {
	      ebcscript_type *Value;

	      store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_L)
	      switch (E->TypeTree->Kind) {
	        case EBCSCRIPT_TYPE_KIND_POINTER:
	          Value = E->TypeTree->As.Pointer.Value;
	          break;
	        case EBCSCRIPT_TYPE_KIND_ARRAY:
	          Value = E->TypeTree->As.Array.Value;
	          break;
	      }
	      Size = Ebcscript_Type_getSize(Value);
	      store_long(Size)
	      store_instruction(EBCSCRIPT_INSTRUCTION_MUL_L)
	    }

	    gencode_op_numeric_ptr(SUB, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_L:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.ShftL.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.ShftL.Right);
	    gencode_op_bit(SHL, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_R:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.ShftR.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.ShftR.Right);
	    gencode_op_bit(SHR, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LT:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Lt.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Lt.Right);
	    /* 演算の結果はEの型(=int)だが、演算はLeftまたはRightの型で行う */
/*	    gencode_op_numeric_ptr(LT, E->TypeTree)*/
	    gencode_op_numeric_ptr(LT, E->As.Lt.Left->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GT:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Gt.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Gt.Right);
	    gencode_op_numeric_ptr(GT, E->As.Gt.Left->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LE:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Le.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Le.Right);
	    gencode_op_numeric_ptr(LE, E->As.Le.Left->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GE:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Ge.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Ge.Right);
	    gencode_op_numeric_ptr(GE, E->As.Ge.Left->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_EQ:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Eq.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Eq.Right);
	    gencode_op_numeric_ptr(EQ, E->As.Eq.Left->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NE:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Ne.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Ne.Right);
	    gencode_op_numeric_ptr(NE, E->As.Ne.Left->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_AND:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.And.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.And.Right);
	    gencode_op_bit(AND, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_XOR:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Xor.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Xor.Right);
	    gencode_op_bit(XOR, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_OR:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Or.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Or.Right);
	    gencode_op_bit(OR, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_AND:	/* && */
	    /* Left != 0 */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.LogAnd.Left);
	    gencode_op_storage(PUSH_IMM, E->As.LogAnd.Left->TypeTree)
	    gencode_zero_numeric_ptr(E->As.LogAnd.Left->TypeTree)
	    gencode_op_numeric_ptr(NE, E->As.LogAnd.Left->TypeTree)
	    store_instruction(EBCSCRIPT_INSTRUCTION_DUP_I)
	    store_instruction(EBCSCRIPT_INSTRUCTION_JMPF_I)
	    P = (void *)(Prs->TU->CP - Prs->TU->Code);
	    store_address(NULL)

	    /* Right != 0 */
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_L)
	    store_long(sizeof(int))
	    store_instruction(EBCSCRIPT_INSTRUCTION_ADD_SP)
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.LogAnd.Right);
	    gencode_op_storage(PUSH_IMM, E->As.LogAnd.Right->TypeTree)
	    gencode_zero_numeric_ptr(E->As.LogAnd.Right->TypeTree)
	    gencode_op_numeric_ptr(NE, E->As.LogAnd.Right->TypeTree)

	    *(void **)((ptrdiff_t)P + Prs->TU->Code) =
	                   (void *)(Prs->TU->CP - Prs->TU->Code - (ptrdiff_t)P);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_OR:	/* || */
	    /* Left != 0 */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.LogOr.Left);
	    gencode_op_storage(PUSH_IMM, E->As.LogOr.Left->TypeTree)
	    gencode_zero_numeric_ptr(E->As.LogOr.Left->TypeTree)
	    gencode_op_numeric_ptr(NE, E->As.LogOr.Left->TypeTree)
	    store_instruction(EBCSCRIPT_INSTRUCTION_DUP_I)
	    store_instruction(EBCSCRIPT_INSTRUCTION_JMPT_I)
	    P = (void *)(Prs->TU->CP - Prs->TU->Code);
	    store_address(NULL)

	    /* Right != 0 */
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_L)
	    store_long(sizeof(int))
	    store_instruction(EBCSCRIPT_INSTRUCTION_ADD_SP)
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.LogOr.Right);
	    gencode_op_storage(PUSH_IMM, E->As.LogOr.Right->TypeTree)
	    gencode_zero_numeric_ptr(E->As.LogOr.Right->TypeTree)
	    gencode_op_numeric_ptr(NE, E->As.LogOr.Right->TypeTree)

	    *(void **)((ptrdiff_t)P + Prs->TU->Code) =
	                   (void *)(Prs->TU->CP - Prs->TU->Code - (ptrdiff_t)P);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COND:	/* ? */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Cond.Left);
	    gencode_op_storage(PUSH_IMM, E->As.Cond.Left->TypeTree)
	    gencode_zero_numeric_ptr(E->As.Cond.Left->TypeTree)
	    gencode_op_numeric_ptr(NE, E->As.Cond.Left->TypeTree)
	    store_instruction(EBCSCRIPT_INSTRUCTION_JMPF_I)
	    P1 = (void *)(Prs->TU->CP - Prs->TU->Code);
	    store_address(NULL)

	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Cond.Center);
	    store_instruction(EBCSCRIPT_INSTRUCTION_JMP)
	    P2 = (void *)(Prs->TU->CP - Prs->TU->Code);
	    store_address(NULL)

	    *(void **)((ptrdiff_t)P1 + Prs->TU->Code) =
	                  (void *)(Prs->TU->CP - Prs->TU->Code - (ptrdiff_t)P1);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Cond.Right);

	    *(void **)((ptrdiff_t)P2 + Prs->TU->Code) =
	                  (void *)(Prs->TU->CP - Prs->TU->Code - (ptrdiff_t)P2);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN:	/* = */
	    /* 右から左 */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Assign.Right);
	    Ebcscript_Parser_gencode_expr_lv(Prs, E->As.Assign.Left);
	    switch (E->TypeTree->Kind) {
	      case EBCSCRIPT_TYPE_KIND_VOID:
	        log_debug0("the type of a variable is never void\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_CHAR:
	      case EBCSCRIPT_TYPE_KIND_UCHAR:
	      case EBCSCRIPT_TYPE_KIND_SHORT:
	      case EBCSCRIPT_TYPE_KIND_USHORT:
	      case EBCSCRIPT_TYPE_KIND_INT:
	      case EBCSCRIPT_TYPE_KIND_UINT:
	      case EBCSCRIPT_TYPE_KIND_LONG:
	      case EBCSCRIPT_TYPE_KIND_ULONG:
	      case EBCSCRIPT_TYPE_KIND_FLOAT:
	      case EBCSCRIPT_TYPE_KIND_DOUBLE:
	      case EBCSCRIPT_TYPE_KIND_POINTER:
	      case EBCSCRIPT_TYPE_KIND_ARRAY:
	      case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	        gencode_op_storage(ASSGN, E->TypeTree)
	        break;
	      case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	        log_debug0("the type of a variable is never enumerator\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_STRUCT:
	      case EBCSCRIPT_TYPE_KIND_UNION:
	        Size = Ebcscript_Type_getSize(E->TypeTree);
	        store_instruction(EBCSCRIPT_INSTRUCTION_ASSGN_OBJECT)
	        store_int(Size)
	        break;
	      case EBCSCRIPT_TYPE_KIND_FUNCTION:
	        log_debug0("the type of a variable is never function\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      default:
	        log_debug0("an invalid type of a variable\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COMMA:	/* , */
	    /* 左から右に評価され、左の式の値は捨てられる。
	       結果の型と値は、右の被演算数の型と値である。 */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Comma.Left);
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	    store_int(Ebcscript_Type_getSize(E->As.Comma.Left->TypeTree))
	    store_instruction(EBCSCRIPT_INSTRUCTION_ADD_SP)
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Comma.Right);
	    break;

	  default:
	    log_debug0("an invalid kind of expression\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;
	}
}

void Ebcscript_Parser_gencode_expr_lv(
 ebcscript_parser *Prs,
 ebcscript_parser_expression *E
)
{
	ebcscript_name *N;
	ebcscript_parser_blockstack *BS;
	int I;
	int Addressing;
	int Nest;
	int Size;
	void *P;
	btree *BT;

	switch (E->Kind) {
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ID:
	    N = E->As.Identifier.Name;

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(
	                                                   BS, N->Identifier)) {
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_debug0("")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    switch (N->Kind) {
	      case EBCSCRIPT_NAME_KIND_TYPEDEF:
	        /* [parse.y]primary_expressionで除外される */
	        log_debug0("an invalid kind of name\n")
	        break;

	      case EBCSCRIPT_NAME_KIND_VARIABLE:
	        Addressing = N->As.Variable.Addressing;
	        Nest       = N->As.Variable.Nest;

	        switch (N->As.Variable.TypeTree->Kind) {
	          case EBCSCRIPT_TYPE_KIND_VOID:
	            log_debug0("the type of a variable is never void\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;

	          case EBCSCRIPT_TYPE_KIND_CHAR:
	          case EBCSCRIPT_TYPE_KIND_UCHAR:
	          case EBCSCRIPT_TYPE_KIND_SHORT:
	          case EBCSCRIPT_TYPE_KIND_USHORT:
	          case EBCSCRIPT_TYPE_KIND_INT:
	          case EBCSCRIPT_TYPE_KIND_UINT:
	          case EBCSCRIPT_TYPE_KIND_LONG:
	          case EBCSCRIPT_TYPE_KIND_ULONG:
	          case EBCSCRIPT_TYPE_KIND_FLOAT:
	          case EBCSCRIPT_TYPE_KIND_DOUBLE:
	          case EBCSCRIPT_TYPE_KIND_POINTER:
	          case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ONSTACKFRAME) {
	              if (Prs->Nest - Nest == 0) {
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_BP)
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	                store_address_n(BS, N)
	                store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	              }
	              if (Prs->Nest - Nest >= 1) {
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_BP)
	                for (; Nest < Prs->Nest; Nest++) {
	                  store_instruction(EBCSCRIPT_INSTRUCTION_LOAD_P)
	                }
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	                store_address_n(BS, N)
	                store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	              }
	            }
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE) {
	              store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_P)
	              store_address_n(BS, N);
	            }
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_UNDEFINED) {
	              store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_P)
	              store_address_n(BS, N);
	            }
	            break;

	          case EBCSCRIPT_TYPE_KIND_ARRAY:
	            log_debug0("lvalue of array variable does not exist\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;

	          case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	            log_debug0("the type of a variable is never enumerator\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;

	          case EBCSCRIPT_TYPE_KIND_STRUCT:
	          case EBCSCRIPT_TYPE_KIND_UNION:
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ONSTACKFRAME) {
	              if (Prs->Nest - Nest == 0) {
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_BP)
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	                store_address_n(BS, N)
	                store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	              }
	              if (Prs->Nest - Nest >= 1) {
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_BP)
	                for (; Nest < Prs->Nest; Nest++) {
	                  store_instruction(EBCSCRIPT_INSTRUCTION_LOAD_P)
	                }
	                store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	                store_address_n(BS, N)
	                store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	              }
	            }
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE) {
	              store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_P)
	              store_address_n(BS, N);
	            }
	            if (Addressing == EBCSCRIPT_NAME_ADDRESSING_UNDEFINED) {
	              store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_P)
	              store_address_n(BS, N);
	            }
	            break;

	          case EBCSCRIPT_TYPE_KIND_FUNCTION:
	            log_debug0("the type of a variable is never function\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;

	          default:
	            log_debug0("an invalid type of a variable\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;
	        }
	        break;

	      case EBCSCRIPT_NAME_KIND_FUNCTION:
	        switch (N->As.Function.TypeTree->Kind) {
	          case EBCSCRIPT_TYPE_KIND_FUNCTION:
	            store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_P)
	            store_address_n(BS, N)
	            break;
	          default:
	            log_debug0("an invalid type of a function\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;
	        }
	        break;

	      case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	      default:
	        log_debug0("invalid name kind\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CONST:
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_FUNC:
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INDEX:
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Index.Left);
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Index.Right);
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	    Size = Ebcscript_Type_getSize(E->TypeTree);
	    store_int(Size)
	    store_instruction(EBCSCRIPT_INSTRUCTION_MUL_I)
	    store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DOT:
	    Ebcscript_Parser_gencode_expr_lv(Prs, E->As.Dot.Child);
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	    store_int(E->As.Dot.Offset)
	    store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ARROW:	/* -> */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Arrow.Child);
								/*右辺値*/
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I)
	    store_int(E->As.Arrow.Offset)
	    store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_POST:	/* 後置++ */
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_POST:	/* 後置-- */
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADDR:	/* & */
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_PTR:	/* * */
	    /* Childの右辺値が左辺値になる */
	    Ebcscript_Parser_gencode_expr_rv(Prs, E->As.Ptr.Child);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UPLUS:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UMINUS:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NOT:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_NOT:	/* ! */
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_PRE:	/* 前置++ */
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_PRE:	/* 前置-- */
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_EXPR:	/* sizeof expr */
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_TYPE:	/* sizeof(type) */
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CAST:	/* 型変換 */
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MUL:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DIV:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MOD:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADD:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SUB:
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_L:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_R:
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LT:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GT:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LE:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GE:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_EQ:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NE:
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_AND:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_XOR:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_OR:
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_AND:	/* && */
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_OR:	/* || */
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COND:	/* ? */
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN:	/* = */
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COMMA:	/* , */
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  default:
	    log_debug0("an invalid kind of expression\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;
	}
}

#define eval_op1(Op, T, Type)						\
	    Ebcscript_Parser_pop_ ## Type(Prs, &(T));			\
	    Ebcscript_Parser_push_ ## Type(Prs, Op (T));

#define eval_op1_numeric(Op, TypeTree)					\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	    eval_op1(Op, C, char)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    eval_op1(Op, UC, uchar)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	    eval_op1(Op, S, short)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    eval_op1(Op, US, ushort)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    eval_op1(Op, I, int)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	    eval_op1(Op, UI, uint)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	    eval_op1(Op, L, long)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    eval_op1(Op, UL, ulong)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_FLOAT:				\
	    eval_op1(Op, F, float)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:				\
	    eval_op1(Op, D, double)					\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

#define eval_op1_bit(Op, TypeTree)					\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    eval_op1(Op, C, char)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    eval_op1(Op, S, short)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    eval_op1(Op, I, int)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    eval_op1(Op, L, long)					\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

#define eval_op2(Op, T, Type)						\
	    Ebcscript_Parser_pop_ ## Type(Prs, &(T ## 2));		\
	    Ebcscript_Parser_pop_ ## Type(Prs, &(T ## 1));		\
	    Ebcscript_Parser_push_ ## Type(Prs, (T ## 1) Op (T ## 2));

#define eval_op2_integer(Op, TypeTree)					\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	    eval_op2(Op, C, char)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    eval_op2(Op, UC, uchar)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	    eval_op2(Op, S, short)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    eval_op2(Op, US, ushort)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    eval_op2(Op, I, int)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	    eval_op2(Op, UI, uint)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	    eval_op2(Op, L, long)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    eval_op2(Op, UL, ulong)					\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

#define eval_op2_numeric(Op, TypeTree)					\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	    eval_op2(Op, C, char)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    eval_op2(Op, UC, uchar)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	    eval_op2(Op, S, short)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    eval_op2(Op, US, ushort)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    eval_op2(Op, I, int)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	    eval_op2(Op, UI, uint)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	    eval_op2(Op, L, long)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    eval_op2(Op, UL, ulong)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_FLOAT:				\
	    eval_op2(Op, F, float)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:				\
	    eval_op2(Op, D, double)					\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

/* Pの扱いが特殊 */
#define eval_add_sub_numeric_ptr(Op, TypeTree)				\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	    eval_op2(Op, C, char)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    eval_op2(Op, UC, uchar)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	    eval_op2(Op, S, short)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    eval_op2(Op, US, ushort)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    eval_op2(Op, I, int)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	    eval_op2(Op, UI, uint)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	    eval_op2(Op, L, long)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    eval_op2(Op, UL, ulong)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_FLOAT:				\
	    eval_op2(Op, F, float)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:				\
	    eval_op2(Op, D, double)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_POINTER:				\
	  case EBCSCRIPT_TYPE_KIND_ARRAY:				\
	    Ebcscript_Parser_pop_int(Prs, &I2);				\
	    Ebcscript_Parser_pop_address(Prs, &P1);			\
	    Ebcscript_Parser_push_address(Prs, P1 Op I2);		\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

#define eval_op2_bit(Op, TypeTree)					\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    eval_op2(Op, C, char)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    eval_op2(Op, S, short)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    eval_op2(Op, I, int)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    eval_op2(Op, L, long)					\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

#define eval_compare(Op, T, Type)					\
	    Ebcscript_Parser_pop_ ## Type(Prs, &(T ## 2));		\
	    Ebcscript_Parser_pop_ ## Type(Prs, &(T ## 1));		\
	    Ebcscript_Parser_push_int(Prs, (T ## 1) Op (T ## 2));

#define eval_compare_numeric_ptr(Op, TypeTree)				\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	    eval_compare(Op, C, char)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    eval_compare(Op, UC, uchar)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	    eval_compare(Op, S, short)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    eval_compare(Op, US, ushort)				\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    eval_compare(Op, I, int)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	    eval_compare(Op, UI, uint)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	    eval_compare(Op, L, long)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    eval_compare(Op, UL, ulong)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_FLOAT:				\
	    eval_compare(Op, F, float)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:				\
	    eval_compare(Op, D, double)					\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_POINTER:				\
	  case EBCSCRIPT_TYPE_KIND_ARRAY:				\
	    eval_compare(Op, P, address)				\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

#define eval_cast_t1_to_t2(T1, Type1, T2, Type2)			\
	Ebcscript_Parser_pop_ ## Type1(Prs, &(T1));			\
	(T2) = (Type2)(T1);						\
	Ebcscript_Parser_push_ ## Type2(Prs, T2);

#define eval_cast_t1_to_(TypeTree1, T2, Type2)				\
	switch ((TypeTree1)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	    eval_cast_t1_to_t2(C, char, T2, Type2)			\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    eval_cast_t1_to_t2(UC, uchar, T2, Type2)			\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	    eval_cast_t1_to_t2(S, short, T2, Type2)			\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    eval_cast_t1_to_t2(US, ushort, T2, Type2)			\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    eval_cast_t1_to_t2(I, int, T2, Type2)			\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	    eval_cast_t1_to_t2(UI, uint, T2, Type2)			\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	    eval_cast_t1_to_t2(L, long, T2, Type2)			\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    eval_cast_t1_to_t2(UL, ulong, T2, Type2)			\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_FLOAT:				\
	    eval_cast_t1_to_t2(F, float, T2, Type2)			\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:				\
	    eval_cast_t1_to_t2(D, double, T2, Type2)			\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_POINTER:				\
	    if (sizeof(address) > sizeof(Type2)) {			\
	      log_warning0(						\
	       "cast from pointer to integer of different size\n");	\
	      longjmp(Prs->CheckPoint, 1);				\
	    }								\
	    eval_cast_t1_to_t2(P, address, L, long)			\
	    eval_cast_t1_to_t2(L, long, T2, Type2)			\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

#define eval_zero_numeric_ptr(TypeTree)					\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    Ebcscript_Parser_push_char(Prs, 0);				\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    Ebcscript_Parser_push_short(Prs, 0);			\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    Ebcscript_Parser_push_int(Prs, 0);				\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    Ebcscript_Parser_push_long(Prs, 0);				\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_FLOAT:				\
	    Ebcscript_Parser_push_float(Prs, 0.0);			\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:				\
	    Ebcscript_Parser_push_double(Prs, 0.0);			\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_POINTER:				\
	  case EBCSCRIPT_TYPE_KIND_ARRAY:				\
	    Ebcscript_Parser_push_address(Prs, NULL);			\
	    break;							\
	  default:							\
	    log_debug0("an invalid kind of type\n")			\
	    longjmp(Prs->CheckPoint, 1);				\
	    break;							\
	}

void Ebcscript_Parser_eval_expr_rv(
 ebcscript_parser *Prs,
 ebcscript_parser_expression *E
)
{
	ebcscript_name *N;
	int Addressing;
	int NestLevel;
	int Size;
	btree *BT;
	char C, C1, C2;
	short S, S1, S2;
	int I, I1, I2;
	long L, L1, L2;
	unsigned char UC, UC1, UC2;
	unsigned short US, US1, US2;
	unsigned int UI, UI1, UI2;
	unsigned long UL, UL1, UL2;
	float F, F1, F2;
	double D, D1, D2;
	void *P, *Q, *P1, *P2;

	typedef void *address;	/* マクロ内でpush, pop関数名を生成するため */
	typedef unsigned char uchar;
	typedef unsigned short ushort;
	typedef unsigned int uint;
	typedef unsigned long ulong;

	switch (E->Kind) {
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ID:
	    N = E->As.Identifier.Name;

	    switch (N->Kind) {
	      case EBCSCRIPT_NAME_KIND_TYPEDEF:
	        /* [parse.y]primary_expressionで除外される */
	        log_debug0("an invalid kind of name\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;

	      case EBCSCRIPT_NAME_KIND_VARIABLE:
	        log_debug0("a variable in a constant expression\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;

	      case EBCSCRIPT_NAME_KIND_FUNCTION:
	        switch (N->As.Function.TypeTree->Kind) {
	          case EBCSCRIPT_TYPE_KIND_FUNCTION:
	            if (N->As.Function.Addressing !=
	                                   EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE) {
	              log_debug0(
	                    "an undefined function in a constant expression\n")
	              longjmp(Prs->CheckPoint, 1);
	            } else {
	              Ebcscript_Parser_push_address(Prs,
	                                            N->As.Function.CodeAddress);
	            }
	            break;
	          default:
	            log_debug0("an invalid type of a function\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;
	        }
	        break;

	      case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	        switch (N->As.Enumerator.TypeTree->Kind) {
	          case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	            Ebcscript_Parser_push_int(Prs, N->As.Enumerator.Value);
	            break;
	          default:
	            log_debug0("an invalid type of a enumerator\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;
	        }
	        break;

	      default:
	        log_debug0("an invalid kind of name\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CONST:
	    switch (E->As.Constant.Literal->Kind) {
	      case EBCSCRIPT_LITERAL_KIND_INTEGER:
	        Ebcscript_Parser_push_long(Prs,
	                                    E->As.Constant.Literal->As.Integer);
	        break;
	      case EBCSCRIPT_LITERAL_KIND_FLOATING:
	        Ebcscript_Parser_push_double(
	                              Prs, E->As.Constant.Literal->As.Floating);
	        break;
	      case EBCSCRIPT_LITERAL_KIND_CHARACTER:
	        Ebcscript_Parser_push_char(Prs,
	                                  E->As.Constant.Literal->As.Character);
	        break;
	      case EBCSCRIPT_LITERAL_KIND_STRING:
	        Ebcscript_Parser_push_address(Prs,
	                                     E->As.Constant.Literal->As.String);
	        break;
	      default:
	        break;
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_FUNC:
	    log_debug0("calling a function in a constant expression\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INDEX:
	    log_error0("initializer element is not constant\n");
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DOT:
	    log_error0("initializer element is not constant\n");
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ARROW:	/* -> */
	    log_error0("initializer element is not constant\n");
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_POST:	/* 後置++ */
	    log_error0("lvalue required as increment operand\n");
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_POST:	/* 後置-- */
	    log_error0("lvalue required as decrement operand\n");
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADDR:	/* & */
	    Ebcscript_Parser_eval_expr_lv(Prs, E->As.Addr.Child);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_PTR:	/* * */
	    log_error0("initializer element is not constant\n");
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UPLUS:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.UPlus.Child);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UMINUS:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.UMinus.Child);
	    eval_op1_numeric(-, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NOT:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Not.Child);
	    eval_op1_bit(~, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_NOT:	/* ! */
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.LogNot.Child);
	    eval_zero_numeric_ptr(E->As.LogNot.Child->TypeTree)
	    eval_op2_numeric(!=, E->As.LogNot.Child->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_PRE:	/* 前置++ */
	    log_error0("lvalue required as increment operand\n");
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_PRE:	/* 前置-- */
	    log_error0("lvalue required as decrement operand\n");
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_EXPR:
							/* sizeof expr */
	    Ebcscript_Parser_push_int(Prs, E->As.SizeofExpr.Size);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_TYPE:
							/* sizeof(type) */
	    Ebcscript_Parser_push_int(Prs, E->As.SizeofType.Size);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CAST:	/* 型変換 */
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Cast.Child);
	    switch (E->TypeTree->Kind) {
	      case EBCSCRIPT_TYPE_KIND_CHAR:
	        eval_cast_t1_to_(E->As.Cast.Child->TypeTree, C, char)
	        break;
	      case EBCSCRIPT_TYPE_KIND_UCHAR:
	        eval_cast_t1_to_(E->As.Cast.Child->TypeTree, UC, uchar)
	        break;
	      case EBCSCRIPT_TYPE_KIND_SHORT:
	        eval_cast_t1_to_(E->As.Cast.Child->TypeTree, S, short)
	        break;
	      case EBCSCRIPT_TYPE_KIND_USHORT:
	        eval_cast_t1_to_(E->As.Cast.Child->TypeTree, US, ushort)
	        break;
	      case EBCSCRIPT_TYPE_KIND_INT:
	      case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	      case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	        eval_cast_t1_to_(E->As.Cast.Child->TypeTree, I, int)
	        break;
	      case EBCSCRIPT_TYPE_KIND_UINT:
	        eval_cast_t1_to_(E->As.Cast.Child->TypeTree, UI, uint)
	        break;
	      case EBCSCRIPT_TYPE_KIND_LONG:
	        eval_cast_t1_to_(E->As.Cast.Child->TypeTree, L, long)
	        break;
	      case EBCSCRIPT_TYPE_KIND_ULONG:
	        eval_cast_t1_to_(E->As.Cast.Child->TypeTree, UL, ulong)
	        break;
	      case EBCSCRIPT_TYPE_KIND_FLOAT:
	        eval_cast_t1_to_(E->As.Cast.Child->TypeTree, F, float)
	        break;
	      case EBCSCRIPT_TYPE_KIND_DOUBLE:
	        eval_cast_t1_to_(E->As.Cast.Child->TypeTree, D, double)
	        break;
	      case EBCSCRIPT_TYPE_KIND_POINTER:
	        eval_cast_t1_to_(E->As.Cast.Child->TypeTree, L, long)
	        eval_cast_t1_to_t2(L, long, P, address)
	        break;
	      default:
	        log_debug0("invalid type kind\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MUL:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Mul.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Mul.Right);
	    eval_op2_numeric(*, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DIV:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Div.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Div.Right);
	    eval_op2_numeric(/, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MOD:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Mod.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Mod.Right);
	    eval_op2_integer(%, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADD:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Add.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Add.Right);
	    eval_add_sub_numeric_ptr(+, E->TypeTree)
	    break;
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SUB:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Sub.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Sub.Right);
	    eval_add_sub_numeric_ptr(-, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_L:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.ShftL.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.ShftL.Right);
	    eval_op2_bit(<<, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_R:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.ShftR.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.ShftR.Right);
	    eval_op2_bit(>>, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LT:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Lt.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Lt.Right);
	    eval_compare_numeric_ptr(<, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GT:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Gt.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Gt.Right);
	    eval_compare_numeric_ptr(>, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LE:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Le.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Le.Right);
	    eval_compare_numeric_ptr(<=, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GE:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Ge.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Ge.Right);
	    eval_compare_numeric_ptr(>=, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_EQ:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Eq.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Eq.Right);
	    eval_compare_numeric_ptr(==, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NE:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Ne.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Ne.Right);
	    eval_compare_numeric_ptr(!=, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_AND:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.And.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.And.Right);
	    eval_op2_bit(&, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_XOR:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Xor.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Xor.Right);
	    eval_op2_bit(^, E->TypeTree)
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_OR:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Or.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Or.Right);
	    eval_op2_bit(|, E->TypeTree)
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_AND:	/* && */
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.LogAnd.Left);
	    eval_zero_numeric_ptr(E->As.LogAnd.Left->TypeTree)
	    eval_op2_numeric(!=, E->As.LogAnd.Left->TypeTree)
	    Ebcscript_Parser_pop_int(Prs, &I);
	    if (I) {
	      Ebcscript_Parser_eval_expr_rv(Prs, E->As.LogAnd.Right);
	      eval_zero_numeric_ptr(E->As.LogAnd.Right->TypeTree)
	      eval_op2_numeric(!=, E->As.LogAnd.Right->TypeTree)
	    } else {
	      Ebcscript_Parser_push_int(Prs, I);
	    }
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_OR:	/* || */
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.LogOr.Left);
	    eval_zero_numeric_ptr(E->As.LogOr.Left->TypeTree)
	    eval_op2_numeric(!=, E->As.LogOr.Left->TypeTree)
	    Ebcscript_Parser_pop_int(Prs, &I);
	    if (!I) {
	      Ebcscript_Parser_eval_expr_rv(Prs, E->As.LogOr.Right);
	      eval_zero_numeric_ptr(E->As.LogOr.Right->TypeTree)
	      eval_op2_numeric(!=, E->As.LogOr.Right->TypeTree)
	    } else {
	      Ebcscript_Parser_push_int(Prs, I);
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COND:	/* ? */
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Cond.Left);
	    eval_zero_numeric_ptr(E->As.Cond.Left->TypeTree)
	    eval_op2_numeric(!=, E->As.Cond.Left->TypeTree)
	    Ebcscript_Parser_pop_int(Prs, &I);
	    if (I) {
	      Ebcscript_Parser_eval_expr_rv(Prs, E->As.Cond.Center);
	    } else {
	      Ebcscript_Parser_eval_expr_rv(Prs, E->As.Cond.Right);
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN:	/* = */
	    log_error0("initializer element is not constant\n");
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COMMA:	/* , */
	    log_debug0("an invalid kind of expression\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  default:
	    log_debug0("an invalid kind of expression\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;
	}
}

void Ebcscript_Parser_eval_expr_lv(
 ebcscript_parser *Prs,
 ebcscript_parser_expression *E
)
{
	ebcscript_name *N;
	int Addressing;
	int NestLevel;
	long Size;
	btree *BT;
	char C, C1, C2;
	short S, S1, S2;
	int I, I1, I2;
	long L, L1, L2;
	unsigned char UC, UC1, UC2;
	unsigned short US, US1, US2;
	unsigned int UI, UI1, UI2;
	unsigned long UL, UL1, UL2;
	float F, F1, F2;
	double D, D1, D2;
	void *P, *Q, *P1, *P2;

	typedef void *address;	/* マクロ内でpush, pop関数名を生成するため */
	typedef unsigned char uchar;
	typedef unsigned short ushort;
	typedef unsigned int uint;
	typedef unsigned long ulong;

	switch (E->Kind) {
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ID:
	    N = E->As.Identifier.Name;

	    switch (N->Kind) {
	      case EBCSCRIPT_NAME_KIND_TYPEDEF:
	        /* [parse.y]primary_expressionで除外される */
	        log_debug0("an invalid kind of name\n")
	        break;
	      case EBCSCRIPT_NAME_KIND_VARIABLE:
	        switch (N->As.Variable.TypeTree->Kind) {
	          case EBCSCRIPT_TYPE_KIND_VOID:
	            log_debug0("the type of a variable is never void\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;
	          case EBCSCRIPT_TYPE_KIND_CHAR:
	          case EBCSCRIPT_TYPE_KIND_UCHAR:
	          case EBCSCRIPT_TYPE_KIND_SHORT:
	          case EBCSCRIPT_TYPE_KIND_USHORT:
	          case EBCSCRIPT_TYPE_KIND_INT:
	          case EBCSCRIPT_TYPE_KIND_UINT:
	          case EBCSCRIPT_TYPE_KIND_LONG:
	          case EBCSCRIPT_TYPE_KIND_ULONG:
	          case EBCSCRIPT_TYPE_KIND_FLOAT:
	          case EBCSCRIPT_TYPE_KIND_DOUBLE:
	          case EBCSCRIPT_TYPE_KIND_POINTER:
	          case EBCSCRIPT_TYPE_KIND_ARRAY:
	          case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	            if (N->As.Variable.Addressing !=
	                                   EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE) {
	              log_debug0(
	                    "an undefined variable in a constant expression\n");
	              longjmp(Prs->CheckPoint, 1);
	            } else {
	              Ebcscript_Parser_push_address(Prs,
	                                                N->As.Variable.Address);
	            }
	            break;
	          case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	            log_debug0("the type of a variable is never enumerator\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;
	          case EBCSCRIPT_TYPE_KIND_STRUCT:
	          case EBCSCRIPT_TYPE_KIND_UNION:
	            if (N->As.Variable.Addressing !=
	                                   EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE) {
	              log_debug0(
	                    "an undefined variable in a constant expression\n");
	              longjmp(Prs->CheckPoint, 1);
	            } else {
	              Ebcscript_Parser_push_address(Prs,
	                                                N->As.Variable.Address);
	            }
	            break;
	          case EBCSCRIPT_TYPE_KIND_FUNCTION:
	            log_debug0("the type of a variable is never function\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;
	          default:
	            log_debug0("an invalid type of a variable\n")
	            longjmp(Prs->CheckPoint, 1);
	            break;
	        }
	        break;
	      case EBCSCRIPT_NAME_KIND_FUNCTION:
	      case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	      default:
	        log_debug0("an invalid kind of name\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CONST:
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_FUNC:
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INDEX:
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Index.Left);
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Index.Right);
	    Size = Ebcscript_Type_getSize(E->As.Index.Left->TypeTree);
	    Ebcscript_Parser_pop_long(Prs, &L2);
	    Ebcscript_Parser_pop_address(Prs, &P1);
	    Ebcscript_Parser_push_address(Prs, P1 + L2 * Size);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DOT:
	    Ebcscript_Parser_eval_expr_lv(Prs, E->As.Dot.Child);
	    Ebcscript_Parser_pop_address(Prs, &P1);
	    I2 = E->As.Dot.Offset;
	    Ebcscript_Parser_push_address(Prs, P1 + I2);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ARROW:	/* -> */
	    Ebcscript_Parser_eval_expr_rv(Prs, E->As.Arrow.Child);
								/*右辺値*/
	    Ebcscript_Parser_pop_address(Prs, &P1);
	    I2 = E->As.Arrow.Offset;
	    Ebcscript_Parser_push_address(Prs, P1 + I2);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_POST:	/* 後置++ */
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_POST:	/* 後置-- */
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADDR:	/* & */
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_PTR:	/* * */
	    log_error0("initializer element is not constant\n");
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UPLUS:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UMINUS:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NOT:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_NOT:	/* ! */
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_PRE:	/* 前置++ */
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_PRE:	/* 前置-- */
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_EXPR:
							/* sizeof expr */
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_TYPE:
							/* sizeof(type) */
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CAST:		/* 型変換 */
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MUL:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DIV:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MOD:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADD:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SUB:
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_L:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_R:
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LT:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GT:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LE:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GE:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_EQ:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NE:
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_AND:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_XOR:
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_OR:
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_AND:	/* && */
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_OR:	/* || */
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COND:	/* ? */
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN:		/* = */
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COMMA:	/* , */
	    log_debug0("an operation which have no lvalue\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  default:
	    log_debug0("an invalid kind of expression\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;
	}
}

void Ebcscript_Parser_Expression_dump(ebcscript_parser_expression *E)
{
	switch (E->Kind) {
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ID:	/* 識別子 */
	    fprintf(Ebcscript_Parser_Expression_Fplog, " ID");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CONST:	/* 定数 */
	    fprintf(Ebcscript_Parser_Expression_Fplog, " CONST");
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_FUNC:	/* () */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(FUNC");
	    Ebcscript_Parser_Expression_dump(E->As.Func.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INDEX:	/* [] */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(INDEX");
	    Ebcscript_Parser_Expression_dump(E->As.Index.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Index.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DOT:	/* . */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(DOT");
	    Ebcscript_Parser_Expression_dump(E->As.Dot.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ARROW:	/* -> */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(ARROW");
	    Ebcscript_Parser_Expression_dump(E->As.Arrow.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_POST:	/* 後置++ */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(INC_POST");
	    Ebcscript_Parser_Expression_dump(E->As.IncPost.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_POST:	/* 後置-- */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(DEC_POST");
	    Ebcscript_Parser_Expression_dump(E->As.DecPost.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADDR:	/* & */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(ADDR");
	    Ebcscript_Parser_Expression_dump(E->As.Addr.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_PTR:	/* * */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(PTR");
	    Ebcscript_Parser_Expression_dump(E->As.Ptr.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UPLUS:	/* + */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(UPLUS");
	    Ebcscript_Parser_Expression_dump(E->As.UPlus.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UMINUS:	/* - */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(UMINUS");
	    Ebcscript_Parser_Expression_dump(E->As.UMinus.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NOT:	/* ~ */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(NOT");
	    Ebcscript_Parser_Expression_dump(E->As.Not.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_NOT:	/* ! */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(LOG_NOT");
	    Ebcscript_Parser_Expression_dump(E->As.LogNot.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_PRE:	/* 前置++ */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(INC_PRE");
	    Ebcscript_Parser_Expression_dump(E->As.IncPre.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_PRE:	/* 前置-- */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(DEC_PRE");
	    Ebcscript_Parser_Expression_dump(E->As.DecPre.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_EXPR:/* sizeof expr */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(SIZEOF_EXPR");
	    Ebcscript_Parser_Expression_dump(E->As.SizeofExpr.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_TYPE:/* sizeof(type) */
	    fprintf(Ebcscript_Parser_Expression_Fplog, " SIZEOF_TYPE");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CAST:	/* 型変換 */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(CAST");
	    Ebcscript_Parser_Expression_dump(E->As.Cast.Child);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MUL:	/* * */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(MUL");
	    Ebcscript_Parser_Expression_dump(E->As.Mul.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Mul.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DIV:	/* / */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(DIV");
	    Ebcscript_Parser_Expression_dump(E->As.Div.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Div.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MOD:	/* % */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(MOD");
	    Ebcscript_Parser_Expression_dump(E->As.Mod.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Mod.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADD:	/* + */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(ADD");
	    Ebcscript_Parser_Expression_dump(E->As.Add.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Add.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SUB:	/* - */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(SUB");
	    Ebcscript_Parser_Expression_dump(E->As.Sub.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Sub.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_L:	/* << */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(SHFT_L");
	    Ebcscript_Parser_Expression_dump(E->As.ShftL.Left);
	    Ebcscript_Parser_Expression_dump(E->As.ShftL.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_R:	/* >> */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(SHFT_R");
	    Ebcscript_Parser_Expression_dump(E->As.ShftR.Left);
	    Ebcscript_Parser_Expression_dump(E->As.ShftR.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LT:	/* < */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(LT");
	    Ebcscript_Parser_Expression_dump(E->As.Lt.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Lt.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GT:	/* > */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(GT");
	    Ebcscript_Parser_Expression_dump(E->As.Gt.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Gt.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LE:	/* <= */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(LE");
	    Ebcscript_Parser_Expression_dump(E->As.Le.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Le.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GE:	/* >= */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(GE");
	    Ebcscript_Parser_Expression_dump(E->As.Ge.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Ge.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_EQ:	/* == */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(EQ");
	    Ebcscript_Parser_Expression_dump(E->As.Eq.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Eq.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NE:	/* != */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(NE");
	    Ebcscript_Parser_Expression_dump(E->As.Ne.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Ne.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_AND:	/* & */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(AND");
	    Ebcscript_Parser_Expression_dump(E->As.And.Left);
	    Ebcscript_Parser_Expression_dump(E->As.And.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_XOR:	/* ^ */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(XOR");
	    Ebcscript_Parser_Expression_dump(E->As.Xor.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Xor.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_OR:	/* | */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(OR");
	    Ebcscript_Parser_Expression_dump(E->As.Or.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Or.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_AND:	/* && */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(LOG_AND");
	    Ebcscript_Parser_Expression_dump(E->As.LogAnd.Left);
	    Ebcscript_Parser_Expression_dump(E->As.LogAnd.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_OR:	/* || */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(LOG_OR");
	    Ebcscript_Parser_Expression_dump(E->As.LogOr.Left);
	    Ebcscript_Parser_Expression_dump(E->As.LogOr.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COND:	/* ? */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(COND");
	    Ebcscript_Parser_Expression_dump(E->As.Cond.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Cond.Center);
	    Ebcscript_Parser_Expression_dump(E->As.Cond.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN:		/* = */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(ASSIGN");
	    Ebcscript_Parser_Expression_dump(E->As.Assign.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Assign.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COMMA:	/* , */
	    fprintf(Ebcscript_Parser_Expression_Fplog, "(COMMA");
	    Ebcscript_Parser_Expression_dump(E->As.Comma.Left);
	    Ebcscript_Parser_Expression_dump(E->As.Comma.Right);
	    fprintf(Ebcscript_Parser_Expression_Fplog, ")");
	    break;

	  default:
	    break;
	}
}

ebcscript_parser_expression *Ebcscript_Parser_Expression_dup(
                                                ebcscript_parser_expression *E0)
{
	ebcscript_parser_expression *E, *E1;
	btree *P, **Q;

	E = Ebcscript_Parser_newExpression();
	switch (E0->Kind) {
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ID:	/* 識別子 */
	    *E = *E0;
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CONST:	/* 定数 */
	    *E = *E0;
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_FUNC:	/* () */
	    E->As.Func.Child =
	                     Ebcscript_Parser_Expression_dup(E0->As.Func.Child);
	    E->As.Func.Arguments = NULL;
	    Q = &E->As.Func.Arguments;
	    for (P = E0->As.Func.Arguments; P != NULL; P = P->Right) {
	      E1 = Ebcscript_Parser_Expression_dup(P->Datum);
	      *Q = newBTree(E1, NULL, NULL);
	      Q = &((*Q)->Right);
	    }
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INDEX:	/* [] */
	    E->As.Index.Left =
	                     Ebcscript_Parser_Expression_dup(E0->As.Index.Left);
	    E->As.Index.Right =
	                    Ebcscript_Parser_Expression_dup(E0->As.Index.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DOT:	/* . */
	    E->As.Dot.Child =
	                      Ebcscript_Parser_Expression_dup(E0->As.Dot.Child);
	    E->As.Dot.Offset = E0->As.Dot.Offset;
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ARROW:	/* -> */
	    E->As.Arrow.Child =
	                    Ebcscript_Parser_Expression_dup(E0->As.Arrow.Child);
	    E->As.Arrow.Offset = E0->As.Arrow.Offset;
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_POST:	/* 後置++ */
	    E->As.IncPost.Child =
	                  Ebcscript_Parser_Expression_dup(E0->As.IncPost.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_POST:	/* 後置-- */
	    E->As.DecPost.Child =
	                  Ebcscript_Parser_Expression_dup(E0->As.DecPost.Child);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADDR:	/* & */
	    E->As.Addr.Child =
	                     Ebcscript_Parser_Expression_dup(E0->As.Addr.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_PTR:	/* * */
	    E->As.Ptr.Child =
	                      Ebcscript_Parser_Expression_dup(E0->As.Ptr.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UPLUS:	/* + */
	    E->As.UPlus.Child =
	                    Ebcscript_Parser_Expression_dup(E0->As.UPlus.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UMINUS:	/* - */
	    E->As.UMinus.Child =
	                   Ebcscript_Parser_Expression_dup(E0->As.UMinus.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NOT:	/* ~ */
	    E->As.Not.Child = Ebcscript_Parser_Expression_dup(E0->As.Not.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_NOT:	/* ! */
	    E->As.LogNot.Child =
	                   Ebcscript_Parser_Expression_dup(E0->As.LogNot.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_PRE:	/* 前置++ */
	    E->As.IncPre.Child =
	                   Ebcscript_Parser_Expression_dup(E0->As.IncPre.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_PRE:	/* 前置-- */
	    E->As.DecPre.Child =
	                   Ebcscript_Parser_Expression_dup(E0->As.DecPre.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_EXPR:/* sizeof expr */
	    E->As.SizeofExpr.Child =
	               Ebcscript_Parser_Expression_dup(E0->As.SizeofExpr.Child);
	    E->As.SizeofExpr.Size = E0->As.SizeofExpr.Size;
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_TYPE:/* sizeof(type) */
	    E->As.SizeofType.TypeTree = E0->As.SizeofType.TypeTree;
	    E->As.SizeofType.Size     = E0->As.SizeofType.Size;
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CAST:	/* 型変換 */
	    E->As.Cast.Child =
	                     Ebcscript_Parser_Expression_dup(E0->As.Cast.Child);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MUL:	/* * */
	    E->As.Mul.Left  = Ebcscript_Parser_Expression_dup(E0->As.Mul.Left);
	    E->As.Mul.Right = Ebcscript_Parser_Expression_dup(E0->As.Mul.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DIV:	/* / */
	    E->As.Div.Left  = Ebcscript_Parser_Expression_dup(E0->As.Div.Left);
	    E->As.Div.Right = Ebcscript_Parser_Expression_dup(E0->As.Div.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MOD:	/* % */
	    E->As.Mod.Left  = Ebcscript_Parser_Expression_dup(E0->As.Mod.Left);
	    E->As.Mod.Right = Ebcscript_Parser_Expression_dup(E0->As.Mod.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADD:	/* + */
	    E->As.Add.Left  = Ebcscript_Parser_Expression_dup(E0->As.Add.Left);
	    E->As.Add.Right = Ebcscript_Parser_Expression_dup(E0->As.Add.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SUB:	/* - */
	    E->As.Sub.Left  = Ebcscript_Parser_Expression_dup(E0->As.Sub.Left);
	    E->As.Sub.Right = Ebcscript_Parser_Expression_dup(E0->As.Sub.Right);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_L:	/* << */
	    E->As.ShftL.Left =
	                     Ebcscript_Parser_Expression_dup(E0->As.ShftL.Left);
	    E->As.ShftL.Right =
	                    Ebcscript_Parser_Expression_dup(E0->As.ShftL.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_R:	/* >> */
	    E->As.ShftR.Left =
	                     Ebcscript_Parser_Expression_dup(E0->As.ShftR.Left);
	    E->As.ShftR.Right =
	                    Ebcscript_Parser_Expression_dup(E0->As.ShftR.Right);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LT:	/* < */
	    E->As.Lt.Left =  Ebcscript_Parser_Expression_dup(E0->As.Lt.Left);
	    E->As.Lt.Right = Ebcscript_Parser_Expression_dup(E0->As.Lt.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GT:	/* > */
	    E->As.Gt.Left =  Ebcscript_Parser_Expression_dup(E0->As.Gt.Left);
	    E->As.Gt.Right = Ebcscript_Parser_Expression_dup(E0->As.Gt.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LE:	/* <= */
	    E->As.Le.Left =  Ebcscript_Parser_Expression_dup(E0->As.Le.Left);
	    E->As.Le.Right = Ebcscript_Parser_Expression_dup(E0->As.Le.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GE:	/* >= */
	    E->As.Ge.Left =  Ebcscript_Parser_Expression_dup(E0->As.Ge.Left);
	    E->As.Ge.Right = Ebcscript_Parser_Expression_dup(E0->As.Ge.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_EQ:	/* == */
	    E->As.Eq.Left =  Ebcscript_Parser_Expression_dup(E0->As.Eq.Left);
	    E->As.Eq.Right = Ebcscript_Parser_Expression_dup(E0->As.Eq.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NE:	/* != */
	    E->As.Ne.Left  = Ebcscript_Parser_Expression_dup(E0->As.Ne.Left);
	    E->As.Ne.Right = Ebcscript_Parser_Expression_dup(E0->As.Ne.Right);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_AND:	/* & */
	    E->As.And.Left =  Ebcscript_Parser_Expression_dup(E0->As.And.Left);
	    E->As.And.Right = Ebcscript_Parser_Expression_dup(E0->As.And.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_XOR:	/* ^ */
	    E->As.Xor.Left  = Ebcscript_Parser_Expression_dup(E0->As.Xor.Left);
	    E->As.Xor.Right = Ebcscript_Parser_Expression_dup(E0->As.Xor.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_OR:	/* | */
	    E->As.Or.Left  = Ebcscript_Parser_Expression_dup(E0->As.Or.Left);
	    E->As.Or.Right = Ebcscript_Parser_Expression_dup(E0->As.Or.Right);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_AND:	/* && */
	    E->As.LogAnd.Left =
	                    Ebcscript_Parser_Expression_dup(E0->As.LogAnd.Left);
	    E->As.LogAnd.Right =
	                   Ebcscript_Parser_Expression_dup(E0->As.LogAnd.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_OR:	/* || */
	    E->As.LogOr.Left =
	                     Ebcscript_Parser_Expression_dup(E0->As.LogOr.Left);
	    E->As.LogOr.Right =
	                    Ebcscript_Parser_Expression_dup(E0->As.LogOr.Right);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COND:	/* ? */
	    E->As.Cond.Left =
	                      Ebcscript_Parser_Expression_dup(E0->As.Cond.Left);
	    E->As.Cond.Center =
	                    Ebcscript_Parser_Expression_dup(E0->As.Cond.Center);
	    E->As.Cond.Right =
	                     Ebcscript_Parser_Expression_dup(E0->As.Cond.Right);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN:		/* = */
	    E->As.Assign.Left =
	                    Ebcscript_Parser_Expression_dup(E0->As.Assign.Left);
	    E->As.Assign.Right =
	                   Ebcscript_Parser_Expression_dup(E0->As.Assign.Right);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COMMA:	/* , */
	    E->As.Comma.Left =
	                     Ebcscript_Parser_Expression_dup(E0->As.Comma.Left);
	    E->As.Comma.Right =
	                    Ebcscript_Parser_Expression_dup(E0->As.Comma.Right);
	    break;

	  default:
	    break;
	}
	return E;
}

ebcscript_parser_expression *Ebcscript_Parser_newExpression(void)
{
	ebcscript_parser_expression *E;

	E = Ebcscript_Parser_Expression_malloc(
	                                 sizeof(ebcscript_parser_expression));
	E->Kind = 0;
	E->TypeTree = NULL;
	E->IsConstant = false;

	return E;
}

void Ebcscript_Parser_deleteExpression(ebcscript_parser_expression *E)
{
	btree *P;

	if (E == NULL)
	  return;

	switch (E->Kind) {
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ID:
	    /* Nameは消さない */
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CONST:
	    /* 文字列リテラル以外は消してもいい。
	       文字列リテラルは[lex.l]でtrnsunitの定数表に登録している。 */
	    switch (E->As.Constant.Literal->Kind) {
	      case EBCSCRIPT_LITERAL_KIND_INTEGER:
	      case EBCSCRIPT_LITERAL_KIND_FLOATING:
	      case EBCSCRIPT_LITERAL_KIND_CHARACTER:
	        Ebcscript_deleteLiteral(E->As.Constant.Literal);
	        break;
	      case EBCSCRIPT_LITERAL_KIND_STRING:
	        break;
	      defualt:
	        break;
	    }
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_FUNC:
	    Ebcscript_Parser_deleteExpression(E->As.Func.Child);
/*	    for (P = E->As.Func.Arguments; P != NULL; P = P->Right) {
	      Ebcscript_Parser_deleteExpression(
	                               (ebcscript_parser_expression *)P->Datum);
	    }
	    BTree_clear(E->As.Func.Arguments, &BTree_clearDummy);
*/
	    BTree_clear(E->As.Func.Arguments,
	                   (void (*)(void *))Ebcscript_Parser_deleteExpression);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INDEX:
	    Ebcscript_Parser_deleteExpression(E->As.Index.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Index.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DOT:
	    Ebcscript_Parser_deleteExpression(E->As.Dot.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ARROW:
	    Ebcscript_Parser_deleteExpression(E->As.Arrow.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_POST:
	    Ebcscript_Parser_deleteExpression(E->As.IncPost.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_POST:
	    Ebcscript_Parser_deleteExpression(E->As.DecPost.Child);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADDR:
	    Ebcscript_Parser_deleteExpression(E->As.Addr.Child);
	    /* [parse.y]でnewType_pointer()している */
	    Ebcscript_deleteType(E->TypeTree);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_PTR:
	    Ebcscript_Parser_deleteExpression(E->As.Ptr.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UPLUS:
	    Ebcscript_Parser_deleteExpression(E->As.UPlus.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_UMINUS:
	    Ebcscript_Parser_deleteExpression(E->As.UMinus.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NOT:
	    Ebcscript_Parser_deleteExpression(E->As.Not.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_NOT:
	    Ebcscript_Parser_deleteExpression(E->As.LogNot.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_PRE:
	    Ebcscript_Parser_deleteExpression(E->As.IncPre.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_PRE:
	    Ebcscript_Parser_deleteExpression(E->As.DecPre.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_EXPR:
	    Ebcscript_Parser_deleteExpression(E->As.SizeofExpr.Child);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_TYPE:
	    /* ここで消してよい */
	    Ebcscript_deleteType(E->As.SizeofType.TypeTree);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_CAST:
	    Ebcscript_Parser_deleteExpression(E->As.Cast.Child);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MUL:
	    Ebcscript_Parser_deleteExpression(E->As.Mul.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Mul.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_DIV:
	    Ebcscript_Parser_deleteExpression(E->As.Div.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Div.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_MOD:
	    Ebcscript_Parser_deleteExpression(E->As.Mod.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Mod.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ADD:
	    Ebcscript_Parser_deleteExpression(E->As.Add.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Add.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SUB:
	    Ebcscript_Parser_deleteExpression(E->As.Sub.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Sub.Right);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_L:
	    Ebcscript_Parser_deleteExpression(E->As.ShftL.Left);
	    Ebcscript_Parser_deleteExpression(E->As.ShftL.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_R:
	    Ebcscript_Parser_deleteExpression(E->As.ShftR.Left);
	    Ebcscript_Parser_deleteExpression(E->As.ShftR.Right);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LT:
	    Ebcscript_Parser_deleteExpression(E->As.Lt.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Lt.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GT:
	    Ebcscript_Parser_deleteExpression(E->As.Gt.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Gt.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LE:
	    Ebcscript_Parser_deleteExpression(E->As.Le.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Le.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_GE:
	    Ebcscript_Parser_deleteExpression(E->As.Ge.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Ge.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_EQ:
	    Ebcscript_Parser_deleteExpression(E->As.Eq.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Eq.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_NE:
	    Ebcscript_Parser_deleteExpression(E->As.Ne.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Ne.Right);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_AND:
	    Ebcscript_Parser_deleteExpression(E->As.And.Left);
	    Ebcscript_Parser_deleteExpression(E->As.And.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_XOR:
	    Ebcscript_Parser_deleteExpression(E->As.Xor.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Xor.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_OR:
	    Ebcscript_Parser_deleteExpression(E->As.Or.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Or.Right);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_AND:
	    Ebcscript_Parser_deleteExpression(E->As.LogAnd.Left);
	    Ebcscript_Parser_deleteExpression(E->As.LogAnd.Right);
	    break;
	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_OR:
	    Ebcscript_Parser_deleteExpression(E->As.LogOr.Left);
	    Ebcscript_Parser_deleteExpression(E->As.LogOr.Right);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COND:
	    Ebcscript_Parser_deleteExpression(E->As.Cond.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Cond.Center);
	    Ebcscript_Parser_deleteExpression(E->As.Cond.Right);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN:
	    Ebcscript_Parser_deleteExpression(E->As.Assign.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Assign.Right);
	    break;

	  case EBCSCRIPT_PARSER_EXPRESSION_KIND_COMMA:
	    Ebcscript_Parser_deleteExpression(E->As.Comma.Left);
	    Ebcscript_Parser_deleteExpression(E->As.Comma.Right);
	    break;

	  default:
	    break;
	}
	Ebcscript_Parser_Expression_free(E);
}

ebcscript_parser_expression *Ebcscript_Parser_newExpression_cast(
                            ebcscript_type *T, ebcscript_parser_expression *C)
{
	ebcscript_parser_expression *E;

/*	if (Ebcscript_Type_equals(T, C->TypeTree)) {
	  return C;
	}
*/
	E = Ebcscript_Parser_newExpression();
	E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_CAST;
	E->TypeTree = T;
	E->IsConstant = C->IsConstant;
	E->As.Cast.Child = C;
	return E;
}

/* -------------------------------------------------------------------------- */
#ifdef DEBUG
int main(int argc, char *argv[])
{
	return 0;
}
#endif
