/*******************************************************************************
   Project : C script
   File    : init.c
   Date    : 2018.7.19-
   Note    : 初期化子のための補助処理
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>
#include "parser.h"
#include "trnsunit.h"
#include "name.h"
#include "code.h"
#include "init.h"
#include "expr.h"
#include "btree.h"
#include "slist.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Parser_Initializer_malloc(size_t Size);
void _Ebcscript_Parser_Initializer_free(void *P);
void _Ebcscript_Parser_Initializer_exit(int Status);
void _Ebcscript_Parser_Initializer_log(const char *Fmt, ...);

FILE *Ebcscript_Parser_Initializer_Fplog/* = stdout*/;
size_t Ebcscript_Parser_Initializer_MallocTotal = 0;
void *(*Ebcscript_Parser_Initializer_malloc)(size_t Size) =
                                          &_Ebcscript_Parser_Initializer_malloc;
void (*Ebcscript_Parser_Initializer_free)(void *) =
                                            &_Ebcscript_Parser_Initializer_free;
void (*Ebcscript_Parser_Initializer_exit)(int) =
                                            &_Ebcscript_Parser_Initializer_exit;
void (*Ebcscript_Parser_Initializer_log)(const char *Fmt, ...) =
                                             &_Ebcscript_Parser_Initializer_log;

/*                                                                   関数定義 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Parser_Initializer_malloc(size_t Size)
{
	void *P;

	if ((P = malloc(Size + sizeof(size_t))) == NULL){
	  Ebcscript_Parser_Initializer_log(
	   "Ebcscript_Parser_Initializer_malloc(): "
	   "error: memory allocation error(total=%d bytes)\n",
	   Ebcscript_Parser_Initializer_MallocTotal);
	  Ebcscript_Parser_Initializer_exit(1);
	}
	*(size_t *)P = Size;
	Ebcscript_Parser_Initializer_MallocTotal += Size;
	return (size_t *)P + 1;
}

void _Ebcscript_Parser_Initializer_free(void *P)
{
	P = (size_t *)P - 1;
	Ebcscript_Parser_Initializer_MallocTotal -= *(size_t *)P;
	free(P);
}

void Ebcscript_Parser_Initializer_onexit(void)
{
	fclose(Ebcscript_Parser_Initializer_Fplog);
}

void _Ebcscript_Parser_Initializer_exit(int Status)
{
	Ebcscript_Parser_Initializer_onexit();
	exit(Status);
}

void _Ebcscript_Parser_Initializer_log(const char *Fmt, ...)
{
	va_list Args;

	fprintf(Ebcscript_Parser_Initializer_Fplog,
	                                      "ebcscript_parser_initializer: ");

	va_start(Args, Fmt);
	vfprintf(Ebcscript_Parser_Initializer_Fplog, Fmt, Args);
	va_end(Args);
}
/* -------------------------------------------------------------------------- */
#define log_debug0(Message)						\
{									\
	Ebcscript_Parser_Initializer_log(				\
	 "%s:%d: debug: %s:%d: " Message,				\
	 Prs->Filename, Prs->Line, __FILE__, __LINE__);			\
}

#define log_error0(Message)						\
{									\
	Ebcscript_Parser_Initializer_log(				\
	 "%s:%d: error: " Message,					\
	 Prs->Filename, Prs->Line);					\
}

#define log_warning0(Message)						\
{									\
	Ebcscript_Parser_Initializer_log(				\
	 "%s:%d: warning: " Message,					\
	 Prs->Filename, Prs->Line);					\
}

