/*******************************************************************************
   Project : C script
   File    : decl.c
   Date    : 2018.6.4-
   Note    : 宣言の解析のための補助処理
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include "parser.h"
#include "decl.h"
#include "name.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Parser_Declaration_malloc(size_t Size);
void _Ebcscript_Parser_Declaration_free(void *P);
void _Ebcscript_Parser_Declaration_exit(int Status);
void _Ebcscript_Parser_Declaration_log(const char *Fmt, ...);

FILE *Ebcscript_Parser_Declaration_Fplog/* = stdout*/;
size_t Ebcscript_Parser_Declaration_MallocTotal = 0;
void *(*Ebcscript_Parser_Declaration_malloc)(size_t Size) =
                                          &_Ebcscript_Parser_Declaration_malloc;
void (*Ebcscript_Parser_Declaration_free)(void *) =
                                            &_Ebcscript_Parser_Declaration_free;
void (*Ebcscript_Parser_Declaration_exit)(int) =
                                            &_Ebcscript_Parser_Declaration_exit;
void (*Ebcscript_Parser_Declaration_log)(const char *Fmt, ...) =
                                             &_Ebcscript_Parser_Declaration_log;

/*                                                                   関数定義 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Parser_Declaration_malloc(size_t Size)
{
	void *P;

	if ((P = malloc(Size + sizeof(size_t))) == NULL){
	  Ebcscript_Parser_Declaration_log(
	   "Ebcscript_Parser_Declaration_malloc():"
	   " error: memory allocation error(total=%d bytes)\n",
	   Ebcscript_Parser_Declaration_MallocTotal);
	  Ebcscript_Parser_Declaration_exit(1);
	}
	*(size_t *)P = Size;
	Ebcscript_Parser_Declaration_MallocTotal += Size;
	return (size_t *)P + 1;
}

void _Ebcscript_Parser_Declaration_free(void *P)
{
	P = (size_t *)P - 1;
	Ebcscript_Parser_Declaration_MallocTotal -= *(size_t *)P;
	free(P);
}

void Ebcscript_Parser_Declaration_onexit(void)
{
	fclose(Ebcscript_Parser_Declaration_Fplog);
}

void _Ebcscript_Parser_Declaration_exit(int Status)
{
	Ebcscript_Parser_Declaration_onexit();
	exit(Status);
}

void _Ebcscript_Parser_Declaration_log(const char *Fmt, ...)
{
	va_list Args;

	fprintf(Ebcscript_Parser_Declaration_Fplog,
	                                      "ebcscript_parser_declaration: ");

	va_start(Args, Fmt);
	vfprintf(Ebcscript_Parser_Declaration_Fplog, Fmt, Args);
	va_end(Args);
}
/* -------------------------------------------------------------------------- */
#define log_debug0(Message)						\
{									\
	Ebcscript_Parser_Declaration_log(				\
	 "%s:%d: debug: %s:%d: " Message,				\
	 Prs->Filename, Prs->Line, __FILE__, __LINE__);			\
}

#define log_debug1(Message, Param1)					\
{									\
	Ebcscript_Parser_Declaration_log(				\
	 "%s:%d: debug: %s:%d: " Message,				\
	 Prs->Filename, Prs->Line, __FILE__, __LINE__,			\
	 Param1);							\
}

#define log_error0(Message)						\
{									\
	Ebcscript_Parser_Declaration_log(				\
	 "%s:%d: error: " Message,					\
	 Prs->Filename, Prs->Line);					\
}

#define log_error1(Message, Param1)					\
{									\
	Ebcscript_Parser_Declaration_log(				\
	 "%s:%d: error: " Message,					\
	 Prs->Filename, Prs->Line,					\
	 Param1);							\
}

ebcscript_parser_declaration *Ebcscript_Parser_newDeclaration(void)
{
	ebcscript_parser_declaration *P;

	P = Ebcscript_Parser_Declaration_malloc(
	                                sizeof(ebcscript_parser_declaration));
	P->StorageClass     = 0;
	P->PrimitiveType    = 0;
	P->ShortOrLong      = 0;
	P->SignedOrUnsigned = 0;
	P->IsConst    = false;
	P->IsVolatile = false;
	P->Type = NULL;

	return P;
}

void Ebcscript_Parser_deleteDeclaration(ebcscript_parser_declaration *P)
{
	Ebcscript_Parser_Declaration_free(P);
}

/* 宣言指定子についての情報D1をD2に統合し、D2を返す */
ebcscript_parser_declaration *Ebcscript_Parser_mergeDeclaration(
 ebcscript_parser *Prs,
 ebcscript_parser_declaration *D1,
 ebcscript_parser_declaration *D2
)
{
#define INT  EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_INT
#define CHAR EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_CHAR
	  /* 排他的な組合せ */
	  if (D1->SignedOrUnsigned != 0 && D2->SignedOrUnsigned != 0) {
	    log_error0("two or more data types in declaration specifiers\n")
	    longjmp(Prs->CheckPoint, 1);
	  }

	  if (D1->ShortOrLong != 0 && D2->ShortOrLong != 0) {
	    log_error0("two or more data types in declaration specifiers\n")
	    longjmp(Prs->CheckPoint, 1);
	  }

	  if ( (D1->PrimitiveType != 0 || D1->Type != NULL)
	    && (D2->PrimitiveType != 0 || D2->Type != NULL) ) {
	    log_error0("two or more data types in declaration specifiers\n")
	    longjmp(Prs->CheckPoint, 1);
	  }

	  /* D1をD2へ統合 */ 
	  D2->SignedOrUnsigned |= D1->SignedOrUnsigned;
	  D2->ShortOrLong      |= D1->ShortOrLong;
	  D2->PrimitiveType    |= D1->PrimitiveType;
	  D2->Type = (D1->Type != NULL) ? D1->Type : D2->Type;

	  /* int以外（struct等含む）にはshort, longは付かない。 */
	  if (!(D2->PrimitiveType == INT) && D2->ShortOrLong != 0) {
	    log_error0("two or more data types in declaration specifiers\n")
	    longjmp(Prs->CheckPoint, 1);
	  }

	  /* char, int以外（struct等含む）にはsigned, unsignedは付かない。 */
	  if ( !(D2->PrimitiveType == CHAR || D2->PrimitiveType == INT)
	                                        && D2->SignedOrUnsigned != 0 ) {
	    log_error0("two or more data types in declaration specifiers\n")
	    longjmp(Prs->CheckPoint, 1);
	  }

	  return D2;
#undef INT
#undef CHAR
}

/* 宣言指定子についての情報Dから型表現木を作る */
ebcscript_type *Ebcscript_Parser_Declaration_toTypeTree(
                                             ebcscript_parser_declaration *D)
{
#define VOID     EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_VOID
#define CHAR     EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_CHAR
#define INT      EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_INT
#define FLOAT    EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_FLOAT
#define DOUBLE   EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_DOUBLE
#define SHORT    EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_SHORT
#define LONG     EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_LONG
#define UNSIGNED EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_UNSIGNED

	ebcscript_type *T;

	switch (D->PrimitiveType) {
	  case 0:
	    if (D->Type != NULL)
	      T = D->Type;
	    /* 4通り */
	    /* short or long */
	    /* signed or unsigned */
	    if (D->ShortOrLong == SHORT && D->SignedOrUnsigned != UNSIGNED)
	      T = Ebcscript_Type_Short();
	    if (D->ShortOrLong == SHORT && D->SignedOrUnsigned == UNSIGNED)
	      T = Ebcscript_Type_UShort();
	    if (D->ShortOrLong == LONG  && D->SignedOrUnsigned != UNSIGNED)
	      T = Ebcscript_Type_Long();
	    if (D->ShortOrLong == LONG  && D->SignedOrUnsigned == UNSIGNED)
	      T = Ebcscript_Type_ULong();
	    break;
	  case VOID:
	      T = Ebcscript_Type_Void();
	    break;
	  case CHAR:
	    /* 2通り */
	    /* signed or unsigned */
	    if (D->SignedOrUnsigned != UNSIGNED)
	      T = Ebcscript_Type_Char();
	    if (D->SignedOrUnsigned == UNSIGNED)
	      T = Ebcscript_Type_UChar();
	    break;
	  case INT:
	    /* 6通り */
	    /* short or long or int */
	    /* signed or unsigned */
	    if (D->ShortOrLong == SHORT && D->SignedOrUnsigned != UNSIGNED)
	      T = Ebcscript_Type_Short();
	    if (D->ShortOrLong == SHORT && D->SignedOrUnsigned == UNSIGNED)
	      T = Ebcscript_Type_UShort();
	    if (D->ShortOrLong == LONG  && D->SignedOrUnsigned != UNSIGNED)
	      T = Ebcscript_Type_Long();
	    if (D->ShortOrLong == LONG  && D->SignedOrUnsigned == UNSIGNED)
	      T = Ebcscript_Type_ULong();
	    if (D->ShortOrLong == 0     && D->SignedOrUnsigned != UNSIGNED)
	      T = Ebcscript_Type_Int();
	    if (D->ShortOrLong == 0     && D->SignedOrUnsigned == UNSIGNED)
	      T = Ebcscript_Type_UInt();
	    break;
	  case FLOAT:
	    T = Ebcscript_Type_Float();
	    break;
	  case DOUBLE:
	    T = Ebcscript_Type_Double();
	    break;
	  default:
	    T = NULL;
	    break;
	}
	return T;
#undef VOID
#undef CHAR
#undef INT
#undef FLOAT
#undef DOUBLE
#undef SHORT
#undef LONG
#undef UNSIGNED
}