void Ebcscript_Parser_walkInitializerAlongType(
 ebcscript_parser *Prs,
 btree *Initializer,
 ebcscript_type *Type
)
{
	ebcscript_parser_expression *E;
	btree *P;
	slist_cell *Q;
	int Length;
	size_t Size;
	ebcscript_name *NMember;

	if (Initializer == NULL)
	  return;

	switch (Type->Kind) {
	  case EBCSCRIPT_TYPE_KIND_VOID:
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
	    /* 単一の初期化子か？ */
	    if (Initializer->Left != NULL) {
	      log_warning0("excess elements in scalar initializer\n")
	    }

	    if (Initializer->Datum != NULL) {
	      /* Typeの型に合わせてキャストを挿入 */
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      if (!Ebcscript_Type_equals(Type, E->TypeTree)) {
	        E = Ebcscript_Parser_newExpression_cast(Type, E);
	        Initializer->Datum = E;
	      }
	    }
	    break;

	  case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	    break;

	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	    break;

	  case EBCSCRIPT_TYPE_KIND_POINTER:
	    break;

	  case EBCSCRIPT_TYPE_KIND_ARRAY:
	    /* char[]の特例 */
	    if (Type->As.Array.Value->Kind == EBCSCRIPT_TYPE_KIND_CHAR
	     && Initializer->Datum != NULL
	     && ((ebcscript_parser_expression *)Initializer->Datum)->Kind
	                             == EBCSCRIPT_PARSER_EXPRESSION_KIND_CONST
	     && ((ebcscript_parser_expression *)Initializer->Datum)->
	           As.Constant.Literal->Kind == EBCSCRIPT_LITERAL_KIND_STRING) {
	      Type->As.Array.Length = strlen(((ebcscript_parser_expression *)
	               Initializer->Datum)->As.Constant.Literal->As.String) + 1;
	      break;
	    }

	    if (!(Initializer->Datum == NULL
	       && Initializer->Left != NULL)) {
	      log_error0("invalid initializer\n")
	      longjmp(Prs->CheckPoint, 1);
	    }
	    Length = 0;
	    for (P = Initializer->Left; P != NULL; P = P->Right) {
	      Length++;
	      Ebcscript_Parser_walkInitializerAlongType(Prs, P,
	                                                Type->As.Array.Value);
	    }
	    if (Type->As.Array.Length == -1) {	/* -1: 未設定 */
	      Type->As.Array.Length = (Length > Type->As.Array.Length) ?
	                                       Length : Type->As.Array.Length;
	    }
	    if (Type->As.Array.Length >= 0) {		/* >=0: 固定サイズ */
	      if (Type->As.Array.Length < Length) {
	        log_warning0("excess elements in array initializer\n")
	      }
	      Type->As.Array.Length = (Length > Type->As.Array.Length) ?
	                                       Length : Type->As.Array.Length;
	    }
	    break;

	  case EBCSCRIPT_TYPE_KIND_STRUCT:
	    if (!(Initializer->Datum == NULL
	       && Initializer->Left != NULL)) {
	      log_error0("invalid initializer\n")
	      longjmp(Prs->CheckPoint, 1);
	    }
	    for (P = Initializer->Left,
	                               Q = Type->As.Struct.Members->Head.Next;
	         P != NULL && Q != NULL;
	         P = P->Right, Q = Q->Next) {
	      NMember = (ebcscript_name *)Q->Datum;
	      Ebcscript_Parser_walkInitializerAlongType(Prs, P,
	                                       NMember->As.Variable.TypeTree);
	    }
	    if (P != NULL && NMember == NULL) {
	      log_warning0("excess elements in struct initializer\n")
	    }
	    break;

	  case EBCSCRIPT_TYPE_KIND_UNION:
	    break;

	  case EBCSCRIPT_TYPE_KIND_FUNCTION:
	    break;

	  default:
	    log_debug0("an invalid kind of type\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;
	}
}

void Ebcscript_Parser_eval_initializer(
 ebcscript_parser *Prs,
 btree *Initializer,
 ebcscript_type *Type,
 void *Address
)
{
	ebcscript_parser_expression *E;
	btree *Q;
	slist_cell *R;
	int Length;
	size_t Size;
	ebcscript_name *NMember;
	char C;
	short S;
	int I;
	long L;
	unsigned char UC;
	unsigned short US;
	unsigned int UI;
	unsigned long UL;
	float F;
	double D;
	void *P;

	if (Initializer == NULL)
	  return;

	switch (Type->Kind) {
	  case EBCSCRIPT_TYPE_KIND_VOID:
	    break;

	  case EBCSCRIPT_TYPE_KIND_CHAR:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_eval_expr_rv(Prs, E);
	      Ebcscript_Parser_pop_char(Prs, &C);
	      *(char *)Address = C;
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_UCHAR:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_eval_expr_rv(Prs, E);
	      Ebcscript_Parser_pop_uchar(Prs, &UC);
	      *(unsigned char *)Address = UC;
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_SHORT:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_eval_expr_rv(Prs, E);
	      Ebcscript_Parser_pop_short(Prs, &S);
	      *(short *)Address = S;
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_USHORT:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_eval_expr_rv(Prs, E);
	      Ebcscript_Parser_pop_ushort(Prs, &US);
	      *(unsigned short *)Address = US;
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_INT:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_eval_expr_rv(Prs, E);
	      Ebcscript_Parser_pop_int(Prs, &I);
	      *(int *)Address = I;
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_UINT:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_eval_expr_rv(Prs, E);
	      Ebcscript_Parser_pop_uint(Prs, &UI);
	      *(unsigned int *)Address = UI;
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_LONG:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_eval_expr_rv(Prs, E);
	      Ebcscript_Parser_pop_long(Prs, &L);
	      *(long *)Address = L;
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_ULONG:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_eval_expr_rv(Prs, E);
	      Ebcscript_Parser_pop_ulong(Prs, &UL);
	      *(unsigned long *)Address = UL;
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_FLOAT:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_eval_expr_rv(Prs, E);
	      Ebcscript_Parser_pop_float(Prs, &F);
	      *(float *)Address = F;
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_eval_expr_rv(Prs, E);
	      Ebcscript_Parser_pop_double(Prs, &D);
	      *(double *)Address = D;
	    }
	    break;

	  case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	    break;

	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	    break;

	  case EBCSCRIPT_TYPE_KIND_POINTER:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_eval_expr_rv(Prs, E);
	      Ebcscript_Parser_pop_address(Prs, &P);
	      *(void **)Address = P;
	    }
	    break;

	  case EBCSCRIPT_TYPE_KIND_ARRAY:
	    /* char []の場合 */
	    if (Type->As.Array.Value->Kind == EBCSCRIPT_TYPE_KIND_CHAR
	     && Initializer->Datum != NULL
	     && ((ebcscript_parser_expression *)Initializer->Datum)->Kind
	                             == EBCSCRIPT_PARSER_EXPRESSION_KIND_CONST
	     && ((ebcscript_parser_expression *)Initializer->Datum)->
	           As.Constant.Literal->Kind == EBCSCRIPT_LITERAL_KIND_STRING) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      memcpy(
	       Address, 
	       E->As.Constant.Literal->As.String,
	       Type->As.Array.Length
	      );
	      break;
	    }

	    Size = Ebcscript_Type_getSize(Type->As.Array.Value);
	    for (Q = Initializer->Left; Q != NULL; Q = Q->Right) {
	      Ebcscript_Parser_eval_initializer(Prs, Q,
	                                       Type->As.Array.Value, Address);
	      Address += Size;
	    }
	    break;

	  case EBCSCRIPT_TYPE_KIND_STRUCT:
	    for (Q = Initializer->Left,
	                               R = Type->As.Struct.Members->Head.Next;
	         Q != NULL && R != NULL;
	         Q = Q->Right, R = R->Next) {
	      NMember = (ebcscript_name *)R->Datum;
	      Ebcscript_Parser_eval_initializer(Prs, Q,
	                              NMember->As.Variable.TypeTree, Address);
	      Size = Ebcscript_Type_getSize(NMember->As.Variable.TypeTree);
	      Address += Size;
	    }
	    break;

	  case EBCSCRIPT_TYPE_KIND_UNION:
	    break;

	  case EBCSCRIPT_TYPE_KIND_FUNCTION:
	    break;

	  default:
	    log_debug0("an invalid kind of type\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;
	}
}