void Ebcscript_Parser_checkRepetation_varfunc(
 ebcscript_parser *Prs,
 ebcscript_name *N0,
 ebcscript_name *N,
 int Nest
)
{
	int N0_Linkage, N_Linkage;
	void *N0_Address, *N_Address;
	void *N0_CodeAddress, *N_CodeAddress;

	/* 名前の種類 */
	if (N0->Kind != N->Kind) {
	  log_error1(
	   "\'%s\' redeclared as different kind of symbol\n", N->Identifier)
	  longjmp(Prs->CheckPoint, 1);
	}

	/* 型 */
	switch(N->Kind) {
	  case EBCSCRIPT_NAME_KIND_VARIABLE:
	    if (!Ebcscript_Type_equals(N->As.Variable.TypeTree,
	                                            N0->As.Variable.TypeTree)) {
	      log_error1("conflicting types for \'%s\'\n", N->Identifier)
	      longjmp(Prs->CheckPoint, 1);
	    }
	    break;
	  case EBCSCRIPT_NAME_KIND_FUNCTION:
	    if (!Ebcscript_Type_equals(N->As.Function.TypeTree,
	                                            N0->As.Function.TypeTree)) {
	      log_error1("conflicting types for \'%s\'\n", N->Identifier)
	      longjmp(Prs->CheckPoint, 1);
	    }
	    break;
	  case EBCSCRIPT_NAME_KIND_TYPEDEF:
	    if (!Ebcscript_Type_equals(N->As.Typedef.TypeTree,
	                                             N0->As.Typedef.TypeTree)) {
	      log_error1("conflicting types for \'%s\'\n", N->Identifier)
	      longjmp(Prs->CheckPoint, 1);
	    }
	    break;
	  case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	    break;
	  default:
	    log_debug0("an invalid kind of name\n")
	    longjmp(Prs->CheckPoint, 1);
	    break;
	}

	/* リンケージ、既定義 */
	switch(N->Kind) {
	  case EBCSCRIPT_NAME_KIND_VARIABLE:
	    N0_Linkage = N0->As.Variable.Linkage;
	    N0_Address = N0->As.Variable.Address;
	    N_Linkage  = N ->As.Variable.Linkage;
	    N_Address  = N ->As.Variable.Address;

	    if (Nest == 0) {

	      /* (1) none, none */
	      if (N0_Linkage == EBCSCRIPT_NAME_LINKAGE_EXTERNAL
	       && N0_Address != NULL
	       && N_Linkage  == EBCSCRIPT_NAME_LINKAGE_EXTERNAL
	       && N_Address  != NULL) {
	        log_error1("redefinition of \'%s\'\n", N->Identifier)
	        longjmp(Prs->CheckPoint, 1);
	      }

	      /* (2) static, none */
	      if (N0_Linkage == EBCSCRIPT_NAME_LINKAGE_INTERNAL
	       && N_Linkage  == EBCSCRIPT_NAME_LINKAGE_EXTERNAL
	       && N_Address  != NULL) {
	        log_error1(
	         "non-static declaration of \'%s\' "
	                                         "follows static declaration\n",
	         N->Identifier)
	        longjmp(Prs->CheckPoint, 1);
	      }

	      /* (3) extern, none */
	      if (N0_Linkage == EBCSCRIPT_NAME_LINKAGE_EXTERNAL
	       && N0_Address == NULL
	       && N_Linkage  == EBCSCRIPT_NAME_LINKAGE_EXTERNAL
	       && N_Address  != NULL) {
	        /* N0に統合 */
	        N0->As.Variable.Address    = N->As.Variable.Address;
	        N0->As.Variable.Addressing = N->As.Variable.Addressing;
	        Ebcscript_deleteName(N);
	      }

	      /* (4) none, static */
	      if (N0_Linkage == EBCSCRIPT_NAME_LINKAGE_EXTERNAL
	       && N0_Address != NULL
	       && N_Linkage  == EBCSCRIPT_NAME_LINKAGE_INTERNAL) {
	        log_error1(
	         "static declaration of \'%s\' "
	                                     "follows non-static declaration\n",
	         N->Identifier)
	        longjmp(Prs->CheckPoint, 1);
	      }

	      /* (5) static, static */
	      if (N0_Linkage == EBCSCRIPT_NAME_LINKAGE_INTERNAL
	       && N_Linkage  == EBCSCRIPT_NAME_LINKAGE_INTERNAL) {
	        /* N0に統合 */
/*	        free(N->As.Variable.Address); */
	        N0->As.Variable.Address    = N->As.Variable.Address;
	        N0->As.Variable.Addressing = N->As.Variable.Addressing;
	        Ebcscript_deleteName(N);
	      }

	      /* (6) extern, static */
	      if (N0_Linkage == EBCSCRIPT_NAME_LINKAGE_EXTERNAL
	       && N0_Address == NULL
	       && N_Linkage  == EBCSCRIPT_NAME_LINKAGE_INTERNAL) {
	        log_error1(
	         "static declaration of \'%s\' "
	                                     "follows non-static declaration\n",
	         N->Identifier)
	        longjmp(Prs->CheckPoint, 1);
	      }

	      /* (7) none, extern */
	      if (N0_Linkage == EBCSCRIPT_NAME_LINKAGE_EXTERNAL
	       && N0_Address != NULL
	       && N_Linkage  == EBCSCRIPT_NAME_LINKAGE_EXTERNAL
	       && N_Address  == NULL) {
	        Ebcscript_deleteName(N);
	      }

	      /* (8) static, extern */
	      if (N0_Linkage == EBCSCRIPT_NAME_LINKAGE_INTERNAL
	       && N_Linkage  == EBCSCRIPT_NAME_LINKAGE_EXTERNAL
	       && N_Address  == NULL) {
	        Ebcscript_deleteName(N);
	      }

	      /* (9) extern, extern */
	      if (N0_Linkage == EBCSCRIPT_NAME_LINKAGE_EXTERNAL
	       && N0_Address == NULL
	       && N_Linkage  == EBCSCRIPT_NAME_LINKAGE_EXTERNAL
	       && N_Address  == NULL) {
	        Ebcscript_deleteName(N);
	      }

	    }

	    if (Nest > 0) {

	      if (N0_Address == NULL && N_Address == NULL) {
	        Ebcscript_deleteName(N);
	      }

	      if (N0_Address == NULL && N_Address != NULL) {
	        log_error1(
	         "declaration of \'%s\' with no linkage "
	                                         "follows extern declaration\n",
	         N->Identifier)
	        longjmp(Prs->CheckPoint, 1);
	      }

	      if (N0_Address != NULL && N_Address == NULL) {
	        log_error1(
	         "extern declaration of \'%s\' follows "
	                                        "declaration with no linkage\n",
	         N->Identifier)
	        longjmp(Prs->CheckPoint, 1);
	      }

	      if (N0_Address != NULL && N_Address != NULL) {
	        log_error1(
	         "redeclaration of \'%s\' with no linkage\n", N->Identifier)
	        longjmp(Prs->CheckPoint, 1);
	      }

	    }
	    break;

	  case EBCSCRIPT_NAME_KIND_FUNCTION:
	    N0_Linkage     = N0->As.Function.Linkage;
	    N0_CodeAddress = N0->As.Function.CodeAddress;
	    N_Linkage      = N ->As.Function.Linkage;
	    N_CodeAddress  = N ->As.Function.CodeAddress;

	    if (N0_Linkage == EBCSCRIPT_NAME_LINKAGE_EXTERNAL
	     &&  N_Linkage == EBCSCRIPT_NAME_LINKAGE_INTERNAL) {
	      log_error1(
	       "static declaration of \'%s\' follows non-static declaration\n",
	       N->Identifier)
	      longjmp(Prs->CheckPoint, 1);
	    }

	    if (N0_CodeAddress == NULL && N_CodeAddress == NULL) {
	      Ebcscript_deleteName(N);
	    }

	    if (N0_CodeAddress != NULL && N_CodeAddress == NULL) {
	      Ebcscript_deleteName(N);
	    }

	    if (N0_CodeAddress == NULL && N_CodeAddress != NULL) {
	      /* Linkageは前のまま */
	      N0->As.Function.CodeAddress = N->As.Function.CodeAddress;
	      N0->As.Function.Addressing  = N->As.Function.Addressing;
	      Ebcscript_deleteName(N);
	    }

	    if (N0_CodeAddress != NULL && N_CodeAddress != NULL) {
	      log_error1("redefinition of \'%s\'\n", N->Identifier)
	      longjmp(Prs->CheckPoint, 1);
	    }
	    break;

	  case EBCSCRIPT_NAME_KIND_TYPEDEF:
	    /* チェック項目なし */
	    Ebcscript_deleteName(N);
	    break;

	  case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	    log_error1("redeclaration of enumrator \'%s\'\n", N->Identifier)
	    longjmp(Prs->CheckPoint, 1);
	    break;

	  default:
	    log_debug0("an invalid kind of name\n");
	    longjmp(Prs->CheckPoint, 1);
	    break;
	}
}