void Ebcscript_Parser_gencode_initializer(
 ebcscript_parser *Prs,
 btree *Initializer,
 ebcscript_type *Type,
 void *Address
)
{
	ebcscript_parser_expression *E;
	btree *Q;
	slist_cell *R;
	int Length;
	size_t Size;
	ebcscript_name *NMember;

	if (Initializer == NULL)
	  return;

	switch (Type->Kind) {
	  case EBCSCRIPT_TYPE_KIND_VOID:
	    break;

	  case EBCSCRIPT_TYPE_KIND_CHAR:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_gencode_expr_rv(Prs, E);
	      Ebcscript_Trnsunit_store_instruction(Prs->TU,
	                                      EBCSCRIPT_INSTRUCTION_POP_LMEM_C);
	      Ebcscript_Trnsunit_store_address(Prs->TU, Address);
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_UCHAR:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_gencode_expr_rv(Prs, E);
	      Ebcscript_Trnsunit_store_instruction(Prs->TU,
	                                      EBCSCRIPT_INSTRUCTION_POP_LMEM_C);
	      Ebcscript_Trnsunit_store_address(Prs->TU, Address);
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_SHORT:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_gencode_expr_rv(Prs, E);
	      Ebcscript_Trnsunit_store_instruction(Prs->TU,
	                                      EBCSCRIPT_INSTRUCTION_POP_LMEM_S);
	      Ebcscript_Trnsunit_store_address(Prs->TU, Address);
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_USHORT:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_gencode_expr_rv(Prs, E);
	      Ebcscript_Trnsunit_store_instruction(Prs->TU,
	                                      EBCSCRIPT_INSTRUCTION_POP_LMEM_S);
	      Ebcscript_Trnsunit_store_address(Prs->TU, Address);
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_INT:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_gencode_expr_rv(Prs, E);
	      Ebcscript_Trnsunit_store_instruction(Prs->TU,
	                                      EBCSCRIPT_INSTRUCTION_POP_LMEM_I);
	      Ebcscript_Trnsunit_store_address(Prs->TU, Address);
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_UINT:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_gencode_expr_rv(Prs, E);
	      Ebcscript_Trnsunit_store_instruction(Prs->TU,
	                                      EBCSCRIPT_INSTRUCTION_POP_LMEM_I);
	      Ebcscript_Trnsunit_store_address(Prs->TU, Address);
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_LONG:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_gencode_expr_rv(Prs, E);
	      Ebcscript_Trnsunit_store_instruction(Prs->TU,
	                                      EBCSCRIPT_INSTRUCTION_POP_LMEM_L);
	      Ebcscript_Trnsunit_store_address(Prs->TU, Address);
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_ULONG:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_gencode_expr_rv(Prs, E);
	      Ebcscript_Trnsunit_store_instruction(Prs->TU,
	                                      EBCSCRIPT_INSTRUCTION_POP_LMEM_L);
	      Ebcscript_Trnsunit_store_address(Prs->TU, Address);
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_FLOAT:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_gencode_expr_rv(Prs, E);
	      Ebcscript_Trnsunit_store_instruction(Prs->TU,
	                                      EBCSCRIPT_INSTRUCTION_POP_LMEM_F);
	      Ebcscript_Trnsunit_store_address(Prs->TU, Address);
	    }
	    break;
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_gencode_expr_rv(Prs, E);
	      Ebcscript_Trnsunit_store_instruction(Prs->TU,
	                                      EBCSCRIPT_INSTRUCTION_POP_LMEM_D);
	      Ebcscript_Trnsunit_store_address(Prs->TU, Address);
	    }
	    break;

	  case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	    break;

	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	    break;

	  case EBCSCRIPT_TYPE_KIND_POINTER:
	    if (Initializer->Datum != NULL) {
	      E = (ebcscript_parser_expression *)Initializer->Datum;
	      Ebcscript_Parser_gencode_expr_rv(Prs, E);
	      Ebcscript_Trnsunit_store_instruction(Prs->TU,
	                                      EBCSCRIPT_INSTRUCTION_POP_LMEM_P);
	      Ebcscript_Trnsunit_store_address(Prs->TU, Address);
	    }
	    break;

	  case EBCSCRIPT_TYPE_KIND_ARRAY:
	    Size = Ebcscript_Type_getSize(Type->As.Array.Value);
	    for (Q = Initializer->Left; Q != NULL; Q = Q->Right) {
	      Ebcscript_Parser_gencode_initializer(Prs, Q,
	                                       Type->As.Array.Value, Address);
	      Address += Size;
	    }
	    break;

	  case EBCSCRIPT_TYPE_KIND_STRUCT:
	    for (Q = Initializer->Left,
	                               R = Type->As.Struct.Members->Head.Next;
	         Q != NULL && R != NULL;
	         Q = Q->Right, R = R->Next) {
	      NMember = (ebcscript_name *)R->Datum;
	      Ebcscript_Parser_gencode_initializer(Prs, Q,
	                              NMember->As.Variable.TypeTree, Address);
	      Size = Ebcscript_Type_getSize(NMember->As.Variable.TypeTree);
	      Address += Size;
	    }
	    break;

	  case EBCSCRIPT_TYPE_KIND_UNION:
	    break;

	  case EBCSCRIPT_TYPE_KIND_FUNCTION:
	    break;

	  default:
	    log_debug0("an invalid kind of type\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;
	}
}

void Ebcscript_Parser_deleteInitializer(btree *Initializer)
{
	if (Initializer == NULL) {
	  return;
	}
	Ebcscript_Parser_deleteInitializer(Initializer->Left);
	Ebcscript_Parser_deleteInitializer(Initializer->Right);

	if (Initializer->Datum != NULL) {
	  Ebcscript_Parser_deleteExpression(Initializer->Datum);
	}
	return;
}

/* -------------------------------------------------------------------------- */
#ifdef DEBUG
int main(int argc, char *argv[])
{
	return 0;
}
#endif