void Ebcscript_Parser_checkRepetation_tag(
 ebcscript_parser *Prs,
 ebcscript_name *N0,
 ebcscript_name *N
)
{
	ebcscript_type *T0, *T;

	/* 名前の種類 */
	if (N0->Kind != N->Kind) {
	  log_error1("\'%s\' defined as wrong kind of tag\n", N->Identifier)
	  Ebcscript_deleteName(N);
	  longjmp(Prs->CheckPoint, 1);
	}

	/* 不完全型か？ */
	switch (N->Kind) {
	  case EBCSCRIPT_NAME_KIND_STRUCT:
	    T0 = N0->As.Struct.TypeTree;

	    if (!SList_isEmpty(T0->As.Struct.Members)) {
	      log_error1("redefinition of \'struct %s\'\n", N->Identifier)
	      Ebcscript_deleteName(N);
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* N0を生かす */
	    Ebcscript_deleteName(N);
	    break;

	  case EBCSCRIPT_NAME_KIND_UNION:
	    T0 = N0->As.Union.TypeTree;

	    if (!SList_isEmpty(T0->As.Union.Members)) {
	      log_error1("redefinition of \'union %s\'\n", N->Identifier)
	      Ebcscript_deleteName(N);
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* N0を生かす */
	    Ebcscript_deleteName(N);
	    break;

	  case EBCSCRIPT_NAME_KIND_ENUMERATION:
	    T0 = N0->As.Enumeration.TypeTree;

	    if (!T0->As.Enumeration.IsEmpty) {
	      log_error1("nested redefinition of \'enum %s\'\n", N->Identifier)
	      Ebcscript_deleteName(N);
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* N0を生かす */
	    Ebcscript_deleteName(N);
	    break;

	  default:
	    log_debug0("an invalid kind of name\n")
	    Ebcscript_deleteName(N);
	    longjmp(Prs->CheckPoint, 1);
	    break;
	}
}

void Ebcscript_Parser_checkRepetation_label(
 ebcscript_parser *Prs,
 ebcscript_name *N0,
 ebcscript_name *N
)
{
	if (N0->As.Label.Addressing != EBCSCRIPT_NAME_ADDRESSING_UNDEFINED) {
	  log_error1("duplicate label \'%s\'\n", N->Identifier)
	  Ebcscript_deleteName(N);
	  longjmp(Prs->CheckPoint, 1);
	}

	/* N0を生かす */
	Ebcscript_deleteName(N);
}
/* -------------------------------------------------------------------------- */
#ifdef DEBUG
int main(int argc, char *argv[])
{
	return 0;
}
#endif
