%{
/*******************************************************************************
   Project : C script
   File    : parse.y
   Date    : 2018.3.24-
   Note    : 構文解析しながら、
             (1)記号表の作成、
             (2)意味チェック、
             (3)中間コードの生成・メモリの割り当て
             を行う。 
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "boolean.h"
#include "trnsunit.h"
#include "parser.h"
#include "decl.h"
#include "expr.h"
#include "init.h"
#include "stmt.h"
#include "name.h"
#include "code.h"
#include "btree.h"
#include "slist.h"
#include "hashmap.h"

int Ebcscript_Parser_yylex(ebcscript_parser *Prs);
static void yyerror(ebcscript_parser *Prs, const char *S);

/*                                                                 マクロ定義 */
/* -------------------------------------------------------------------------- */
#define log_debug0(Message)						\
{									\
	Ebcscript_Parser_log(						\
	 "%s:%d: debug: %s:%d: " Message,				\
	 Prs->Filename, Prs->Line, __FILE__, __LINE__);			\
}

#define log_debug1(Message, Param1)					\
{									\
	Ebcscript_Parser_log(						\
	 "%s:%d: debug: %s:%d: " Message,				\
	 Prs->Filename, Prs->Line, __FILE__, __LINE__,			\
	 Param1);							\
}

#define log_error0(Message)						\
{									\
	Ebcscript_Parser_log(						\
	 "%s:%d: error: " Message,					\
	 Prs->Filename, Prs->Line);					\
}

#define log_error1(Message, Param1)					\
{									\
	Ebcscript_Parser_log(						\
	 "%s:%d: error: " Message,					\
	 Prs->Filename, Prs->Line,					\
	 Param1);							\
}

#define log_warning0(Message)						\
{									\
	Ebcscript_Parser_log(						\
	 "%s:%d: warning: " Message,					\
	 Prs->Filename, Prs->Line);					\
}

#define log_warning1(Message, Param1)					\
{									\
	Ebcscript_Parser_log(						\
	 "%s:%d: warning: " Message,					\
	 Prs->Filename, Prs->Line,					\
	 Param1);							\
}

#define store_instruction(Inst)						\
	Ebcscript_Trnsunit_store_instruction(Prs->TU, Inst);

#define store_address(P)						\
	Ebcscript_Trnsunit_store_address(Prs->TU, P);

#define store_int(I)							\
	Ebcscript_Trnsunit_store_int(Prs->TU, I);

#define store_long(L)							\
	Ebcscript_Trnsunit_store_long(Prs->TU, L);

#define gencode_op_numeric_ptr(Op, TypeTree)				\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                            EBCSCRIPT_INSTRUCTION_ ## Op ## _C);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                           EBCSCRIPT_INSTRUCTION_ ## Op ## _UC);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                            EBCSCRIPT_INSTRUCTION_ ## Op ## _S);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                           EBCSCRIPT_INSTRUCTION_ ## Op ## _US);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                            EBCSCRIPT_INSTRUCTION_ ## Op ## _I);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                           EBCSCRIPT_INSTRUCTION_ ## Op ## _UI);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                            EBCSCRIPT_INSTRUCTION_ ## Op ## _L);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                           EBCSCRIPT_INSTRUCTION_ ## Op ## _UL);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_FLOAT:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                            EBCSCRIPT_INSTRUCTION_ ## Op ## _F);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                            EBCSCRIPT_INSTRUCTION_ ## Op ## _D);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_POINTER:				\
	  case EBCSCRIPT_TYPE_KIND_ARRAY:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                            EBCSCRIPT_INSTRUCTION_ ## Op ## _P);\
	    break;							\
	  default:							\
	    break;							\
	}

#define gencode_op_storage(Op, TypeTree)				\
	switch ((TypeTree)->Kind) {					\
	  case EBCSCRIPT_TYPE_KIND_CHAR:				\
	  case EBCSCRIPT_TYPE_KIND_UCHAR:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                            EBCSCRIPT_INSTRUCTION_ ## Op ## _C);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_SHORT:				\
	  case EBCSCRIPT_TYPE_KIND_USHORT:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                            EBCSCRIPT_INSTRUCTION_ ## Op ## _S);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_INT:					\
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:				\
	  case EBCSCRIPT_TYPE_KIND_UINT:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                            EBCSCRIPT_INSTRUCTION_ ## Op ## _I);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_LONG:				\
	  case EBCSCRIPT_TYPE_KIND_ULONG:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                            EBCSCRIPT_INSTRUCTION_ ## Op ## _L);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_FLOAT:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                            EBCSCRIPT_INSTRUCTION_ ## Op ## _F);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                            EBCSCRIPT_INSTRUCTION_ ## Op ## _D);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_POINTER:				\
	  case EBCSCRIPT_TYPE_KIND_ARRAY:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                            EBCSCRIPT_INSTRUCTION_ ## Op ## _P);\
	    break;							\
	  case EBCSCRIPT_TYPE_KIND_STRUCT:				\
	  case EBCSCRIPT_TYPE_KIND_UNION:				\
	    Ebcscript_Trnsunit_store_instruction(Prs->TU,		\
	                       EBCSCRIPT_INSTRUCTION_ ## Op ## _OBJECT);\
	    Ebcscript_Trnsunit_store_int(Prs->TU,			\
	                         (int)Ebcscript_Type_getSize(TypeTree));\
	    break;							\
	  default:							\
	    break;							\
	}

/*----------------------------------------------------------------------------*/
%}
%name-prefix "Ebcscript_Parser_yy"
%parse-param {ebcscript_parser *Prs}
%lex-param {ebcscript_parser *Prs}

%union {
	int TokenType;
	char *Identifier;
	btree *BTreeInitDeclarator;
	btree *BTreeInitializer;
	btree *BTreeExpression;
	btree *BTreeName;
	ebcscript_type *Type;
	ebcscript_name *Name;
	ebcscript_literal *Literal;
	ebcscript_parser_declaration *Declaration;
	ebcscript_parser_expression *Expression;
}

%destructor { } <*>

%destructor { } <TokenType>

%destructor {
	log_debug0("%%destructor <Identifier>\n")
	Ebcscript_Name_free($$);
} <Identifier>

%destructor {
	btree *P;

	log_debug0("%%destructor <BTreeInitDeclarator>\n")
	for (P = $$; P != NULL; P = P->Right) {
	  Ebcscript_Parser_deleteInitializer(P->Left);
	  Ebcscript_deleteName(P->Datum);
	}
	BTree_clear($$, &BTree_clearDummy);
} <BTreeInitDeclarator>

%destructor {
	log_debug0("%%destructor <BTreeInitializer>\n")
	Ebcscript_Parser_deleteInitializer($$);
} <BTreeInitializer>

%destructor {
	log_debug0("%%destructor <BTreeExpression>\n")
	BTree_clear($$, (void (*)(void *))Ebcscript_Parser_deleteExpression);
} <BTreeExpression>

%destructor {
	log_debug0("%%destructor <BTreeName>\n")
	BTree_clear($$, (void (*)(void *))Ebcscript_deleteName);
} <BTreeName>

%destructor {
	log_debug0("%%destructor <Type>\n")
	Ebcscript_deleteType($$);
} <Type>

%destructor {
	log_debug0("%%destructor <Name>\n")
	Ebcscript_deleteName($$);
} <Name>

%destructor {
	log_debug0("%%destructor <Literal>\n")
	Ebcscript_deleteLiteral($$);
} <Literal>

%destructor {
	log_debug0("%%destructor <Declaration>\n")
	Ebcscript_Parser_deleteDeclaration($$);
} <Declaration>

%destructor {
	log_debug0("%%destructor <Expression>\n")
	Ebcscript_Parser_deleteExpression($$);
} <Expression>

%type <Declaration> declaration_specifiers
%type <Declaration> type_specifier
%type <Declaration> specifier_qualifier_list

%type <Expression> primary_expression
%type <Expression> postfix_expression
%type <Expression> unary_expression
%type <Expression> cast_expression
%type <Expression> multiplicative_expression
%type <Expression> additive_expression
%type <Expression> shift_expression
%type <Expression> relational_expression
%type <Expression> equality_expression
%type <Expression> and_expression
%type <Expression> exclusive_or_expression
%type <Expression> inclusive_or_expression
%type <Expression> logical_and_expression
%type <Expression> logical_or_expression
%type <Expression> conditional_expression
%type <Expression> assignment_expression
%type <Expression> expression
%type <Expression> constant_expression

%type <TokenType> storage_class_specifier
%type <TokenType> type_qualifier
%type <TokenType> struct_or_union
%type <TokenType> unary_operator
%type <TokenType> assignment_operator

%type <BTreeExpression> argument_expression_list

%type <BTreeInitializer> initializer_list
%type <BTreeInitializer> initializer

%type <BTreeInitDeclarator> init_declarator_list
%type <BTreeInitDeclarator> init_declarator

%type <BTreeName> struct_declaration_list
%type <BTreeName> struct_declaration
%type <BTreeName> struct_declarator_list
%type <BTreeName> parameter_type_list
%type <BTreeName> parameter_list

%type <Type> pointer
%type <Type> struct_or_union_specifier
%type <Type> type_name
%type <Type> abstract_declarator
%type <Type> direct_abstract_declarator
%type <Type> enum_specifier
%type <Type> typedef_name

%type <Name> declarator
%type <Name> direct_declarator
%type <Name> struct_declarator
%type <Name> enumerator
%type <Name> parameter_declaration

%token <Identifier> IDENTIFIER
%token <Name> TYPEDEF_NAME

%token <Literal> CONSTANT
%token <Literal> STRING_LITERAL

%token
	TYPEDEF
	EXTERN
	STATIC

%token
	VOID
	CHAR
	SHORT
	INT
	LONG
	FLOAT
	DOUBLE
	SIGNED
	UNSIGNED

%token
	STRUCT
	UNION
	ENUM

%token
	CONST
	VOLATILE

%token
	BREAK
	CASE
	CONTINUE
	DEFAULT
	DO
	ELSE
	FOR
	GOTO
	IF
	RETURN
	SWITCH
	WHILE

%token
	RIGHT_OP
	LEFT_OP
	INC_OP
	DEC_OP
	PTR_OP
	AND_OP
	OR_OP
	LE_OP
	GE_OP
	EQ_OP
	NE_OP

%token
	SIZEOF

%token
	RIGHT_ASSIGN
	LEFT_ASSIGN
	ADD_ASSIGN
	SUB_ASSIGN
	MUL_ASSIGN
	DIV_ASSIGN
	MOD_ASSIGN
	AND_ASSIGN
	XOR_ASSIGN
	OR_ASSIGN

%start translation_unit
%%
/*--------------------------------------------------------------- Expressions */
primary_expression
	: IDENTIFIER
	  {
	    ebcscript_parser_expression *E;
	    ebcscript_name **N0, *N;
	    ebcscript_parser_blockstack *BS;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Name_free($1);
	      YYABORT;
	    }

	    /* 記号表チェーンにあるか？ */
	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(BS,
	                                                                  $1)) {
	        N = *N0;
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_error1("\'%s\' undeclared\n", $1)
	      longjmp(Prs->CheckPoint, 1);
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ID;
	    switch (N->Kind) {
	      case EBCSCRIPT_NAME_KIND_VARIABLE:
	        E->TypeTree = N->As.Variable.TypeTree;
	        E->IsConstant = false;
	        E->As.Identifier.Name = N;
	        break;
	      case EBCSCRIPT_NAME_KIND_FUNCTION:
	        E->TypeTree = N->As.Function.TypeTree;
	        E->IsConstant = false;
	        E->As.Identifier.Name = N;
	        break;
	      case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	        E->TypeTree = N->As.Enumerator.TypeTree;
	        E->IsConstant = true;
	        E->As.Identifier.Name = N;
	        break;
	      case EBCSCRIPT_NAME_KIND_TYPEDEF:
	        /* [lex.l]で除外される */
	      default:
	        log_debug0("")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }
	    $$ = E;
	    Ebcscript_Name_free($1);
	  }
	| CONSTANT
	  {
	    ebcscript_parser_expression *E;

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_CONST;
	    E->TypeTree = $1->TypeTree;
	    E->IsConstant = true;
	    E->As.Constant.Literal = $1;
	    $$ = E;
	  }
	| STRING_LITERAL
	  {
	    ebcscript_parser_expression *E;
	    ebcscript_literal **L0;

	    if (L0 = (ebcscript_literal **)
	                            Hashmap_find(Prs->TU->ConstTbl, $1->Text)) {
	      Ebcscript_deleteLiteral($1);
	      $1 = *L0;
	    } else {
	      /* 定数表に登録 */
	      Hashmap_add(Prs->TU->ConstTbl, $1->Text, $1);
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_CONST;
	    E->TypeTree = $1->TypeTree;
	    E->IsConstant = true;
	    E->As.Constant.Literal = $1;
	    $$ = E;
	  }
	| '(' expression ')'
	  {
	    $$ = $2;
	  }
	;

postfix_expression
	: primary_expression
	  {
	    $$ = $1;
	  }
	| postfix_expression '[' expression ']'
	  {
	    ebcscript_parser_expression *E;
	    ebcscript_type *TL, *Value;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    if (!Ebcscript_Type_isInteger($3->TypeTree)) {
	      log_error0("array subscript is not intger\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    switch ($1->TypeTree->Kind) {
	      case EBCSCRIPT_TYPE_KIND_ARRAY:
	        Value = $1->TypeTree->As.Array.Value;
	        break;
	      case EBCSCRIPT_TYPE_KIND_POINTER:
	        Value = $1->TypeTree->As.Pointer.Value;
	        break;
	      default:
	        log_error0("subscripted value is neither array "
	                                             "nor pointer nor vector\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }

	    /* $3にキャストを挟んでlongにする */
	    TL = Ebcscript_Type_Long();
	    if (!Ebcscript_Type_equals(TL, $3->TypeTree)) {
	      $3 = Ebcscript_Parser_newExpression_cast(TL, $3);
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_INDEX;
	    E->TypeTree = Value;
	    E->IsConstant = false;
	    E->As.Index.Left = $1;
	    E->As.Index.Right = $3;

	    $$ = E;
	  }
	| postfix_expression '(' ')'
	  {
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      YYABORT;
	    }

	    if ($1->TypeTree->Kind != EBCSCRIPT_TYPE_KIND_FUNCTION) {
	      log_error0("called a object is not a function or "
	                                                   "function pointer\n")
	      longjmp(Prs->CheckPoint, 1);
	    }
	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_FUNC;
	    E->TypeTree = $1->TypeTree->As.Function.ReturnValue;
	    E->IsConstant = false;
	    E->As.Func.Child = $1;
	    E->As.Func.Arguments = NULL;
	    $$ = E;
	  }
	| postfix_expression '(' argument_expression_list ')'
	  {
	    ebcscript_parser_expression *E, *C;
	    slist_cell *P;
	    btree *Q;
	    ebcscript_type *PT, *QT;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      for (Q = $3; Q != NULL; Q = Q->Right) {
	        Ebcscript_Parser_deleteExpression(Q->Datum);
	      }
	      BTree_clear($3, &BTree_clearDummy);
/*	      BTree_clear($3, (void (*)(void *))Ebcscript_deleteExpression);*/
	      YYABORT;
	    }

	    if ($1->TypeTree->Kind != EBCSCRIPT_TYPE_KIND_FUNCTION) {
	      log_error0("called a object is not a function or "
	                                                   "function pointer\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* 引数の型の一致をチェック */
	    P = $1->TypeTree->As.Function.Parameters->Head.Next;
	    Q = $3;
	    while (P != NULL && Q != NULL) {
	      switch (((ebcscript_name *)P->Datum)->Kind) {
	        case EBCSCRIPT_NAME_KIND_VARIABLE:
	          PT = ((ebcscript_name *)P->Datum)->As.Variable.TypeTree;
	          break;
	        case EBCSCRIPT_NAME_KIND_FUNCTION:
	          PT = ((ebcscript_name *)P->Datum)->As.Function.TypeTree;
	          break;
	      }
	      QT = ((ebcscript_parser_expression *)Q->Datum)->TypeTree;

	      /* 数値型とその他で場合分け */
	      if (Ebcscript_Type_isNumeric(PT)
	       && Ebcscript_Type_isNumeric(QT)) {
	        if (!Ebcscript_Type_equals(PT, QT)) {
	          Q->Datum = Ebcscript_Parser_newExpression_cast(PT, Q->Datum);
	        }
	      } else {
	        if (!Ebcscript_Type_equals(PT, QT)) {
	          log_error1(
	           "incompatible type for argument of \'%s\'\n",
	           ((ebcscript_name *)P->Datum)->Identifier)
	          longjmp(Prs->CheckPoint, 1);
	        }
	      }

	      P = P->Next;
	      Q = Q->Right;
	    }
	    if (P == NULL && Q != NULL) {
	      log_error0("too many arguments to the function\n")
	      longjmp(Prs->CheckPoint, 1);
	    }
	    if (P != NULL && Q == NULL) {
	      log_error0("too few arguments to the function\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_FUNC;
	    E->TypeTree = $1->TypeTree->As.Function.ReturnValue;
	    E->IsConstant = false;
	    E->As.Func.Child = $1;
	    E->As.Func.Arguments = $3;
	    $$ = E;
	  }
	| postfix_expression '.' IDENTIFIER
	  {
	    ebcscript_parser_expression *E;
	    ebcscript_name *N;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Name_free($3);
	      YYABORT;
	    }

	    if ($1->TypeTree->Kind != EBCSCRIPT_TYPE_KIND_STRUCT
	     && $1->TypeTree->Kind != EBCSCRIPT_TYPE_KIND_UNION) {
	      log_error1(
	       "request for member \'%s\' in something not"
	                                              " a structure or union\n",
	       $3)
	      longjmp(Prs->CheckPoint, 1);
	    }

	    switch ($1->TypeTree->Kind) {
	      case EBCSCRIPT_TYPE_KIND_STRUCT:
	        if (!(N = Ebcscript_Type_Struct_findMember(
	                                   &($1->TypeTree->As.Struct), $3))) {
	          log_error1("the struct has no member named \'%s\'\n", $3)
	          longjmp(Prs->CheckPoint, 1);
	        }
	        break;
	      case EBCSCRIPT_TYPE_KIND_UNION:
	        if (!(N = Ebcscript_Type_Union_findMember(
	                                    &($1->TypeTree->As.Union), $3))) {
	          log_error1("the struct has no member named \'%s\'\n", $3)
	          longjmp(Prs->CheckPoint, 1);
	        }
	        break;
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_DOT;
	    E->IsConstant = $1->IsConstant;
	    E->As.Dot.Child = $1;
	    switch (N->Kind) {
	      case EBCSCRIPT_NAME_KIND_VARIABLE:
	        E->TypeTree = N->As.Variable.TypeTree;
	        E->As.Dot.Offset = (int)N->As.Variable.Address;
	        break;
	      case EBCSCRIPT_NAME_KIND_FUNCTION:
	        /* struct_declaratorで弾いているので来ないはず */
	        E->TypeTree = N->As.Function.TypeTree;
	        E->As.Dot.Offset = (int)N->As.Function.CodeAddress;
	        break;
	    }
	    Ebcscript_Name_free($3);
	    $$ = E;
	  }
	| postfix_expression PTR_OP IDENTIFIER
	  {
	    ebcscript_parser_expression *E;
	    ebcscript_name *N;
	    ebcscript_type *Value;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Name_free($3);
	      YYABORT;
	    }

	    if (!Ebcscript_Type_isPointer($1->TypeTree)) {
	      log_error0("invalid type argument of \'->\'\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    Value = $1->TypeTree->As.Pointer.Value;

	    if (Value->Kind != EBCSCRIPT_TYPE_KIND_STRUCT
	     && Value->Kind != EBCSCRIPT_TYPE_KIND_UNION) { 
	      log_error1(
	       "request for member \'%s\' in something not"
	                                              " a structure or union\n",
	       $3);
	      longjmp(Prs->CheckPoint, 1);
	    }

	    switch (Value->Kind) {
	      case EBCSCRIPT_TYPE_KIND_STRUCT:
	        if (!(N = Ebcscript_Type_Struct_findMember(
	                                          &(Value->As.Struct), $3))) {
	          log_error1("the union has no member named \'%s\'\n", $3);
	          longjmp(Prs->CheckPoint, 1);
	        }
	        break;
	      case EBCSCRIPT_TYPE_KIND_UNION:
	        if (!(N = Ebcscript_Type_Union_findMember(
	                                           &(Value->As.Union), $3))) {
	          log_error1("the union has no member named \'%s\'\n", $3)
	          longjmp(Prs->CheckPoint, 1);
	        }
	        break;
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ARROW;
	    E->IsConstant = $1->IsConstant;
	    E->As.Arrow.Child = $1;
	    switch (N->Kind) {
	      case EBCSCRIPT_NAME_KIND_VARIABLE:
	        E->TypeTree = N->As.Variable.TypeTree;
	        E->As.Arrow.Offset = (int)N->As.Variable.Address;
	        break;
	      case EBCSCRIPT_NAME_KIND_FUNCTION:
	        /* struct_declaratorで弾いているので来ないはず */
	        E->TypeTree = N->As.Function.TypeTree;
	        E->As.Arrow.Offset = (int)N->As.Function.CodeAddress;
	        break;
	    }
	    Ebcscript_Name_free($3);
	    $$ = E;
	  }
	| postfix_expression INC_OP
	  {
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      YYABORT;
	    }

	    if (!Ebcscript_Type_isNumeric($1->TypeTree)
	     && !Ebcscript_Type_isPointer($1->TypeTree)) {
	      log_error0("wrong type argument to increment\n")
	      longjmp(Prs->CheckPoint, 1);
	    }
	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_POST;
	    E->IsConstant = $1->IsConstant;
	    E->TypeTree = $1->TypeTree;
	    E->As.IncPost.Child = $1;
	    $$ = E;
	  }
	| postfix_expression DEC_OP
	  {
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      YYABORT;
	    }

	    if (!Ebcscript_Type_isNumeric($1->TypeTree)
	     && !Ebcscript_Type_isPointer($1->TypeTree)) {
	      log_error0("wrong type argument to decrement\n")
	      longjmp(Prs->CheckPoint, 1);
	    }
	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_POST;
	    E->IsConstant = $1->IsConstant;
	    E->TypeTree = $1->TypeTree;
	    E->As.DecPost.Child = $1;
	    $$ = E;
	  }
	;

argument_expression_list
	: assignment_expression
	  {
	    $$ = newBTree($1, NULL, NULL);
	  }
	| argument_expression_list ',' assignment_expression
	  {
	    btree *P;

	    /* $1の末尾に$3を追加する。 */
	    for (P = $1; P->Right != NULL; P = P->Right)
	      ;
	    P->Right = newBTree($3, NULL, NULL);
	    $$ = $1;
	  }
	;

unary_expression
	: postfix_expression
	  {
	    $$ = $1;
	  }
	| INC_OP unary_expression
	  {
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($2);
	      YYABORT;
	    }

	    if (!Ebcscript_Type_isNumeric($2->TypeTree)
	     && !Ebcscript_Type_isPointer($2->TypeTree)) {
	      log_error0("wrong type argument to increment\n")
	      longjmp(Prs->CheckPoint, 1);
	    }
	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_INC_PRE;
	    E->IsConstant = $2->IsConstant;
	    E->TypeTree = $2->TypeTree;
	    E->As.IncPre.Child = $2;
	    $$ = E;
	  }
	| DEC_OP unary_expression
	  {
	    ebcscript_parser_expression *E;

	    if (!Ebcscript_Type_isNumeric($2->TypeTree)
	     && !Ebcscript_Type_isPointer($2->TypeTree)) {
	      log_error0("wrong type argument to decrement\n")
	      Ebcscript_Parser_deleteExpression($2);
	      YYABORT;
	    }
	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_DEC_PRE;
	    E->IsConstant = $2->IsConstant;
	    E->TypeTree = $2->TypeTree;
	    E->As.DecPre.Child = $2;
	    $$ = E;
	  }
	| unary_operator cast_expression
	  {
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($2);
	      YYABORT;
	    }

	    E = Ebcscript_Parser_newExpression();
	    switch ($1) {
	      case '&':
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ADDR;
	        /* TypeTreeの先頭にPOINTERを追加 */
	        /* deleteExpression()で削除する */
	        E->TypeTree = Ebcscript_newType_pointer();
	        E->TypeTree->As.Pointer.Value =
	                                       Ebcscript_Type_dup($2->TypeTree);
	        E->IsConstant = $2->IsConstant;
	        E->As.Addr.Child = $2;
	        break;
	      case '*':
	        if ($2->TypeTree->Kind != EBCSCRIPT_TYPE_KIND_POINTER) {
	          log_error0("invalid type argument of unary \'*\'\n")
	          longjmp(Prs->CheckPoint, 1);
	        }
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_PTR;
	        /* 先頭の次の要素を指す */
	        E->TypeTree = $2->TypeTree->As.Pointer.Value;
	        E->IsConstant = false;
	        E->As.Ptr.Child = $2;
	        break;
	      case '+':
	        if (!Ebcscript_Type_isNumeric($2->TypeTree)) {
	          log_error0("wrong type argument to unary plus\n")
	          longjmp(Prs->CheckPoint, 1);
	        }
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_UPLUS;
	        E->TypeTree = $2->TypeTree;
	        E->IsConstant = $2->IsConstant;
	        E->As.UPlus.Child = $2;
	        break;
	      case '-':
	        if (!Ebcscript_Type_isNumeric($2->TypeTree)) {
	          log_error0("wrong type argument to unary plus\n")
	          longjmp(Prs->CheckPoint, 1);
	        }
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_UMINUS;
	        E->TypeTree = $2->TypeTree;
	        E->IsConstant = $2->IsConstant;
	        E->As.UMinus.Child = $2;
	        break;
	      case '~':
	        if (!Ebcscript_Type_isInteger($2->TypeTree)) {
	          log_error0("wrong type argument to bit-complement\n")
	          longjmp(Prs->CheckPoint, 1);
	        }
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_NOT;
	        E->TypeTree = $2->TypeTree;
	        E->IsConstant = $2->IsConstant;
	        E->As.Not.Child = $2;
	        break;
	      case '!':
	        if (!Ebcscript_Type_isNumeric($2->TypeTree)
	         && !Ebcscript_Type_isPointer($2->TypeTree)) {
	          log_error0("wrong type argument to unary exclamation mark\n")
	          longjmp(Prs->CheckPoint, 1);
	        }
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_NOT;
	        E->TypeTree = Ebcscript_Type_Int();
	        E->IsConstant = $2->IsConstant;
	        E->As.LogNot.Child = $2;
	        break;
	    }
	    $$ = E;
	  }
	| SIZEOF unary_expression
	  {
	    ebcscript_parser_expression *E;

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_EXPR;
	    E->TypeTree = Ebcscript_Type_Int();
	    E->IsConstant = $2->IsConstant;
	    E->As.SizeofExpr.Size = Ebcscript_Type_getSize($2->TypeTree);
	    E->As.SizeofExpr.Child = $2;
	    $$ = E;
	  }
	| SIZEOF '(' type_name ')'
	  {
	    ebcscript_parser_expression *E;

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_SIZEOF_TYPE;
	    E->TypeTree = Ebcscript_Type_Int();
	    E->IsConstant = true;
	    E->As.SizeofType.Size = Ebcscript_Type_getSize($3);
	    E->As.SizeofType.TypeTree = $3;
	    $$ = E;
	  }
	;

unary_operator
	: '&'
	  {
	    $$ = '&';
	  }
	| '*'
	  {
	    $$ = '*';
	  }
	| '+'
	  {
	    $$ = '+';
	  }
	| '-'
	  {
	    $$ = '-';
	  }
	| '~'
	  {
	    $$ = '~';
	  }
	| '!'
	  {
	    $$ = '!';
	  }
	;

cast_expression
	: unary_expression
	  {
	    $$ = $1;
	  }
	| '(' type_name ')' cast_expression
	  {
	    ebcscript_parser_expression *E;

	    E = Ebcscript_Parser_newExpression_cast($2, $4);
	    $$ = E;
	  }
	;

multiplicative_expression
	: cast_expression
	  {
	    $$ = $1;
	  }
	| multiplicative_expression '*' cast_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* 型が一致しないときは、キャスト変換のノードを挿入する */
	    if (!Ebcscript_Type_isNumeric($1->TypeTree)
	     || !Ebcscript_Type_isNumeric($3->TypeTree)) {
	      log_error0("invalid operands to binary *\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* ノードの型を決定する */
	    T1 = $1->TypeTree;
	    T2 = $3->TypeTree;
	    TE = Ebcscript_Type_balance_numeric(T1, T2);

	    /* キャストを挟む */
	    if (!Ebcscript_Type_equals(TE, T1)) {
	      $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	    }
	    if (!Ebcscript_Type_equals(TE, T2)) {
	      $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_MUL;
	    E->TypeTree = TE;
	    E->IsConstant = $1->IsConstant && $3->IsConstant;
	    E->As.Mul.Left = $1;
	    E->As.Mul.Right = $3;
	    $$ = E;
	  }
	| multiplicative_expression '/' cast_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    if (!Ebcscript_Type_isNumeric($1->TypeTree)
	     || !Ebcscript_Type_isNumeric($3->TypeTree)) {
	      log_error0("invalid operands to binary /\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* ノードの型を決定する */
	    T1 = $1->TypeTree;
	    T2 = $3->TypeTree;
	    TE = Ebcscript_Type_balance_numeric(T1, T2);

	    /* キャストを挟む */
	    if (!Ebcscript_Type_equals(TE, T1)) {
	      $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	    }
	    if (!Ebcscript_Type_equals(TE, T2)) {
	      $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_DIV;
	    E->TypeTree = TE;
	    E->IsConstant = $1->IsConstant && $3->IsConstant;
	    E->As.Div.Left = $1;
	    E->As.Div.Right = $3;
	    $$ = E;
	  }
	| multiplicative_expression '%' cast_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* '*', '/'と異なり被演算数は整数型 */
	    if (!Ebcscript_Type_isInteger($1->TypeTree)
	     || !Ebcscript_Type_isInteger($3->TypeTree)) {
	      log_error0("invalid operands to binary \%\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* ノードの型を決定する */
	    T1 = $1->TypeTree;
	    T2 = $3->TypeTree;
	    TE = Ebcscript_Type_balance_integer(T1, T2);

	    /* キャストを挟む */
	    if (!Ebcscript_Type_equals(TE, T1)) {
	      $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	    }
	    if (!Ebcscript_Type_equals(TE, T2)) {
	      $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_MOD;
	    E->TypeTree = TE;
	    E->IsConstant = $1->IsConstant && $3->IsConstant;
	    E->As.Mod.Left = $1;
	    E->As.Mod.Right = $3;
	    $$ = E;
	  }
	;

additive_expression
	: multiplicative_expression
	  {
	    $$ = $1;
	  }
	| additive_expression '+' multiplicative_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* 被演算数は算術型かポインタ型。但し、両方ポインタ型は不可。 */
	    if (!( Ebcscript_Type_isNumeric($1->TypeTree)
	        && Ebcscript_Type_isNumeric($3->TypeTree)
	        || Ebcscript_Type_isPointer($1->TypeTree)
	        && Ebcscript_Type_isInteger($3->TypeTree)
	        || Ebcscript_Type_isInteger($1->TypeTree)
	        && Ebcscript_Type_isPointer($3->TypeTree) ) ) {
	      log_error0("invalid operands to binary +\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* (算術型, 算術型) */
	    if (Ebcscript_Type_isNumeric($1->TypeTree)
	     && Ebcscript_Type_isNumeric($3->TypeTree)) {
	      /* ノードの型を決定する */
	      T1 = $1->TypeTree;
	      T2 = $3->TypeTree;
	      TE = Ebcscript_Type_balance_numeric(T1, T2);

	      /* キャストを挟む */
	      if (!Ebcscript_Type_equals(TE, T1)) {
	        $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	      }
	      if (!Ebcscript_Type_equals(TE, T2)) {
	        $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	      }

	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ADD;
	      E->TypeTree = TE;
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Add.Left = $1;
	      E->As.Add.Right = $3;
	    }

	    /* (ポインタ型, 整数型) */
	    if (Ebcscript_Type_isPointer($1->TypeTree)
	     && Ebcscript_Type_isInteger($3->TypeTree)) {

	      TE = Ebcscript_Type_Long();
	      if (!Ebcscript_Type_equals(TE, $3->TypeTree)) {
	        $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	      }

	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ADD;
	      E->TypeTree = $1->TypeTree;
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Add.Left = $1;
	      E->As.Add.Right = $3;
	    }

	    /* (整数型, ポインタ型) */
	    if (Ebcscript_Type_isInteger($1->TypeTree)
	     && Ebcscript_Type_isPointer($3->TypeTree)) {

	      TE = Ebcscript_Type_Long();
	      if (!Ebcscript_Type_equals(TE, $1->TypeTree)) {
	        $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	      }

	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ADD;
	      E->TypeTree = $3->TypeTree;
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Add.Left = $3;
	      E->As.Add.Right = $1;
	    }

	    $$ = E;
	  }
	| additive_expression '-' multiplicative_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    if (!( Ebcscript_Type_isNumeric($1->TypeTree)
	        && Ebcscript_Type_isNumeric($3->TypeTree)
	        || Ebcscript_Type_isPointer($1->TypeTree)
	        && Ebcscript_Type_isInteger($3->TypeTree) ) ) {
	      log_error0("invalid operands to binary -\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* (算術型, 算術型) */
	    if (Ebcscript_Type_isNumeric($1->TypeTree)
	     && Ebcscript_Type_isNumeric($3->TypeTree)) {
	      /* ノードの型を決定する */
	      T1 = $1->TypeTree;
	      T2 = $3->TypeTree;
	      TE = Ebcscript_Type_balance_numeric(T1, T2);

	      /* キャストを挟む */
	      if (!Ebcscript_Type_equals(TE, T1)) {
	        $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	      }
	      if (!Ebcscript_Type_equals(TE, T2)) {
	        $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	      }

	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_SUB;
	      E->TypeTree = TE;
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Sub.Left = $1;
	      E->As.Sub.Right = $3;
	    }

	    /* (ポインタ型, 整数型) */
	    if (Ebcscript_Type_isPointer($1->TypeTree)
	     && Ebcscript_Type_isInteger($3->TypeTree)) {

	      TE = Ebcscript_Type_Long();
	      if (!Ebcscript_Type_equals(TE, $3->TypeTree)) {
	        $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	      }

	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_SUB;
	      E->TypeTree = $1->TypeTree;
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Sub.Left = $1;
	      E->As.Sub.Right = $3;
	    }

	    $$ = E;
	  }
	;

shift_expression
	: additive_expression
	  {
	    $$ = $1;
	  }
	| shift_expression LEFT_OP additive_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* 型が一致しないときは、キャスト変換のノードを挿入する */
	    if (!Ebcscript_Type_isInteger($1->TypeTree)
	     || !Ebcscript_Type_isInteger($3->TypeTree)) {
	      log_error0("invalid operands to binary <<\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* ノードの型を決定する */
	    T1 = $1->TypeTree;
	    T2 = $3->TypeTree;
	    TE = Ebcscript_Type_balance_integer(T1, T2);

	    /* キャストを挟む */
	    if (!Ebcscript_Type_equals(TE, T1)) {
	      $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	    }
	    if (!Ebcscript_Type_equals(TE, T2)) {
	      $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_L;
	    E->TypeTree = TE;
	    E->IsConstant = $1->IsConstant && $3->IsConstant;
	    E->As.ShftL.Left = $1;
	    E->As.ShftL.Right = $3;
	    $$ = E;
	  }
	| shift_expression RIGHT_OP additive_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* 型が一致しないときは、キャスト変換のノードを挿入する */
	    if (!Ebcscript_Type_isInteger($1->TypeTree)
	     || !Ebcscript_Type_isInteger($3->TypeTree)) {
	      log_error0("invalid operands to binary >>\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* ノードの型を決定する */
	    T1 = $1->TypeTree;
	    T2 = $3->TypeTree;
	    TE = Ebcscript_Type_balance_integer(T1, T2);

	    /* キャストを挟む */
	    if (!Ebcscript_Type_equals(TE, T1)) {
	      $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	    }
	    if (!Ebcscript_Type_equals(TE, T2)) {
	      $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_R;
	    E->TypeTree = TE;
	    E->IsConstant = $1->IsConstant && $3->IsConstant;
	    E->As.ShftR.Left = $1;
	    E->As.ShftR.Right = $3;
	    $$ = E;
	  }
	;

relational_expression
	: shift_expression
	  {
	    $$ = $1;
	  }
	| relational_expression '<' shift_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* 型が一致しないときは、キャスト変換のノードを挿入する */
	    if (!(Ebcscript_Type_isNumeric($1->TypeTree)
	       && Ebcscript_Type_isNumeric($3->TypeTree)
	       || Ebcscript_Type_isPointer($1->TypeTree)
	       && Ebcscript_Type_isPointer($3->TypeTree) ) ) {
	      log_error0("invalid operands to binary <\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    if (Ebcscript_Type_isNumeric($1->TypeTree)
	     && Ebcscript_Type_isNumeric($3->TypeTree)) {
	      /* ノードの型を決定する */
	      T1 = $1->TypeTree;
	      T2 = $3->TypeTree;
	      TE = Ebcscript_Type_balance_numeric(T1, T2);

	      /* キャストを挟む */
	      if (!Ebcscript_Type_equals(TE, T1)) {
	        $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	      }
	      if (!Ebcscript_Type_equals(TE, T2)) {
	        $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	      }

	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_LT;
	      E->TypeTree = Ebcscript_Type_Int();
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Lt.Left = $1;
	      E->As.Lt.Right = $3;
	    }

	    if (Ebcscript_Type_isPointer($1->TypeTree)
	     && Ebcscript_Type_isPointer($3->TypeTree)) {
	      if (!Ebcscript_Type_equals($1->TypeTree, $3->TypeTree)) {
	        log_warning0("comparison of distinct pointer types lacks a"
	                                                              " cast\n")
	        longjmp(Prs->CheckPoint, 1);
	      }
	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_LT;
	      E->TypeTree = Ebcscript_Type_Int();
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Lt.Left = $1;
	      E->As.Lt.Right = $3;
	    }

	    $$ = E;
	  }
	| relational_expression '>' shift_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* 型が一致しないときは、キャスト変換のノードを挿入する */
	    if (!(Ebcscript_Type_isNumeric($1->TypeTree)
	       && Ebcscript_Type_isNumeric($3->TypeTree)
	       || Ebcscript_Type_isPointer($1->TypeTree)
	       && Ebcscript_Type_isPointer($3->TypeTree) ) ) {
	      log_error0("invalid operands to binary >\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    if (Ebcscript_Type_isNumeric($1->TypeTree)
	     && Ebcscript_Type_isNumeric($3->TypeTree)) {
	      /* ノードの型を決定する */
	      T1 = $1->TypeTree;
	      T2 = $3->TypeTree;
	      TE = Ebcscript_Type_balance_numeric(T1, T2);

	      /* キャストを挟む */
	      if (!Ebcscript_Type_equals(TE, T1)) {
	        $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	      }
	      if (!Ebcscript_Type_equals(TE, T2)) {
	        $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	      }

	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_GT;
	      E->TypeTree = Ebcscript_Type_Int();
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Gt.Left = $1;
	      E->As.Gt.Right = $3;
	    }

	    if (Ebcscript_Type_isPointer($1->TypeTree)
	     && Ebcscript_Type_isPointer($3->TypeTree)) {
	      if (!Ebcscript_Type_equals($1->TypeTree, $3->TypeTree)) {
	        log_error0("comparison of distinct pointer types lacks a"
	                                                              " cast\n")
	        longjmp(Prs->CheckPoint, 1);
	      }
	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_GT;
	      E->TypeTree = Ebcscript_Type_Int();
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Gt.Left = $1;
	      E->As.Gt.Right = $3;
	    }

	    $$ = E;
	  }
	| relational_expression LE_OP shift_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* 型が一致しないときは、キャスト変換のノードを挿入する */
	    if (!(Ebcscript_Type_isNumeric($1->TypeTree)
	       && Ebcscript_Type_isNumeric($3->TypeTree)
	       || Ebcscript_Type_isPointer($1->TypeTree)
	       && Ebcscript_Type_isPointer($3->TypeTree) ) ) {
	      log_error0("invalid operands to binary <=\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    if (Ebcscript_Type_isNumeric($1->TypeTree)
	     && Ebcscript_Type_isNumeric($3->TypeTree)) {
	      /* ノードの型を決定する */
	      T1 = $1->TypeTree;
	      T2 = $3->TypeTree;
	      TE = Ebcscript_Type_balance_numeric(T1, T2);

	      /* キャストを挟む */
	      if (!Ebcscript_Type_equals(TE, T1)) {
	        $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	      }
	      if (!Ebcscript_Type_equals(TE, T2)) {
	        $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	      }

	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_LE;
	      E->TypeTree = Ebcscript_Type_Int();
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Le.Left = $1;
	      E->As.Le.Right = $3;
	    }

	    if (Ebcscript_Type_isPointer($1->TypeTree)
	     && Ebcscript_Type_isPointer($3->TypeTree)) {
	      if (!Ebcscript_Type_equals($1->TypeTree, $3->TypeTree)) {
	        log_warning0("comparison of distinct pointer types lacks a"
	                                                              " cast\n")
	        longjmp(Prs->CheckPoint, 1);
	      }
	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_LE;
	      E->TypeTree = Ebcscript_Type_Int();
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Le.Left = $1;
	      E->As.Le.Right = $3;
	    }

	    $$ = E;
	  }
	| relational_expression GE_OP shift_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* 型が一致しないときは、キャスト変換のノードを挿入する */
	    if (!(Ebcscript_Type_isNumeric($1->TypeTree)
	       && Ebcscript_Type_isNumeric($3->TypeTree)
	       || Ebcscript_Type_isPointer($1->TypeTree)
	       && Ebcscript_Type_isPointer($3->TypeTree) ) ) {
	      log_error0("invalid operands to binary >=\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    if (Ebcscript_Type_isNumeric($1->TypeTree)
	     && Ebcscript_Type_isNumeric($3->TypeTree)) {
	      /* ノードの型を決定する */
	      T1 = $1->TypeTree;
	      T2 = $3->TypeTree;
	      TE = Ebcscript_Type_balance_numeric(T1, T2);

	      /* キャストを挟む */
	      if (!Ebcscript_Type_equals(TE, T1)) {
	        $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	      }
	      if (!Ebcscript_Type_equals(TE, T2)) {
	        $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	      }

	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_GE;
	      E->TypeTree = Ebcscript_Type_Int();
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Ge.Left = $1;
	      E->As.Ge.Right = $3;
	    }

	    if (Ebcscript_Type_isPointer($1->TypeTree)
	     && Ebcscript_Type_isPointer($3->TypeTree)) {
	      if (!Ebcscript_Type_equals($1->TypeTree, $3->TypeTree)) {
	        log_warning0("comparison of distinct pointer types lacks a"
	                                                              " cast\n")
	        longjmp(Prs->CheckPoint, 1);
	      }
	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_GE;
	      E->TypeTree = Ebcscript_Type_Int();
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Ge.Left = $1;
	      E->As.Ge.Right = $3;
	    }

	    $$ = E;
	  }
	;

equality_expression
	: relational_expression
	  {
	    $$ = $1;
	  }
	| equality_expression EQ_OP relational_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* 型が一致しないときは、キャスト変換のノードを挿入する */
	    if (!(Ebcscript_Type_isNumeric($1->TypeTree)
	       && Ebcscript_Type_isNumeric($3->TypeTree)
	       || Ebcscript_Type_isPointer($1->TypeTree)
	       && Ebcscript_Type_isPointer($3->TypeTree) ) ) {
	      log_error0("invalid operands to binary ==\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    if (Ebcscript_Type_isNumeric($1->TypeTree)
	     && Ebcscript_Type_isNumeric($3->TypeTree)) {
	      /* ノードの型を決定する */
	      T1 = $1->TypeTree;
	      T2 = $3->TypeTree;
	      TE = Ebcscript_Type_balance_numeric(T1, T2);

	      /* キャストを挟む */
	      if (!Ebcscript_Type_equals(TE, T1)) {
	        $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	      }
	      if (!Ebcscript_Type_equals(TE, T2)) {
	        $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	      }

	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_EQ;
	      E->TypeTree = Ebcscript_Type_Int();
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Eq.Left = $1;
	      E->As.Eq.Right = $3;
	    }

	    if (Ebcscript_Type_isPointer($1->TypeTree)
	     && Ebcscript_Type_isPointer($3->TypeTree)) {
	      if (!Ebcscript_Type_equals($1->TypeTree, $3->TypeTree)) {
	        log_warning0("comparison of distinct pointer types lacks a"
	                                                              " cast\n")
	        longjmp(Prs->CheckPoint, 1);
	      }
	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_EQ;
	      E->TypeTree = Ebcscript_Type_Int();
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Eq.Left = $1;
	      E->As.Eq.Right = $3;
	    }

	    $$ = E;
	  }
	| equality_expression NE_OP relational_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* 型が一致しないときは、キャスト変換のノードを挿入する */
	    if (!(Ebcscript_Type_isNumeric($1->TypeTree)
	       && Ebcscript_Type_isNumeric($3->TypeTree)
	       || Ebcscript_Type_isPointer($1->TypeTree)
	       && Ebcscript_Type_isPointer($3->TypeTree) ) ) {
	      log_error0("invalid operands to binary !=\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    if (Ebcscript_Type_isNumeric($1->TypeTree)
	     && Ebcscript_Type_isNumeric($3->TypeTree)) {
	      /* ノードの型を決定する */
	      T1 = $1->TypeTree;
	      T2 = $3->TypeTree;
	      TE = Ebcscript_Type_balance_numeric(T1, T2);

	      /* キャストを挟む */
	      if (!Ebcscript_Type_equals(TE, T1)) {
	        $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	      }
	      if (!Ebcscript_Type_equals(TE, T2)) {
	        $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	      }

	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_NE;
	      E->TypeTree = TE;
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Ne.Left = $1;
	      E->As.Ne.Right = $3;
	    }

	    if (Ebcscript_Type_isPointer($1->TypeTree)
	     && Ebcscript_Type_isPointer($3->TypeTree)) {
	      if (!Ebcscript_Type_equals($1->TypeTree, $3->TypeTree)) {
	        log_warning0("comparison of distinct pointer types lacks a"
	                                                              " cast\n")
	        longjmp(Prs->CheckPoint, 1);
	      }
	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_NE;
	      E->TypeTree = Ebcscript_Type_Int();
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.Ne.Left = $1;
	      E->As.Ne.Right = $3;
	    }

	    $$ = E;
	  }
	;

and_expression
	: equality_expression
	  {
	    $$ = $1;
	  }
	| and_expression '&' equality_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* 型が一致しないときは、キャスト変換のノードを挿入する */
	    if (!Ebcscript_Type_isInteger($1->TypeTree)
	     || !Ebcscript_Type_isInteger($3->TypeTree)) {
	      log_error0("invalid operands to binary &\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* ノードの型を決定する */
	    T1 = $1->TypeTree;
	    T2 = $3->TypeTree;
	    TE = Ebcscript_Type_balance_integer(T1, T2);

	    /* キャストを挟む */
	    if (!Ebcscript_Type_equals(TE, T1)) {
	      $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	    }
	    if (!Ebcscript_Type_equals(TE, T2)) {
	      $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_AND;
	    E->TypeTree = TE;
	    E->IsConstant = $1->IsConstant && $3->IsConstant;
	    E->As.And.Left = $1;
	    E->As.And.Right = $3;
	    $$ = E;
	  }
	;

exclusive_or_expression
	: and_expression
	  {
	    $$ = $1;
	  }
	| exclusive_or_expression '^' and_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* 型が一致しないときは、キャスト変換のノードを挿入する */
	    if (!Ebcscript_Type_isInteger($1->TypeTree)
	     || !Ebcscript_Type_isInteger($3->TypeTree)) {
	      log_error0("invalid operands to binary ^\n");
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* ノードの型を決定する */
	    T1 = $1->TypeTree;
	    T2 = $3->TypeTree;
	    TE = Ebcscript_Type_balance_integer(T1, T2);

	    /* キャストを挟む */
	    if (!Ebcscript_Type_equals(TE, T1)) {
	      $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	    }
	    if (!Ebcscript_Type_equals(TE, T2)) {
	      $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_XOR;
	    E->TypeTree = TE;
	    E->IsConstant = $1->IsConstant && $3->IsConstant;
	    E->As.Xor.Left = $1;
	    E->As.Xor.Right = $3;
	    $$ = E;
	  }
	;

inclusive_or_expression
	: exclusive_or_expression
	  {
	    $$ = $1;
	  }
	| inclusive_or_expression '|' exclusive_or_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* 型が一致しないときは、キャスト変換のノードを挿入する */
	    if (!Ebcscript_Type_isInteger($1->TypeTree)
	     || !Ebcscript_Type_isInteger($3->TypeTree)) {
	      log_error0("invalid operands to binary |\n");
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* ノードの型を決定する */
	    T1 = $1->TypeTree;
	    T2 = $3->TypeTree;
	    TE = Ebcscript_Type_balance_integer(T1, T2);

	    /* キャストを挟む */
	    if (!Ebcscript_Type_equals(TE, T1)) {
	      $1 = Ebcscript_Parser_newExpression_cast(TE, $1);
	    }
	    if (!Ebcscript_Type_equals(TE, T2)) {
	      $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_OR;
	    E->TypeTree = TE;
	    E->IsConstant = $1->IsConstant && $3->IsConstant;
	    E->As.Or.Left = $1;
	    E->As.Or.Right = $3;
	    $$ = E;
	  }
	;

logical_and_expression
	: inclusive_or_expression
	  {
	    $$ = $1;
	  }
	| logical_and_expression AND_OP inclusive_or_expression
	  {
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    if ($1->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_STRUCT
	     || $3->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_STRUCT) { 
	      log_error0("used struct type value where scalar is required\n");
	      longjmp(Prs->CheckPoint, 1);
	    }
	    if ($1->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_UNION
	     || $3->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_UNION) { 
	      log_error0("used union type value where scalar is required\n");
	      longjmp(Prs->CheckPoint, 1);
	    }
	    /* 算術型かポインタ型 */
	    if ( (Ebcscript_Type_isNumeric($1->TypeTree)
	       || Ebcscript_Type_isPointer($1->TypeTree))
	      && (Ebcscript_Type_isNumeric($3->TypeTree)
	       || Ebcscript_Type_isPointer($3->TypeTree)) ) {

	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_AND;
	      E->TypeTree = Ebcscript_Type_Int();
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.LogAnd.Left = $1;
	      E->As.LogAnd.Right = $3;
	    }
	    $$ = E;
	  }
	;

logical_or_expression
	: logical_and_expression
	  {
	    $$ = $1;
	  }
	| logical_or_expression OR_OP logical_and_expression
	  {
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    if ($1->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_STRUCT
	     || $3->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_STRUCT) { 
	      log_error0("used struct type value where scalar is required\n");
	      longjmp(Prs->CheckPoint, 1);
	    }
	    if ($1->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_UNION
	     || $3->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_UNION) { 
	      log_error0("used union type value where scalar is required\n");
	      longjmp(Prs->CheckPoint, 1);
	    }
	    /* 算術型かポインタ型 */
	    if ( (Ebcscript_Type_isNumeric($1->TypeTree)
	       || Ebcscript_Type_isPointer($1->TypeTree))
	      && (Ebcscript_Type_isNumeric($3->TypeTree)
	       || Ebcscript_Type_isPointer($3->TypeTree)) ) {

	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_LOG_OR;
	      E->TypeTree = Ebcscript_Type_Int();
	      E->IsConstant = $1->IsConstant && $3->IsConstant;
	      E->As.LogOr.Left = $1;
	      E->As.LogOr.Right = $3;
	    }
	    $$ = E;
	  }
	;

conditional_expression
	: logical_or_expression
	  {
	    $$ = $1;
	  }
	| logical_or_expression '?' expression ':' conditional_expression
	  {
	    ebcscript_type *TE, *T1, *T2;
	    ebcscript_parser_expression *E;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      Ebcscript_Parser_deleteExpression($5);
	      YYABORT;
	    }

	    /* (1)両方、算術型 → 合わせる */
	    if ( Ebcscript_Type_isNumeric($3->TypeTree)
	      && Ebcscript_Type_isNumeric($5->TypeTree) ) {
	      T1 = $3->TypeTree;
	      T2 = $5->TypeTree;
	      TE = Ebcscript_Type_balance_numeric(T1, T2);

	      if (!Ebcscript_Type_equals(TE, T1)) {
	        $3 = Ebcscript_Parser_newExpression_cast(TE, $3);
	      }
	      if (!Ebcscript_Type_equals(TE, T2)) {
	        $5 = Ebcscript_Parser_newExpression_cast(TE, $5);
	      }

	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_COND;
	      E->TypeTree = TE;
	      E->IsConstant =
	                     $1->IsConstant && $3->IsConstant && $5->IsConstant;
	      E->As.Cond.Left = $1;
	      E->As.Cond.Center = $3;
	      E->As.Cond.Right = $5;
	    }

	    /* (2)両方、void
	          両方、同じ型の構造体
	          両方、同じ型の共用体
	          両方、同じ型へのポインタ */
	    if (($3->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_VOID
	      || $3->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_STRUCT
	      || $3->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_UNION
	      || $3->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_POINTER)
	      && Ebcscript_Type_equals($3->TypeTree, $5->TypeTree)) {
	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_COND;
	      E->TypeTree = $3->TypeTree;
	      E->IsConstant =
	                     $1->IsConstant && $3->IsConstant && $5->IsConstant;
	      E->As.Cond.Left = $1;
	      E->As.Cond.Center = $3;
	      E->As.Cond.Right = $5;
	    }

	    /* (3)ポインタ型, 定数0 → ポインタ型 */
	    if ( Ebcscript_Type_isPointer($3->TypeTree)
	      && Ebcscript_Parser_Expression_isZero($5) ) {
	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_COND;
	      E->TypeTree = $3->TypeTree;
	      E->IsConstant =
	                     $1->IsConstant && $3->IsConstant && $5->IsConstant;
	      E->As.Cond.Left = $1;
	      E->As.Cond.Center = $3;
	      /* キャストを挟む */
	      $5 = Ebcscript_Parser_newExpression_cast($3->TypeTree, $5);
	      E->As.Cond.Right = $5;
	    }
	    if ( Ebcscript_Parser_Expression_isZero($3)
	      && Ebcscript_Type_isPointer($5->TypeTree) ) {
	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_COND;
	      E->TypeTree = $5->TypeTree;
	      E->IsConstant =
	                     $1->IsConstant && $3->IsConstant && $5->IsConstant;
	      E->As.Cond.Left = $1;
	      /* キャストを挟む */
	      $3 = Ebcscript_Parser_newExpression_cast($5->TypeTree, $3);
	      E->As.Cond.Center = $3;
	      E->As.Cond.Right = $5;
	    }

	    /* (4)voidへのポインタ型, ポインタ →　voidへのポインタ型 */
	    if ($3->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_POINTER
	     && $3->TypeTree->As.Pointer.Value->Kind
	                                          == EBCSCRIPT_TYPE_KIND_VOID
	     && Ebcscript_Type_isPointer($5->TypeTree) ) {
	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_COND;
	      E->TypeTree = $3->TypeTree;
	      E->IsConstant =
	                     $1->IsConstant && $3->IsConstant && $5->IsConstant;
	      E->As.Cond.Left = $1;
	      E->As.Cond.Center = $3;
	      E->As.Cond.Right = $5;
	    }
	    if (Ebcscript_Type_isPointer($3->TypeTree)
	     && $5->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_POINTER
	     && $5->TypeTree->As.Pointer.Value->Kind
	                                          == EBCSCRIPT_TYPE_KIND_VOID) {
	      E = Ebcscript_Parser_newExpression();
	      E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_COND;
	      E->TypeTree = $5->TypeTree;
	      E->IsConstant =
	                     $1->IsConstant && $3->IsConstant && $5->IsConstant;
	      E->As.Cond.Left = $1;
	      E->As.Cond.Center = $3;
	      E->As.Cond.Right = $5;
	    }
/*	    else {
	      log_warning0("pointer type mismatch in conditional expression\n")
	    }*/
	    $$ = E;
	  }
	;

assignment_expression
	: conditional_expression
	  {
	    $$ = $1;
	  }
	| unary_expression assignment_operator assignment_expression
	  {
	    ebcscript_parser_expression *E, *E1;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* 左被演算数としては左辺値が要求される。
	       この左辺値は変更可能でなければならない。
	       すなわち、これは配列であってはならず、また不完全な型をもったり、
	       関数であったりしてはいけない。*/
	    if ($1->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_FUNCTION) {
	      log_error0("lvalue required as left operand of assignment\n");
	      longjmp(Prs->CheckPoint, 1);
	    }
	    if ($1->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_ARRAY) {
	      log_error0("assignment to expression with array type\n");
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* (1)両方の被演算数が算術型を持つ。
	          このとき、右被演算数は代入により左側の型に変換される。 */
	    if (Ebcscript_Type_isNumeric($1->TypeTree)
	     && Ebcscript_Type_isNumeric($3->TypeTree)) {
	      /* キャストを挟む */
	      if (!Ebcscript_Type_equals($1->TypeTree, $3->TypeTree)) {
	        $3 = Ebcscript_Parser_newExpression_cast($1->TypeTree, $3);
	      }
	    } else

	    /* (2)両方の被演算数が同じ型の構造体あるいは共用体である。 */
	    if (($1->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_STRUCT
	      || $1->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_UNION)
	      && Ebcscript_Type_equals($1->TypeTree, $3->TypeTree)) {
	      /* 処置不要 */
	    } else

	    /* (3)1つの被演算数がポインタで、他方がvoidへのポインタである。 */
	    if ($1->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_POINTER
	     && $1->TypeTree->As.Pointer.Value->Kind
	                                          == EBCSCRIPT_TYPE_KIND_VOID
	     && $3->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_POINTER ) {
	      /* 処置不要 */
	    } else
	    if ($1->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_POINTER
	     && $3->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_POINTER
	     && $3->TypeTree->As.Pointer.Value->Kind
	                                          == EBCSCRIPT_TYPE_KIND_VOID) {
	      /* 処置不要 */
	    } else

	    /* (4)左被演算数がポインタで、右被演算数が値0をもつ定数である。 */
	    if ( $1->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_POINTER
	      && Ebcscript_Parser_Expression_isZero($3) ) {
	      /* キャストを挟む */
	      $3 = Ebcscript_Parser_newExpression_cast($1->TypeTree, $3);
	    } else

	    /* (5)両被演算数が関数あるいはオブジェクトへのポインタで、
	          このオブジェクトは右被演算数にconstあるいはvolatileが
	          ないこともある点を除けば、その型が同じである。 */
	    if ( $1->TypeTree->Kind == EBCSCRIPT_TYPE_KIND_POINTER
	      && Ebcscript_Type_equals($1->TypeTree, $3->TypeTree) ) {
	    }

	    /* (6)その他 */
	    else {
	      log_error0("incompatible types when assigning\n");
	      longjmp(Prs->CheckPoint, 1);
	    }

	    E = Ebcscript_Parser_newExpression();
	    E->TypeTree = $1->TypeTree;
	    E->IsConstant = $1->IsConstant && $3->IsConstant;
	    switch ($2) {
	      case '=':
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN;
	        E->As.Assign.Left = $1;
	        E->As.Assign.Right = $3;
	        break;
	      case MUL_ASSIGN:
/*	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_MUL;*/
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN;
	        if (!Ebcscript_Type_isNumeric($1->TypeTree)
	         || !Ebcscript_Type_isNumeric($3->TypeTree)) {
	          log_error0("invalid operands to binary *\n");
	          Ebcscript_Parser_deleteExpression(E);
	          longjmp(Prs->CheckPoint, 1);
	        }
	        E1 = Ebcscript_Parser_newExpression();
	        E1->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_MUL;
	        E1->TypeTree = $1->TypeTree;
	        E1->IsConstant = $1->IsConstant && $3->IsConstant;
	        E1->As.Mul.Left = Ebcscript_Parser_Expression_dup($1);
	        E1->As.Mul.Right = $3;

	        E->As.Assign.Left = $1;
	        E->As.Assign.Right = E1;
	        break;
	      case DIV_ASSIGN:
/*	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_DIV;*/
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN;
	        if (!Ebcscript_Type_isNumeric($1->TypeTree)
	         || !Ebcscript_Type_isNumeric($3->TypeTree)) {
	          log_error0("invalid operands to binary /\n");
	          Ebcscript_Parser_deleteExpression(E);
	          longjmp(Prs->CheckPoint, 1);
	        }
	        E1 = Ebcscript_Parser_newExpression();
	        E1->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_DIV;
	        E1->TypeTree = $1->TypeTree;
	        E1->IsConstant = $1->IsConstant && $3->IsConstant;
	        E1->As.Div.Left = Ebcscript_Parser_Expression_dup($1);
	        E1->As.Div.Right = $3;

	        E->As.Assign.Left = $1;
	        E->As.Assign.Right = E1;
	        break;
	      case MOD_ASSIGN:
/*	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_MOD;*/
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN;
	        if (!Ebcscript_Type_isInteger($1->TypeTree)
	         || !Ebcscript_Type_isInteger($3->TypeTree)) {
	          log_error0("invalid operands to binary \%\n");
	          Ebcscript_Parser_deleteExpression(E);
	          longjmp(Prs->CheckPoint, 1);
	        }
	        E1 = Ebcscript_Parser_newExpression();
	        E1->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_MOD;
	        E1->TypeTree = $1->TypeTree;
	        E1->IsConstant = $1->IsConstant && $3->IsConstant;
	        E1->As.Mod.Left = Ebcscript_Parser_Expression_dup($1);
	        E1->As.Mod.Right = $3;

	        E->As.Assign.Left = $1;
	        E->As.Assign.Right = E1;
	        break;
	      case ADD_ASSIGN:
/*	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_ADD;*/
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN;
	        /* 被演算数は算術型かポインタ型。但し、両方ポインタ型は不可。 */
	        if (!( Ebcscript_Type_isNumeric($1->TypeTree)
	            && Ebcscript_Type_isNumeric($3->TypeTree)
	            || Ebcscript_Type_isPointer($1->TypeTree)
	            && Ebcscript_Type_isInteger($3->TypeTree) ) ) {
	          log_error0("invalid operands to binary +\n");
	          Ebcscript_Parser_deleteExpression(E);
	          longjmp(Prs->CheckPoint, 1);
	        }
	        if (Ebcscript_Type_isNumeric($1->TypeTree)
	         && Ebcscript_Type_isNumeric($3->TypeTree)) {
	          E1 = Ebcscript_Parser_newExpression();
	          E1->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ADD;
	          E1->TypeTree = $1->TypeTree;
	          E1->IsConstant = $1->IsConstant && $3->IsConstant;
	          E1->As.Add.Left = Ebcscript_Parser_Expression_dup($1);
	          E1->As.Add.Right = $3;

	          E->As.Assign.Left = $1;
	          E->As.Assign.Right = E1;
	        }
	        if (Ebcscript_Type_isPointer($1->TypeTree)
	         && Ebcscript_Type_isInteger($3->TypeTree)) {
	          E1 = Ebcscript_Parser_newExpression();
	          E1->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ADD;
	          E1->TypeTree = $1->TypeTree;
	          E1->IsConstant = $1->IsConstant && $3->IsConstant;
	          E1->As.Add.Left = Ebcscript_Parser_Expression_dup($1);
	          E1->As.Add.Right = $3;

	          E->As.Assign.Left = $1;
	          E->As.Assign.Right = E1;
	        }
	        break;
	      case SUB_ASSIGN:
/*	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_SUB;*/
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN;
	        /* 被演算数は算術型かポインタ型。但し、両方ポインタ型は不可。 */
	        if (!( Ebcscript_Type_isNumeric($1->TypeTree)
	            && Ebcscript_Type_isNumeric($3->TypeTree)
	            || Ebcscript_Type_isPointer($1->TypeTree)
	            && Ebcscript_Type_isInteger($3->TypeTree) ) ) {
	          log_error0("invalid operands to binary -\n");
	          Ebcscript_Parser_deleteExpression(E);
	          longjmp(Prs->CheckPoint, 1);
	        }
	        if (Ebcscript_Type_isNumeric($1->TypeTree)
	         && Ebcscript_Type_isNumeric($3->TypeTree)) {
	          E1 = Ebcscript_Parser_newExpression();
	          E1->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_SUB;
	          E1->TypeTree = $1->TypeTree;
	          E1->IsConstant = $1->IsConstant && $3->IsConstant;
	          E1->As.Sub.Left = Ebcscript_Parser_Expression_dup($1);
	          E1->As.Sub.Right = $3;

	          E->As.Assign.Left = $1;
	          E->As.Assign.Right = E1;
	        }
	        if (Ebcscript_Type_isPointer($1->TypeTree)
	         && Ebcscript_Type_isInteger($3->TypeTree)) {
	          E1 = Ebcscript_Parser_newExpression();
	          E1->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_SUB;
	          E1->TypeTree = $1->TypeTree;
	          E1->IsConstant = $1->IsConstant && $3->IsConstant;
	          E1->As.Sub.Left = Ebcscript_Parser_Expression_dup($1);
	          E1->As.Sub.Right = $3;

	          E->As.Assign.Left = $1;
	          E->As.Assign.Right = E1;
	        }
	        break;
	      case LEFT_ASSIGN:
/*	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_SHFT_L;*/
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN;
	        if (!Ebcscript_Type_isInteger($1->TypeTree)
	         || !Ebcscript_Type_isInteger($3->TypeTree)) {
	          log_error0("invalid operands to binary <<\n");
	          Ebcscript_Parser_deleteExpression(E);
	          longjmp(Prs->CheckPoint, 1);
	        }
	        E1 = Ebcscript_Parser_newExpression();
	        E1->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_L;
	        E1->TypeTree = $1->TypeTree;
	        E1->IsConstant = $1->IsConstant && $3->IsConstant;
	        E1->As.ShftL.Left = Ebcscript_Parser_Expression_dup($1);
	        E1->As.ShftL.Right = $3;

	        E->As.Assign.Left = $1;
	        E->As.Assign.Right = E1;
	        break;
	      case RIGHT_ASSIGN:
/*	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_SHFT_R;*/
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN;
	        if (!Ebcscript_Type_isInteger($1->TypeTree)
	         || !Ebcscript_Type_isInteger($3->TypeTree)) {
	          log_error0("invalid operands to binary >>\n");
	          Ebcscript_Parser_deleteExpression(E);
	          longjmp(Prs->CheckPoint, 1);
	        }
	        E1 = Ebcscript_Parser_newExpression();
	        E1->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_SHFT_R;
	        E1->TypeTree = $1->TypeTree;
	        E1->IsConstant = $1->IsConstant && $3->IsConstant;
	        E1->As.ShftR.Left = Ebcscript_Parser_Expression_dup($1);
	        E1->As.ShftR.Right = $3;

	        E->As.Assign.Left = $1;
	        E->As.Assign.Right = E1;
	        break;
	      case AND_ASSIGN:
/*	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_AND;*/
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN;
	        if (!Ebcscript_Type_isInteger($1->TypeTree)
	         || !Ebcscript_Type_isInteger($3->TypeTree)) {
	          log_error0("invalid operands to binary &\n");
	          Ebcscript_Parser_deleteExpression(E);
	          longjmp(Prs->CheckPoint, 1);
	        }
	        E1 = Ebcscript_Parser_newExpression();
	        E1->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_AND;
	        E1->TypeTree = $1->TypeTree;
	        E1->IsConstant = $1->IsConstant && $3->IsConstant;
	        E1->As.And.Left = Ebcscript_Parser_Expression_dup($1);
	        E1->As.And.Right = $3;

	        E->As.Assign.Left = $1;
	        E->As.Assign.Right = E1;
	        break;
	      case OR_ASSIGN:
/*	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_OR;*/
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN;
	        if (!Ebcscript_Type_isInteger($1->TypeTree)
	         || !Ebcscript_Type_isInteger($3->TypeTree)) {
	          log_error0("invalid operands to binary |\n");
	          Ebcscript_Parser_deleteExpression(E);
	          longjmp(Prs->CheckPoint, 1);
	        }
	        E1 = Ebcscript_Parser_newExpression();
	        E1->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_OR;
	        E1->TypeTree = $1->TypeTree;
	        E1->IsConstant = $1->IsConstant && $3->IsConstant;
	        E1->As.Or.Left = Ebcscript_Parser_Expression_dup($1);
	        E1->As.Or.Right = $3;

	        E->As.Assign.Left = $1;
	        E->As.Assign.Right = E1;
	        break;
	      case XOR_ASSIGN:
/*	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN_XOR;*/
	        E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_ASSGN;
	        if (!Ebcscript_Type_isInteger($1->TypeTree)
	         || !Ebcscript_Type_isInteger($3->TypeTree)) {
	          log_error0("invalid operands to binary ^\n");
	          Ebcscript_Parser_deleteExpression(E);
	          longjmp(Prs->CheckPoint, 1);
	        }
	        E1 = Ebcscript_Parser_newExpression();
	        E1->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_XOR;
	        E1->TypeTree = $1->TypeTree;
	        E1->IsConstant = $1->IsConstant && $3->IsConstant;
	        E1->As.Xor.Left = Ebcscript_Parser_Expression_dup($1);
	        E1->As.Xor.Right = $3;

	        E->As.Assign.Left = $1;
	        E->As.Assign.Right = E1;
	        break;
	    }
	    $$ = E;
	  }
	;

assignment_operator
	: '='
	  {
	    $$ = '=';
	  }
	| MUL_ASSIGN
	  {
	    $$ = MUL_ASSIGN;
	  }
	| DIV_ASSIGN
	  {
	    $$ = DIV_ASSIGN;
	  }
	| MOD_ASSIGN
	  {
	    $$ = MOD_ASSIGN;
	  }
	| ADD_ASSIGN
	  {
	    $$ = ADD_ASSIGN;
	  }
	| SUB_ASSIGN
	  {
	    $$ = SUB_ASSIGN;
	  }
	| LEFT_ASSIGN
	  {
	    $$ = LEFT_ASSIGN;
	  }
	| RIGHT_ASSIGN
	  {
	    $$ = RIGHT_ASSIGN;
	  }
	| AND_ASSIGN
	  {
	    $$ = AND_ASSIGN;
	  }
	| XOR_ASSIGN
	  {
	    $$ = XOR_ASSIGN;
	  }
	| OR_ASSIGN
	  {
	    $$ = OR_ASSIGN;
	  }
	;

expression
	: assignment_expression
	  {
	    $$ = $1;
	  }
	| expression ',' assignment_expression
	  {
	    ebcscript_parser_expression *E;

	    /* 左から右に評価され、左の式の値は捨てられる。
	       結果の型と値は、右の被演算数の型と値である。 */
	    E = Ebcscript_Parser_newExpression();
	    E->Kind = EBCSCRIPT_PARSER_EXPRESSION_KIND_COMMA;
	    E->TypeTree = $3->TypeTree;
	    E->IsConstant = $1->IsConstant && $3->IsConstant;
	    E->As.Comma.Left = $1;
	    E->As.Comma.Right = $3;
	    $$ = E;
	  }
	;

constant_expression
	: conditional_expression
	  {
	    $$ = $1;
	  }
	;

/*-------------------------------------------------------------- Declarations */
declaration
	: declaration_specifiers ';'
	  {
	    /* 構造体、共用体及び列挙体のタグ名の宣言のみ */
	    /* StorageClassにより場合分け */
	    if ($1->StorageClass ==
	                       EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_TYPEDEF) {
	        log_warning0("useless storage class specifier "
	                                               "in empty declaration\n")
	    }
	    if ($1->StorageClass ==
	                        EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_STATIC) {
	        log_warning0("useless storage class specifier "
	                                               "in empty declaration\n")
	    }
	    if ($1->StorageClass ==
	                        EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_EXTERN) {
	        log_warning0("useless storage class specifier "
	                                               "in empty declaration\n")
	    }
	    if ($1->StorageClass == 0) {
	      if ($1->Type == NULL) {
	        log_warning0("useless type name in empty declaration\n")
	      } else {
	        /* 構造体、共用体及び列挙体の宣言 */
	        /* ここですべき処理はない。*/
	      }
	    }
	    Ebcscript_Parser_deleteDeclaration($1);
	  }
	| declaration_specifiers init_declarator_list ';'
	  {
	    btree *P, *Q;
	    ebcscript_type *T;
	    ebcscript_name *N, **N0;
	    size_t Size;
	    ebcscript_parser_blockstack *BS;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteDeclaration($1);
	      for (P = $2; P != NULL; P = P->Right) {
	        Ebcscript_Parser_deleteInitializer(P->Left);
	        Ebcscript_deleteName(P->Datum);
	      }
	      BTree_clear($2, &BTree_clearDummy);
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT
	       || BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK) {
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_debug0("")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* 名前の種類がここで確定する。 */

	    /* $<Declaration>1の情報から型表現木を作る */
	    T = Ebcscript_Parser_Declaration_toTypeTree($1);

	    /* StorageClassにより場合分け */
	    if ($1->StorageClass ==
	                       EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_TYPEDEF) {
	      /* リストの全要素<Name>に対して */
	      for (P = $2; P != NULL; P = P->Right) {
	        N = (ebcscript_name *)P->Datum;

	        /* 初期化子がついているならエラー */
	        if (P->Left != NULL) {
	          log_error1("typedef \'%s\' is initialized\n", N->Identifier)
	          longjmp(Prs->CheckPoint, 1);
	        }

	        /* 型を適用 */
	        Ebcscript_Name_applyType(N, Ebcscript_Type_dup(T));

	        /* 名前の種類の確定 */
	        N->Kind = EBCSCRIPT_NAME_KIND_TYPEDEF;
	        {
	          ebcscript_name Tmp;
	          Tmp.As.Variable.TypeTree = N->As.Variable.TypeTree;
	          N->As.Typedef.TypeTree = Tmp.As.Variable.TypeTree;
	        }

	        /* 名前の重複チェック */
	        if (N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(
	                                                 BS, N->Identifier)) {
	          Ebcscript_Parser_checkRepetation_varfunc(Prs, *N0, N,
	                                                             Prs->Nest);
	          N = *N0;
	        } else {
	          /* 記号表へ登録 */
	          Ebcscript_Parser_Blockstack_addVarFuncTypeEnum(BS, N);
	        }
	      }
	    }

	    if ($1->StorageClass == 0) {
	      /* リストの全要素<Name>に対して */
	      for (P = $2; P != NULL; P = P->Right) {
	        N = (ebcscript_name *)P->Datum;

	        /* 型を適用 */
	        Ebcscript_Name_applyType(N, Ebcscript_Type_dup(T));

	        /* 名前の種類の確定 */
	        Ebcscript_Name_fixKind(N);

	        /* 名前の種類ごとの処理 */
	        if (N->Kind == EBCSCRIPT_NAME_KIND_VARIABLE) {

	          /* 初期化子をなぞって未確定配列サイズを確定。キャストの挿入 */
	          Ebcscript_Parser_walkInitializerAlongType(Prs, P->Left,
	                                               N->As.Variable.TypeTree);

	          /* メモリ割付け */
	          Size = Ebcscript_Type_getSize(N->As.Variable.TypeTree);
	          if (Prs->Nest == 0) {
	            /* 関数の外側で宣言されたオブジェクトは外部リンケージをもつ
	               静的なものとして扱われる。*/
	            N->As.Variable.Linkage = EBCSCRIPT_NAME_LINKAGE_EXTERNAL;
	            N->As.Variable.Address =
	                              Ebcscript_Trnsunit_mallocG(Prs->TU, Size);
	            N->As.Variable.Addressing =
	                                     EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE;
	            N->As.Variable.Nest = Prs->Nest;
	          }
	          if (Prs->Nest > 0) {
	            /* あるブロック内で識別子の宣言にextern指定子が
	               含まれていなければ、その識別子はリンケージを持たず、
	               その関数にユニークである。*/
	            /* 関数の中で宣言されたオブジェクトはautoになる */
	            N->As.Variable.Linkage = EBCSCRIPT_NAME_LINKAGE_NOLINK;
	            N->As.Variable.Address =
	                        Ebcscript_Parser_Blockstack_mallocL(BS, Size);
	            N->As.Variable.Addressing =
	                                 EBCSCRIPT_NAME_ADDRESSING_ONSTACKFRAME;
	            N->As.Variable.Nest = Prs->Nest;
	          }

	          /* 初期化子の処理 */
	          if (Prs->Nest == 0) {
	            /* 初期化子を評価して代入 */
	            Ebcscript_Parser_eval_initializer(Prs, P->Left,
	                       N->As.Variable.TypeTree, N->As.Variable.Address);
	          }
	          if (Prs->Nest > 0) {
	            /* 初期化子のリテラル化、代入コード生成 */
	            Ebcscript_Parser_gencode_initializer(Prs, P->Left,
	                       N->As.Variable.TypeTree, N->As.Variable.Address);
	          }
	        }
	        if (N->Kind == EBCSCRIPT_NAME_KIND_FUNCTION) {
	          /* 初期化子がついているならエラー */
	          if (P->Left != NULL) {
	            log_error1("function \'%s\' is initialized like \n",
	                                                         N->Identifier);
	            longjmp(Prs->CheckPoint, 1);
	          }

	          /* 関数の中で宣言された関数はexternになる；
	             関数の外側で宣言された関数は外部リンケージをもつ
	             静的なものとして扱われる。*/
	          N->As.Function.Linkage = EBCSCRIPT_NAME_LINKAGE_EXTERNAL;
	          N->As.Function.CodeAddress = NULL;
	          N->As.Function.Addressing =
	                                    EBCSCRIPT_NAME_ADDRESSING_UNDEFINED;
	          N->As.Function.FunctionID = NULL;
	        }

	        /* 名前の重複チェック */
	        if (N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(
	                                                 BS, N->Identifier)) {
	          Ebcscript_Parser_checkRepetation_varfunc(Prs, *N0, N,
	                                                             Prs->Nest);
	          /* チェック通過時、Nは削除されている */
	          N = *N0;
	        } else {
	          /* 記号表へ登録 */
	          Ebcscript_Parser_Blockstack_addVarFuncTypeEnum(BS, N);
	        }
	      }
	    }

	    if ($1->StorageClass ==
	                        EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_STATIC) {
	      for (P = $2; P != NULL; P = P->Right) {
	        N = (ebcscript_name *)P->Datum;

	        /* 型を適用 */
	        Ebcscript_Name_applyType(N, Ebcscript_Type_dup(T));

	        /* 名前の種類の確定 */
	        Ebcscript_Name_fixKind(N);

	        /* 名前の種類ごとの処理 */
	        if (N->Kind == EBCSCRIPT_NAME_KIND_VARIABLE) {

	          /* 初期化子をなぞって未確定配列サイズを確定 */
	          Ebcscript_Parser_walkInitializerAlongType(Prs, P->Left,
	                                               N->As.Variable.TypeTree);

	          /* メモリ割付け */
	          /* static指定子は宣言されたオブジェクトに静的な
	             記憶クラスを与えるためのもので、関数の内側でも外側でも
	             使ってよい。 */
	          Size = Ebcscript_Type_getSize(N->As.Variable.TypeTree);
	          if (Prs->Nest == 0) {
	            N->As.Variable.Linkage = EBCSCRIPT_NAME_LINKAGE_INTERNAL;
	            N->As.Variable.Address =
	                              Ebcscript_Trnsunit_mallocG(Prs->TU, Size);
	            N->As.Variable.Addressing =
	                                     EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE;
	            N->As.Variable.Nest = Prs->Nest;
	          }
	          if (Prs->Nest > 0) {
	            /* あるブロック内で識別子の宣言にextern指定子が
	               含まれていなければ、その識別子はリンケージを持たず、
	               その関数にユニークである。*/
	            N->As.Variable.Linkage = EBCSCRIPT_NAME_LINKAGE_NOLINK;
	            N->As.Variable.Address = /* LではなくG */
	                              Ebcscript_Trnsunit_mallocG(Prs->TU, Size);
	            N->As.Variable.Addressing =
	                                     EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE;
	            N->As.Variable.Nest = Prs->Nest;	/* 0にしてもよい。 */
	          }

	          /* 初期化子の処理 */
	          if (Prs->Nest == 0) {
	            /* 初期化子を評価して代入 */
	            Ebcscript_Parser_eval_initializer(Prs, P->Left,
	                       N->As.Variable.TypeTree, N->As.Variable.Address);
	          }
	          if (Prs->Nest > 0) {
	            /* 初期化子を評価して代入 *//* gencodeではなくeval */
	            Ebcscript_Parser_eval_initializer(Prs, P->Left,
	                       N->As.Variable.TypeTree, N->As.Variable.Address);
	          }
	        }
	        if (N->Kind == EBCSCRIPT_NAME_KIND_FUNCTION) {
	          /* 初期化子がついているならエラー */
	          if (P->Left != NULL) {
	            log_error1("function \'%s\' is initialized like \n",
	                                                          N->Identifier)
	            longjmp(Prs->CheckPoint, 1);
	          }

	          if (Prs->Nest == 0) {
	            N->As.Function.Linkage = EBCSCRIPT_NAME_LINKAGE_INTERNAL;
	            N->As.Function.CodeAddress = NULL;
	            N->As.Function.Addressing =
	                                    EBCSCRIPT_NAME_ADDRESSING_UNDEFINED;
	            N->As.Function.FunctionID = NULL;
	          }
	          if (Prs->Nest > 0) {
	            log_error1("invalid storage class for function \'%s\'\n",
	                                                          N->Identifier)
	            longjmp(Prs->CheckPoint, 1);
	          }
	        }

	        /* 名前の重複チェック */
	        if (N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(
	                                                 BS, N->Identifier)) {
	          Ebcscript_Parser_checkRepetation_varfunc(Prs, *N0, N,
	                                                             Prs->Nest);
	          N = *N0;
	        } else {
	          /* 記号表へ登録 */
	          Ebcscript_Parser_Blockstack_addVarFuncTypeEnum(BS, N);
	        }
	      }
	    }

	    if ($1->StorageClass ==
	                        EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_EXTERN) {
	      for (P = $2; P != NULL; P = P->Right) {
	        N = (ebcscript_name *)P->Datum;

	        /* 初期化子がついているならエラー */
	        if (P->Left != NULL) {
	          log_warning1("\'%s\' initialized and declared \'extern\'\n",
	                                                          N->Identifier)
	          longjmp(Prs->CheckPoint, 1);
	        }

	        /* 型を適用 */
	        Ebcscript_Name_applyType(N, Ebcscript_Type_dup(T));

	        /* 名前の種類の確定 */
	        Ebcscript_Name_fixKind(N);

	        /* 名前の種類ごとの処理 */
	        if (N->Kind == EBCSCRIPT_NAME_KIND_VARIABLE) {
	          if (Prs->Nest == 0) {
	            N->As.Variable.Linkage = EBCSCRIPT_NAME_LINKAGE_EXTERNAL;
	            N->As.Variable.Address = NULL;
	            N->As.Variable.Addressing =
	                                    EBCSCRIPT_NAME_ADDRESSING_UNDEFINED;
	            N->As.Variable.Nest = Prs->Nest;
	          }
	          if (Prs->Nest > 0) {
	            /* 関数の中で使われるextern付きの宣言は、
	               宣言されたオブジェクト用のメモリがどこか他で
	               定義されていることを表す。 */
	            /* あるブロック内で識別子の宣言にexternが含まれていて、
	               その識別子に対する外部宣言がそのブロックを囲む通用範囲で
	               アクティブであれば、その識別子はその外部宣言と同じ
	               リンケージを持ち、同じオブジェクトあるいは関数を持つ。
	               しかし目に見える外部宣言がなければ、そのリンケージは
	               外部的である。 */
	            N->As.Variable.Linkage = EBCSCRIPT_NAME_LINKAGE_NOLINK;
	            N->As.Variable.Address = NULL;
	            N->As.Variable.Addressing =
	                                    EBCSCRIPT_NAME_ADDRESSING_UNDEFINED;
	            N->As.Variable.Nest = Prs->Nest;
	          }
	        }
	        if (N->Kind == EBCSCRIPT_NAME_KIND_FUNCTION) {
	          N->As.Function.Linkage = EBCSCRIPT_NAME_LINKAGE_EXTERNAL;
	          N->As.Function.CodeAddress = NULL;
	          N->As.Function.Addressing =
	                                    EBCSCRIPT_NAME_ADDRESSING_UNDEFINED;
	          N->As.Function.FunctionID = NULL;
	        }

	        /* 名前の重複チェック */
	        if (N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(
	                                                 BS, N->Identifier)) {
	          Ebcscript_Parser_checkRepetation_varfunc(Prs, *N0, N,
	                                                             Prs->Nest);
	          N = *N0;
	        } else {
	          /* 記号表へ登録 */
	          Ebcscript_Parser_Blockstack_addVarFuncTypeEnum(BS, N);
	        }
	      }
	    }

	    Ebcscript_Parser_deleteDeclaration($1);
	    for (P = $2; P != NULL; P = P->Right) {
	      Ebcscript_Parser_deleteInitializer(P->Left);
	    }
	    BTree_clear($2, &BTree_clearDummy);
	  }
	;

declaration_specifiers
	: storage_class_specifier
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->StorageClass = $1;
	  }
	| storage_class_specifier declaration_specifiers
	  {
	    if ($2->StorageClass != 0) {
	      log_error0("multiple storage classes in declaration specifiers")
	      Ebcscript_Parser_deleteDeclaration($2);
	      YYABORT;
	    } else {
	     $2->StorageClass = $1;
	    }
	    $$ = $2;
	  }
	| type_specifier
	  {
	    $$ = $1;
	  }
	| type_specifier declaration_specifiers
	  {
	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteDeclaration($1);
	      Ebcscript_Parser_deleteDeclaration($2);
	      YYABORT;
	    }

	    /* <Declaration>の統合とチェック */
	    Ebcscript_Parser_mergeDeclaration(Prs, $1, $2);

	    Ebcscript_Parser_deleteDeclaration($1);
	    $$ = $2;
	  }
	| type_qualifier
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->IsConst    |= ($1 == CONST)    ? true : false;
	    $$->IsVolatile |= ($1 == VOLATILE) ? true : false;
	  }
	| type_qualifier declaration_specifiers
	  {
	    $2->IsConst    |= ($1 == CONST)    ? true : false;
	    $2->IsVolatile |= ($1 == VOLATILE) ? true : false;
	    $$ = $2;
	  }
	;

init_declarator_list
	: init_declarator
	  {
	    $$ = $1;
	  }
	| init_declarator_list ',' init_declarator
	  {
	    btree *P;

	    /* $1の末尾に$3を追加する。 */
	    for (P = $1; P->Right != NULL; P = P->Right)
	      ;
	    P->Right = $3;
	    $$ = $1;
	  }
	;

init_declarator
	: declarator
	  {
	    $$ = newBTree($1, NULL, NULL);
	  }
	| declarator '=' initializer
	  {
	    $$ = newBTree($1, $3, NULL);
	  }
	;

storage_class_specifier
	: TYPEDEF
	  {
	    $$ = EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_TYPEDEF;
	  }
	| EXTERN
	  {
	    $$ = EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_EXTERN;
	  }
	| STATIC
	  {
	    $$ = EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_STATIC;
	  }
	;

type_specifier
	: VOID
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->PrimitiveType = EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_VOID;
	  }
	| CHAR
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->PrimitiveType = EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_CHAR;
	  }
	| SHORT
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->ShortOrLong = EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_SHORT;
	  }
	| INT
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->PrimitiveType = EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_INT;
	  }
	| LONG
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->ShortOrLong = EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_LONG;
	  }
	| FLOAT
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->PrimitiveType = EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_FLOAT;
	  }
	| DOUBLE
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->PrimitiveType = EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_DOUBLE;
	  }
	| SIGNED
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->SignedOrUnsigned =
	                          EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_SIGNED;
	  }
	| UNSIGNED
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->SignedOrUnsigned =
	                        EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_UNSIGNED;
	  }
	| struct_or_union_specifier
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->Type = $1;
	  }
	| enum_specifier
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->Type = $1;
	  }
	| typedef_name
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->Type = $1;
	  }
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}'
	  {
	    btree *P;
	    ebcscript_type *T;
	    ebcscript_name **N0, *N, *NMember;
	    ebcscript_parser_blockstack *BS;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Name_free($2);
	      BTree_clear($4, (void (*)(void *))Ebcscript_deleteName);
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK
	       || BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT) {
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_debug0("")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* 型表現木のみが上へ。タグ名はここより上には上がらない。 */
	    if ($1 == STRUCT) {
	      /* 不完全型としてタグ名を作成 */
	      T = Ebcscript_newType_struct();
	      N = Ebcscript_newName_struct($2);
	      N->As.Struct.TypeTree = T;

	      /* 名前の重複チェック */
	      if (N0 = Ebcscript_Parser_Blockstack_findTag(BS, N->Identifier)) {
	        Ebcscript_Parser_checkRepetation_tag(Prs, *N0, N);
	        N = *N0;
	      } else {
	        /* 記号表へ登録 */
	        Ebcscript_Parser_Blockstack_addTag(BS, N);
	      }

	      /* Tにメンバリスト$4を追加。サイズの決定 */
	      T = N->As.Struct.TypeTree;
	      for (P = $4; P != NULL; P = P->Right) {
	        NMember = (ebcscript_name *)P->Datum;
	        if (!Ebcscript_Type_Struct_addMember(&T->As.Struct, NMember)) {
	          log_error1("duplicate member \'%s\'\n", NMember->Identifier)
	          SList_clear(T->As.Struct.Members, &SList_clearDummy);
	          longjmp(Prs->CheckPoint, 1);
	        }
	      }
	      T->As.Struct.Size += Ebcscript_Name_align(
	                                 T->As.Struct.Size, T->As.Struct.Align);
	    }
	    if ($1 == UNION) {
	      /* 不完全型としてタグ名を作成 */
	      T = Ebcscript_newType_union();
	      N = Ebcscript_newName_union($2);
	      N->As.Union.TypeTree = T;

	      /* 名前の重複チェック */
	      if (N0 = Ebcscript_Parser_Blockstack_findTag(BS, $2)) {
	        Ebcscript_Parser_checkRepetation_tag(Prs, *N0, N);
	        N = *N0;
	      } else {
	        /* 記号表へ登録 */
	        Ebcscript_Parser_Blockstack_addTag(BS, N);
	      }

	      /* Tにメンバリスト$4を追加。サイズの決定 */
	      T = N->As.Union.TypeTree;
	      for (P = $4; P != NULL; P = P->Right) {
	        NMember = (ebcscript_name *)P->Datum;
	        if (!Ebcscript_Type_Union_addMember(&T->As.Union, NMember)) {
	          log_error1("duplicate member \'%s\'\n", NMember->Identifier)
	          SList_clear(T->As.Struct.Members, &SList_clearDummy);
	          longjmp(Prs->CheckPoint, 1);
	        }
	      }
	    }
	    BTree_clear($4, &BTree_clearDummy);	/* Datumは削除しない。 */
	    Ebcscript_Name_free($2);
	    $$ = T;
	  }
	| struct_or_union '{' struct_declaration_list '}'
	  {
	    btree *P;
	    ebcscript_type *T;
	    ebcscript_name *N, *NMember;
	    char StrTag[11] = "$";
	    ebcscript_parser_blockstack *BS;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      BTree_clear($3, (void (*)(void *))Ebcscript_deleteName);
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK
	       || BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT) {
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_debug0("")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* 仮のタグ名で記号表に登録しておく。削除時に使う。 */
	    if ($1 == STRUCT) {
	      /* 不完全型としてタグ名を作成 */
	      sprintf(StrTag, "$%d", Prs->AnonymousNum++);
	      T = Ebcscript_newType_struct();
	      N = Ebcscript_newName_struct(StrTag);
	      N->As.Struct.TypeTree = T;

	      /* 記号表へ登録 */
	      Ebcscript_Parser_Blockstack_addTag(BS, N);

	      /* Tにメンバリスト$3を設定して完成させる。 */
	      for (P = $3; P != NULL; P = P->Right) {
	        NMember = (ebcscript_name *)P->Datum;
	        if (!Ebcscript_Type_Struct_addMember(&T->As.Struct, NMember)) {
	          log_error1("duplicate member \'%s\'\n", NMember->Identifier)
	          SList_clear(T->As.Struct.Members, &SList_clearDummy);
	          longjmp(Prs->CheckPoint, 1);
	        }
	      }
	      T->As.Struct.Size += Ebcscript_Name_align(
	                             T->As.Struct.Size, T->As.Struct.Align);
	    }
	    if ($1 == UNION) {
	      /* 不完全型としてタグ名を作成 */
	      sprintf(StrTag, "$%d", Prs->AnonymousNum++);
	      T = Ebcscript_newType_union();
	      N = Ebcscript_newName_union(StrTag);
	      N->As.Union.TypeTree = T;

	      /* 記号表へ登録 */
	      Ebcscript_Parser_Blockstack_addTag(BS, N);

	      /* Tにメンバリスト$3を設定して完成させる。 */
	      for (P = $3; P != NULL; P = P->Right) {
	        NMember = (ebcscript_name *)P->Datum;
	        if (!Ebcscript_Type_Union_addMember(&T->As.Union, NMember)) {
	          log_error1("duplicate member \'%s\'\n", NMember->Identifier)
	          SList_clear(T->As.Struct.Members, &SList_clearDummy);
	          longjmp(Prs->CheckPoint, 1);
	        }
	      }
	    }
	    BTree_clear($3, &BTree_clearDummy);	/* Datumは削除しない。 */
	    $$ = T;
	  }
	| struct_or_union IDENTIFIER
	  {
	    ebcscript_name **N0, *N;
	    ebcscript_type *T;
	    ebcscript_parser_blockstack *BS;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Name_free($2);
	      YYABORT;
	    }

	    /* 記号表になければ不完全型の構造体・共用体として登録
	       不完全型であるかはisEmpty(Members)で表現する。 */
	    if ($1 == STRUCT) {
	      for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	        if (N0 = Ebcscript_Parser_Blockstack_findTag(BS, $2)) {
	          N = *N0;
	          break;
	        }
	      }
	      if (BS == NULL) {
	        for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	          if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK
	           || BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT) {
	            break;
	          }
	        }
	        if (BS == NULL) {
	          log_debug0("")
	          longjmp(Prs->CheckPoint, 1);
	        }

	        T = Ebcscript_newType_struct();
	        N = Ebcscript_newName_struct($2);
	        N->As.Struct.TypeTree = T;
	        Ebcscript_Parser_Blockstack_addTag(BS, N);
	      }

	      switch (N->Kind) {
	        case EBCSCRIPT_NAME_KIND_STRUCT:
	          T = N->As.Struct.TypeTree;
	          break;
	        default:
	          log_error1("\'%s\' defined as wrong kind of tag\n",
	                                                          N->Identifier)
	          longjmp(Prs->CheckPoint, 1);
	          break;
	      }
	    }
	    if ($1 == UNION) {
	      for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	        if (N0 = Ebcscript_Parser_Blockstack_findTag(BS, $2)) {
	          N = *N0;
	          break;
	        }
	      }
	      if (BS == NULL) {
	        for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	          if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK
	           || BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT) {
	            break;
	          }
	        }
	        if (BS == NULL) {
	          log_debug0("")
	          longjmp(Prs->CheckPoint, 1);
	        }

	        T = Ebcscript_newType_union();
	        N = Ebcscript_newName_union($2);
	        N->As.Union.TypeTree = T;
	        Ebcscript_Parser_Blockstack_addTag(BS, N);
	      }

	      switch (N->Kind) {
	        case EBCSCRIPT_NAME_KIND_UNION:
	          T = N->As.Union.TypeTree;
	          break;
	        default:
	          log_error1("\'%s\' defined as wrong kind of tag\n",
	                                                          N->Identifier)
	          longjmp(Prs->CheckPoint, 1);
	          break;
	      }
	    }
	    Ebcscript_Name_free($2);
	    $$ = T;
	  }
	;

struct_or_union
	: STRUCT
	  {
	    $$ = STRUCT;
	  }
	| UNION
	  {
	    $$ = UNION;
	  }
	;

struct_declaration_list
	: struct_declaration
	  {
	    $$ = $1;
	  }
	| struct_declaration_list struct_declaration
	  {
	    btree *P;

	    /* $1の末尾に$2を接ぎ木する。 */
	    for (P = $1; P->Right != NULL; P = P->Right)
	      ;
	    P->Right = $2;
	    $$ = $1;
	  }
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
	  {
	    btree *P;
	    ebcscript_type *T;
	    ebcscript_name *N;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteDeclaration($1);
	      for (P = $2; P != NULL; P = P->Right) {
	        Ebcscript_deleteName(P->Datum);
	      }
	      BTree_clear($2, &BTree_clearDummy);
	      YYABORT;
	    }

	    /* $<Declaration>1の情報から型表現木を作る */
	    T = Ebcscript_Parser_Declaration_toTypeTree($1);

	    /* リストの全要素<Name>に対して型を適用 */
	    for (P = $2; P != NULL; P = P->Right) {
	      N = (ebcscript_name *)P->Datum;
	      Ebcscript_Name_applyType(N, Ebcscript_Type_dup(T));

	      /* 名前の種類の確定 */
	      Ebcscript_Name_fixKind(N);

	      if (N->Kind == EBCSCRIPT_NAME_KIND_FUNCTION) {
	        log_error1("field \'%s\' declared as a function\n",
	                                                         N->Identifier);
	        longjmp(Prs->CheckPoint, 1);
	      }

	      /* 不完全型のチェック */
	      if (N->Kind == EBCSCRIPT_NAME_KIND_VARIABLE) {
	        ebcscript_type *T;

	        T = N->As.Variable.TypeTree;
	        if (T->Kind == EBCSCRIPT_TYPE_KIND_VOID) {
	          log_error1("variable or field \'%s\' declared void\n",
	                                                         N->Identifier);
	          longjmp(Prs->CheckPoint, 1);
	        }

	        if (T->Kind == EBCSCRIPT_TYPE_KIND_STRUCT
	         && SList_isEmpty(T->As.Struct.Members)) {
	          log_error1("field \'%s\' has incomplete type\n",
	                                                         N->Identifier);
	          longjmp(Prs->CheckPoint, 1);
	        }

	        if (T->Kind == EBCSCRIPT_TYPE_KIND_UNION
	         && SList_isEmpty(T->As.Union.Members)) {
	          log_error1("field \'%s\' has incomplete type\n",
	                                                         N->Identifier);
	          longjmp(Prs->CheckPoint, 1);
	        }

	        if (T->Kind == EBCSCRIPT_TYPE_KIND_ARRAY
	         && T->As.Array.Length == -1) {
	          log_error1("field \'%s\' has incomplete type\n",
	                                                         N->Identifier);
	          longjmp(Prs->CheckPoint, 1);
	        }
	      }

	    }
	    /* 加工後のリスト */
	    $$ = $2;

	    Ebcscript_Parser_deleteDeclaration($1);
	  }
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
	  {
	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteDeclaration($1);
	      Ebcscript_Parser_deleteDeclaration($2);
	      YYABORT;
	    }

	    /* declaration_specifiersと全く同じ内容 */
	    /* <Declaration>の統合とチェック */
	    Ebcscript_Parser_mergeDeclaration(Prs, $1, $2);

	    Ebcscript_Parser_deleteDeclaration($1);
	    $$ = $2;
	  }
	| type_specifier
	  {
	    $$ = $1;
	  }
	| type_qualifier specifier_qualifier_list
	  {
	    $2->IsConst    |= ($1 == CONST)    ? true : false;
	    $2->IsVolatile |= ($1 == VOLATILE) ? true : false;
	    $$ = $2;
	  }
	| type_qualifier
	  {
	    $$ = Ebcscript_Parser_newDeclaration();
	    $$->IsConst    |= ($1 == CONST)    ? true : false;
	    $$->IsVolatile |= ($1 == VOLATILE) ? true : false;
	  }
	;

struct_declarator_list
	: struct_declarator
	  {
	    $$ = newBTree($1, NULL, NULL);
	  }
	| struct_declarator_list ',' struct_declarator
	  {
	    btree *P;

	    /* $1の末尾に$3を追加する。 */
	    for (P = $1; P->Right != NULL; P = P->Right)
	      ;
	    P->Right = newBTree($3, NULL, NULL);
	    $$ = $1;
	  }
	;

struct_declarator
	: declarator
	  {
	    $$ = $1;
	  }
	;

enum_specifier
	: ENUM '{'
	  {
	    Prs->EnumNum = 0;
	  }
	  enumerator_list '}'
	  {
	    ebcscript_name *N;
	    ebcscript_type *T;
	    char StrTag[11] = "$";
	    ebcscript_parser_blockstack *BS;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK
	       || BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT) {
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_debug0("")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* 仮のタグ名で記号表に登録しておく。削除時に使う。 */
	    sprintf(StrTag, "$%d", Prs->AnonymousNum++);
	    T = Ebcscript_newType_enumeration();
	    N = Ebcscript_newName_enumeration(StrTag);
	    N->As.Enumeration.TypeTree = T;
	    T->As.Enumeration.IsEmpty = false;
	    Ebcscript_Parser_Blockstack_addTag(BS, N);
	    $$ = T;
	  }
	| ENUM IDENTIFIER '{'
	  {
	    Prs->EnumNum = 0;
	  }
	  enumerator_list '}'
	  {
	    ebcscript_name **N0, *N;
	    ebcscript_type *T;
	    ebcscript_parser_blockstack *BS;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Name_free($2);
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK
	       || BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT) {
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_debug0("")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* 不完全型としてタグ名を作成 */
	    T = Ebcscript_newType_enumeration();
	    N = Ebcscript_newName_enumeration($2);
	    N->As.Enumeration.TypeTree = T;
	    T->As.Enumeration.IsEmpty = false;

	    /* 名前の重複チェック */
	    if (N0 = Ebcscript_Parser_Blockstack_findTag(BS, N->Identifier)) {
	      Ebcscript_Parser_checkRepetation_tag(Prs, *N0, N);
	      N = *N0;
	    } else {
	      /* 記号表へ登録 */
	      Ebcscript_Parser_Blockstack_addTag(BS, N);
	    }
	    T = N->As.Enumeration.TypeTree;
	    Ebcscript_Name_free($2);
	    $$ = T;
	  }
	| ENUM IDENTIFIER
	  {
	    ebcscript_name **N0, *N;
	    ebcscript_type *T;
	    ebcscript_parser_blockstack *BS;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Name_free($2);
	      YYABORT;
	    }

	    /* 登録済みなら型を返す。不完全型である場合も含む */
	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (N0 = Ebcscript_Parser_Blockstack_findTag(BS, $2)) {
	        N = *N0;
	        break;
	      }
	    }
	    if (BS == NULL) {
	      for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	        if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK
	         || BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT) {
	          break;
	        }
	      }
	      if (BS == NULL) {
	        log_debug0("")
	        longjmp(Prs->CheckPoint, 1);
	      }

	      /* 未登録。不完全型として新規に登録 */
	      T = Ebcscript_newType_enumeration();
	      N = Ebcscript_newName_enumeration($2);
	      N->As.Enumeration.TypeTree = T;
	      Ebcscript_Parser_Blockstack_addTag(BS, N);
	      T->As.Enumeration.IsEmpty = true;	/* 不完全型 */
	    }

	    switch (N->Kind) {
	      case EBCSCRIPT_NAME_KIND_ENUMERATION:
	        T = N->As.Enumeration.TypeTree;
	        break;
	      default:
	        log_error1("\'%s\' defined as wrong kind of tag\n",
	                                                          N->Identifier)
	        longjmp(Prs->CheckPoint, 1);
	        break;
	    }
	    Ebcscript_Name_free($2);
	    $$ = T;
	  }
	;

enumerator_list
	: enumerator
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name **N0;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_deleteName($1);
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK
	       || BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT) {
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_debug0("")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* 列挙体ごとに名前空間を持つわけではない。 */
	    /* 名前の重複チェック */
	    if (N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(BS,
	                                                      $1->Identifier)) {
	      Ebcscript_Parser_checkRepetation_varfunc(Prs, *N0, $1, Prs->Nest);
	    } else {
	      /* 記号表へ登録 */
	      Ebcscript_Parser_Blockstack_addVarFuncTypeEnum(BS, $1);
	    }
	    /* 上には何も上げない。 */
	  }
	| enumerator_list ',' enumerator
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name **N0;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_deleteName($3);
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK
	       || BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT) {
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_debug0("")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* 列挙体ごとに名前空間を持つわけではない。 */
	    /* 名前の重複チェック */
	    if (N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(BS,
	                                                      $3->Identifier)) {
	      Ebcscript_Parser_checkRepetation_varfunc(Prs, *N0, $3, Prs->Nest);
	    } else {
	      /* 記号表へ登録 */
	      Ebcscript_Parser_Blockstack_addVarFuncTypeEnum(BS, $3);
	    }
	    /* 上には何も上げない。 */
	  }
	;

enumerator
	: IDENTIFIER
	  {
	    ebcscript_name *N;

	    N = Ebcscript_newName_enumerator($1);
	    N->As.Enumerator.Value = Prs->EnumNum++;
	    N->As.Enumerator.TypeTree = Ebcscript_newType_enumerator();

	    Ebcscript_Name_free($1);
	    $$ = N;
	  }
	| IDENTIFIER '=' constant_expression
	  {
	    int V = 0;
	    ebcscript_type *TI;
	    ebcscript_name *N;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Name_free($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    if (!Ebcscript_Type_isInteger($3->TypeTree)) {
	      log_error1(
	       "enumerator value for \'%s\' is not an integer constant\n",
	       $1);
	      longjmp(Prs->CheckPoint, 1);
	    }
	    if (!$3->IsConstant) {
	      log_error1(
	       "enumerator value for \'%s\' is not an integer constant\n",
	       $1);
	      longjmp(Prs->CheckPoint, 1);
	    }

	    TI = Ebcscript_Type_Int();
	    if (!Ebcscript_Type_equals(TI, $3->TypeTree)) {
	      $3 = Ebcscript_Parser_newExpression_cast(TI, $3);
	    }

	    Ebcscript_Parser_eval_expr_rv(Prs, $3);
	    Ebcscript_Parser_pop_int(Prs, &V);
	    Prs->EnumNum = V;

	    N = Ebcscript_newName_enumerator($1);
	    N->As.Enumerator.Value = Prs->EnumNum++;
	    N->As.Enumerator.TypeTree = Ebcscript_newType_enumerator();

	    Ebcscript_Name_free($1);
	    Ebcscript_Parser_deleteExpression($3);
	    $$ = N;
	  }
	;

type_qualifier
	: CONST
	  {
	    $$ = (int)CONST;
	  }
	| VOLATILE
	  {
	    $$ = (int)VOLATILE;
	  }
	;

declarator
	: pointer direct_declarator
	  {
	    /* "pointer to" */
	    /* $<type>1を$<name>2へ接ぎ木する。
	       $<name>2.Kindは暫定的にVariable。 */
	    Ebcscript_Type_addLast(&($2->As.Variable.TypeTree), $1);
	    $$ = $2;
	  }
	| direct_declarator
	  {
	    $$ = $1;
	  }
	;

direct_declarator
	: IDENTIFIER
	  {
	    ebcscript_name *N;

	    /* IDENTIFIERを仮格納。name.Kindは暫定的にVARIABLEとする。最終的に、
	       (1)変数 VARIABLE
	       (2)関数 FUNCTION
	       (3)Typedef名 TYPEDEF
	       のいずれかになる。*/
	    /* name.As.Variable.TypeTreeは、type.Kindが
	       (1)ポインタ（"pointer to"）
	       (2)関数（"function returning"）
	       (3)配列（"array[] of"）
	       のtypeのリストになる。*/
	    N = Ebcscript_newName_variable($1);
	    Ebcscript_Name_free($1);
	    $$ = N;
	  }
	| '(' declarator ')'
	  {
	    $$ = $2;
	  }
	| direct_declarator '[' constant_expression ']'
	  {
	    int Length;
	    ebcscript_type *TI, *TA;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_deleteName($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* "array[] of" */
	    Ebcscript_Type_addLast(&($1->As.Variable.TypeTree),
	                                        TA = Ebcscript_newType_array());

	    /* 要素数 */
	    if (!Ebcscript_Type_isInteger($3->TypeTree)) {
	      log_error1("size of array \'%s\' has non-integer type\n",
	                                                        $1->Identifier);
	      longjmp(Prs->CheckPoint, 1);
	    }

	    TI = Ebcscript_Type_Int();
	    if (!Ebcscript_Type_equals(TI, $3->TypeTree)) {
	      $3 = Ebcscript_Parser_newExpression_cast(TI, $3);
	    }

	    if ($3->IsConstant) {
	      Ebcscript_Parser_eval_expr_rv(Prs, $3);
	      Ebcscript_Parser_pop_int(Prs, &Length);
	      if (Length <= 0) {
	        log_error1("size of array \'%s\' is negative\n",
	                                                        $1->Identifier);
	        longjmp(Prs->CheckPoint, 1);
	      }
	      TA->As.Array.Length = Length;	/* >=0: 固定サイズ */
	    } else {
	      log_error0("variable-sized object\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    Ebcscript_Parser_deleteExpression($3);
	    $$ = $1;
	  }
	| direct_declarator '[' ']'
	  {
	    ebcscript_type *TA;

	    /* "array[] of" */
	    Ebcscript_Type_addLast(&($1->As.Variable.TypeTree),
	                                        TA = Ebcscript_newType_array());
	    TA->As.Array.Length = -1;		/* -1: 未設定 */
	    $$ = $1;
	  }
	| direct_declarator '(' ')'
	  {
	    /* function returning */
	    Ebcscript_Type_addLast(&($1->As.Variable.TypeTree),
	                                          Ebcscript_newType_function());
	    $$ = $1;
	  }
	| direct_declarator '(' parameter_type_list ')'
	  {
	    ebcscript_type *T;
	    btree *P;

	    T = Ebcscript_newType_function();
	    /* 引数リスト */
	    for (P = $3; P != NULL; P = P->Right) {
	      Ebcscript_Type_Function_addParameter(&T->As.Function, P->Datum);
	    }
	    BTree_clear($3, &BTree_clearDummy);

	    /* function returning */
	    Ebcscript_Type_addLast(&($1->As.Variable.TypeTree), T);
	    $$ = $1;
	  }
	;

pointer
	: '*'
	  {
	    $$ = Ebcscript_newType_pointer();
	  }
	| '*' pointer
	  {
	    /* 繋げる。 */
	    $$ = Ebcscript_newType_pointer();
	    $$->As.Pointer.Value = $2;
	  }
	| '*' type_qualifier_list
	  {
	    /* type_qualifier_listの効果は未実装 */
	    $$ = Ebcscript_newType_pointer();
	  }
	| '*' type_qualifier_list pointer
	  {
	    /* type_qualifier_listの効果は未実装 */
	    /* 繋げる。 */
	    $$ = Ebcscript_newType_pointer();
	    $$->As.Pointer.Value = $3;
	  }
	;

/* 未実装 */
type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	{
	  $2;	/* warningを消すため */
	}
	;

parameter_type_list
	: parameter_list
	  {
	    $$ = $1;
	  }
/*	| parameter_list ',' ELLIPSIS*/
	;

parameter_list
	: parameter_declaration
	  {
	    ebcscript_type *T;

	    /* 関数の場合、ポインタを挟んで変数にする */
	    if ($1->As.Variable.TypeTree->Kind ==
	                                         EBCSCRIPT_TYPE_KIND_FUNCTION) {
	      T = Ebcscript_newType_pointer();
	      T->As.Pointer.Value = $1->As.Variable.TypeTree;
	      $1->As.Variable.TypeTree = T;
	    }
	    Ebcscript_Name_fixKind($1);

	    $$ = newBTree($1, NULL, NULL);
	  }
	| parameter_list ',' parameter_declaration
	  {
	    btree *P;
	    ebcscript_type *T;

	    /* 関数の場合、ポインタを挟んで変数にする */
	    if ($3->As.Variable.TypeTree->Kind ==
	                                         EBCSCRIPT_TYPE_KIND_FUNCTION) {
	      T = Ebcscript_newType_pointer();
	      T->As.Pointer.Value = $3->As.Variable.TypeTree;
	      $3->As.Variable.TypeTree = T;
	    }
	    Ebcscript_Name_fixKind($3);

	    /* $1の末尾に$3を追加する。 */
	    for (P = $1; P->Right != NULL; P = P->Right)
	      ;
	    P->Right = newBTree($3, NULL, NULL);
	    $$ = $1;
	  }
	;

parameter_declaration
	: declaration_specifiers declarator
	  {
	    ebcscript_type *T;

	    /* $<Declaration>1の情報から型表現木を作る */
	    if ($1->StorageClass != 0) {
	      log_error0("storage class specifierd for unnamed parameter\n")
	      Ebcscript_Parser_deleteDeclaration($1);
	      YYABORT;
	    }
	    T = Ebcscript_Parser_Declaration_toTypeTree($1);
	    Ebcscript_Parser_deleteDeclaration($1);

	    /* $<Name>2に型を適用 */
	    Ebcscript_Name_applyType($2, Ebcscript_Type_dup(T));
	    $$ = $2;
	  }
	| declaration_specifiers abstract_declarator
	  {
	    ebcscript_type *T;
	    ebcscript_name *N;
	    char StrParam[11] = "$";

	    /* $<Declaration>1の情報から型表現木を作る */
	    if ($1->StorageClass != 0) {
	      log_error0("storage class specifierd for unnamed parameter\n")
	      Ebcscript_Parser_deleteDeclaration($1);
	      Ebcscript_deleteType($2);
	      YYABORT;
	    }
	    T = Ebcscript_Parser_Declaration_toTypeTree($1);

	    /* $<Type>2に型を適用 */
	    Ebcscript_Type_addLast(&($2), Ebcscript_Type_dup(T));

	    /* 仮の引数名 */
	    sprintf(StrParam, "$%d", Prs->AnonymousNum++);
	    N = Ebcscript_newName_variable(StrParam);
	    N->As.Variable.TypeTree = $2;

	    Ebcscript_Parser_deleteDeclaration($1);
	    $$ = N;
	  }
	| declaration_specifiers
	  {
	    ebcscript_type *T;
	    ebcscript_name *N;
	    char StrParam[11] = "$";

	    /* $<Declaration>1の情報から型表現木を作る */
	    if ($1->StorageClass != 0) {
	      log_error0("storage class specifierd for unnamed parameter\n");
	      Ebcscript_Parser_deleteDeclaration($1);
	      YYABORT;
	    }
	    T = Ebcscript_Parser_Declaration_toTypeTree($1);

	    /* 仮の引数名 */
	    sprintf(StrParam, "$%d", Prs->AnonymousNum++);
	    N = Ebcscript_newName_variable(StrParam);
	    N->As.Variable.TypeTree = Ebcscript_Type_dup(T);

	    Ebcscript_Parser_deleteDeclaration($1);
	    $$ = N;
	  }
	;

type_name
	: specifier_qualifier_list
	  {
	    ebcscript_type *T;

	    T = Ebcscript_Parser_Declaration_toTypeTree($1);
	    Ebcscript_Parser_deleteDeclaration($1);
	    $$ = T;
	  }
	| specifier_qualifier_list abstract_declarator
	  {
	    ebcscript_type *T;

	    T = Ebcscript_Parser_Declaration_toTypeTree($1);
	    Ebcscript_Parser_deleteDeclaration($1);
	    Ebcscript_Type_addLast(&($2), T);
	    $$ = $2;
	 }
	;

abstract_declarator
	: pointer
	  {
	    $$ = $1;
	  }
	| direct_abstract_declarator
	  {
	    $$ = $1;
	  }
	| pointer direct_abstract_declarator
	  {
	    /* "pointer to" */
	    /* $<Type>1を$<Type>2へ接ぎ木する。*/
	    Ebcscript_Type_addLast(&($2), $1);
	    $$ = $2;
	  }
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
	  {
	    $$ = $2;
	  }
	| '[' ']'
	  {
	    /* "array[] of" */
	    $$ = Ebcscript_newType_array();
	    $$->As.Array.Length = -1;		/* -1: 未設定 */
	  }
	| '[' constant_expression ']'
	  {
	    int Length;
	    ebcscript_type *TI, *TA;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($2);
	      YYABORT;
	    }

	    /* "array[] of" */
	    TA = Ebcscript_newType_array();

	    /* 要素数 */
	    if (!Ebcscript_Type_isInteger($2->TypeTree)) {
	      log_error0("size of array has non-integer type\n");
	      longjmp(Prs->CheckPoint, 1);
	    }

	    TI = Ebcscript_Type_Int();
	    if (!Ebcscript_Type_equals(TI, $2->TypeTree)) {
	      $2 = Ebcscript_Parser_newExpression_cast(TI, $2);
	    }

	    if ($2->IsConstant) {
	      Ebcscript_Parser_eval_expr_rv(Prs, $2);
	      Ebcscript_Parser_pop_int(Prs, &Length);
	      if (Length <= 0) {
	        log_error0("size of array is negative\n");
	        longjmp(Prs->CheckPoint, 1);
	      }
	      TA->As.Array.Length = Length;	/* >=0: 固定サイズ */
	    } else {
	      log_error0("variable-sized object\n");
	      longjmp(Prs->CheckPoint, 1);
	    }

	    Ebcscript_Parser_deleteExpression($2);
	    $$ = TA;
	  }
	| direct_abstract_declarator '[' ']'
	  {
	    ebcscript_type *TA;

	    /* "array[] of" */
	    Ebcscript_Type_addLast(&($1), TA = Ebcscript_newType_array());
	    TA->As.Array.Length = -1;		/* -1: 未設定 */
	    $$ = $1;
	  }
	| direct_abstract_declarator '[' constant_expression ']'
	  {
	    int Length;
	    ebcscript_type *TI, *TA;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_deleteType($1);
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    /* "array[] of" */
	    Ebcscript_Type_addLast(&($1), TA = Ebcscript_newType_array());

	    /* 要素数 */
	    if (!Ebcscript_Type_isInteger($3->TypeTree)) {
	      log_error0("size of array has non-integer type\n");
	      longjmp(Prs->CheckPoint, 1);
	    }

	    TI = Ebcscript_Type_Int();
	    if (!Ebcscript_Type_equals(TI, $3->TypeTree)) {
	      $3 = Ebcscript_Parser_newExpression_cast(TI, $3);
	    }

	    if ($3->IsConstant) {
	      Ebcscript_Parser_eval_expr_rv(Prs, $3);
	      Ebcscript_Parser_pop_int(Prs, &Length);
	      if (Length <= 0) {
	        log_error0("size of array is negative\n");
	        longjmp(Prs->CheckPoint, 1);
	      }
	      TA->As.Array.Length = Length;	/* >=0: 固定サイズ */
	    } else {
	      log_error0("variable-sized object\n");
	      longjmp(Prs->CheckPoint, 1);
	    }

	    Ebcscript_Parser_deleteExpression($3);
	    $$ = $1;
	  }
	| '(' ')'
	  {
	    /* function returning */
	    $$ = Ebcscript_newType_function();
	  }
	| '(' parameter_type_list ')'
	  {
	    /* function returning */
	    ebcscript_type *T;
	    btree *P;

	    T = Ebcscript_newType_function();
	    /* 引数リスト */
	    for (P = $2; P != NULL; P = P->Right) {
	      SList_addLast(T->As.Function.Parameters, P->Datum);
	    }
	    $$ = T;
	  }
	| direct_abstract_declarator '(' ')'
	  {
	    /* function returning */
	    Ebcscript_Type_addLast(&($1), Ebcscript_newType_function());
	    $$ = $1;
	  }
	| direct_abstract_declarator '(' parameter_type_list ')'
	  {
	    /* function returning */
	    ebcscript_type *T;
	    btree *P;

	    T = Ebcscript_newType_function();
	    /* 引数リスト */
	    for (P = $3; P != NULL; P = P->Right) {
	      SList_addLast(T->As.Function.Parameters, P->Datum);
	    }
	    Ebcscript_Type_addLast(&($1), T);
	    $$ = $1;
	  }
	;

typedef_name
	: TYPEDEF_NAME
	  {
	    ebcscript_name **N;

	    $$ = $1->As.Typedef.TypeTree;
	  }
	;

initializer
	: assignment_expression
	  {
	    $$ = newBTree($1, NULL, NULL);
	  }
	| '{' initializer_list '}'
	  {
	    $$ = newBTree(NULL, $2, NULL);
	  }
	| '{' initializer_list ',' '}'
	  {
	    $$ = newBTree(NULL, $2, NULL);
	  }
	;

initializer_list
	: initializer
	  {
	    $$ = $1;
	  }
	| initializer_list ',' initializer
	  {
	    btree *P;

	    /* $1の末尾に$3を追加する。 */
	    for (P = $1; P->Right != NULL; P = P->Right)
	      ;
	    P->Right = $3;
	    $$ = $1;
	  }
	;

/*---------------------------------------------------------------- Statements */
statement
	: labeled_statement
	| {
	    ebcscript_parser_blockstack *B;

	    Prs->Nest++;
	    B = Ebcscript_Parser_newBlockstack_block();
	    Ebcscript_Parser_pushBlockstack(Prs, B);

	    /* 入口処理のコード生成 */
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_BP);
	    store_instruction(EBCSCRIPT_INSTRUCTION_MOV_SP_BP);

	    /* フレームの確保 */
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I);
	    SList_addFront(Prs->BS->As.Block.UnresolvedFrameSizeList,
	     Ebcscript_newUnresolved((void *)(Prs->TU->CP - Prs->TU->Code),
	                                                                 NULL));
	    store_int(0);
	    store_instruction(EBCSCRIPT_INSTRUCTION_SUB_SP);
	  }
	  compound_statement
	  {
	    ebcscript_parser_blockstack *B;

	    /* 出口処理のコード生成 */
	    store_instruction(EBCSCRIPT_INSTRUCTION_MOV_BP_SP);
	    store_instruction(EBCSCRIPT_INSTRUCTION_POP_BP);

	    /* フレームサイズのバックパッチ */
	    Prs->BS->As.Block.FrameSize = (int)Prs->BS->As.Block.DP;
	    Ebcscript_Parser_resolve_block(Prs, Prs->BS);

	    Prs->Nest--;
	    B = Ebcscript_Parser_popBlockstack(Prs);
	    Ebcscript_Parser_deleteBlockstack(B);
	  }
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement
	: IDENTIFIER ':'
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name **N0, *N;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Name_free($1);
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION) {
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_debug0("")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    N = Ebcscript_newName_label($1);

	    /* 名前の重複チェック */
	    if (N0 = Ebcscript_Parser_Blockstack_findLabel(BS, N->Identifier)) {
	      /* gotoの前方参照でラベルが仮登録されている場合 */
	      Ebcscript_Parser_checkRepetation_label(Prs, *N0, N);
	      /* チェック通過時、Nは削除されている */
	      N = *N0;
	    } else {
	      /* 記号表へ登録 */
	      Ebcscript_Parser_Blockstack_addLabel(BS, N);
	    }
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;

	    Ebcscript_Name_free($1);
	  }
	  statement
	| CASE constant_expression ':'
	  {
	    ebcscript_parser_blockstack *BS;
	    int V;
	    ebcscript_type *TI;
	    slist_cell *P;
	    ebcscript_name *N;
	    char S[1/*'$'*/ + sizeof(int) * 2 + 1];

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($2);
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_SWITCH) {
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_error0("case label not within a switch statement\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    if (!Ebcscript_Type_isInteger($2->TypeTree) || !$2->IsConstant) {
	      log_error0("case label does not reduce to an integer constant\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    TI = Ebcscript_Type_Int();
	    if (!Ebcscript_Type_equals(TI, $2->TypeTree)) {
	      $2 = Ebcscript_Parser_newExpression_cast(TI, $2);
	    }
	    Ebcscript_Parser_eval_expr_rv(Prs, $2);
	    Ebcscript_Parser_pop_int(Prs, &V);
	    sprintf(S, "$%x", V);

	    /* ラベル値の重複のチェック */
	    for (P = BS->As.Switch.LCase->Head.Next;
	         P != NULL;
	         P = P->Next) {
	      N = (ebcscript_name *)P->Datum;

	      if (!strcmp(N->Identifier, S)) {
	        log_error0("duplicate case value\n")
	        longjmp(Prs->CheckPoint, 1);
	      }
	    }
	    N = Ebcscript_newName_label(S);
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;
	    SList_addLast(BS->As.Switch.LCase, N);

	    Ebcscript_Parser_deleteExpression($2);
	  }
	  statement
	| DEFAULT ':'
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name *N;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_SWITCH) {
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_error0("\'default\'label not within a switch statement\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    N = Prs->BS->As.Switch.LDefault;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;
	  }
	  statement
	;

compound_statement
	: '{' '}'
	| '{' statement_list '}'
	| '{' declaration_list '}'
	| '{' declaration_list statement_list '}'
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

statement_list
	: statement
	| statement_list statement
	;

expression_statement
	: ';'
	| expression ';'
	  {
	    Ebcscript_Parser_gencode_expr_rv(Prs, $1);
	    Ebcscript_Parser_deleteExpression($1);
	    /* スタックに積まれた値を捨てる */
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I);
	    store_int(Ebcscript_Type_getSize($1->TypeTree));
	    store_instruction(EBCSCRIPT_INSTRUCTION_ADD_SP);
	  }
	;

_if_prefix_
	: IF '(' expression ')'
	  {
	    ebcscript_parser_blockstack *BS;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    switch ($3->TypeTree->Kind) {
	      case EBCSCRIPT_TYPE_KIND_VOID:
	        log_error0("void value not ignored as it ought to be\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_STRUCT:
	        log_error0("used struct type value where scalar is required\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_UNION:
	        log_error0("used union type value where scalar is required\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      default:
	        break;
	    }

	    Ebcscript_Parser_gencode_expr_rv(Prs, $3);

	    BS = Ebcscript_Parser_newBlockstack_if();
	    Ebcscript_Parser_pushBlockstack(Prs, BS);

	    gencode_op_numeric_ptr(JMPF, $3->TypeTree)
	    Ebcscript_Parser_store_address_n(
	                                     Prs, Prs->BS, Prs->BS->As.If.LElse);

	    Ebcscript_Parser_deleteExpression($3);
	  }
	;

selection_statement
	: _if_prefix_ statement
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name *N;

	    N = Prs->BS->As.If.LElse;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;

	    Ebcscript_Parser_resolve_if(Prs, Prs->BS);

	    BS = Ebcscript_Parser_popBlockstack(Prs);
	    Ebcscript_Parser_deleteBlockstack(BS);
	  }
	| _if_prefix_ statement ELSE
	  {
	    ebcscript_name *N;

	    store_instruction(EBCSCRIPT_INSTRUCTION_JMP);
	    Ebcscript_Parser_store_address_n(
	                                    Prs, Prs->BS, Prs->BS->As.If.LEndif);

	    N = Prs->BS->As.If.LElse;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;
	  }
	  statement
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name *N;

	    N = Prs->BS->As.If.LEndif;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;

	    Ebcscript_Parser_resolve_if(Prs, Prs->BS);

	    BS = Ebcscript_Parser_popBlockstack(Prs);
	    Ebcscript_Parser_deleteBlockstack(BS);
	  }
	| SWITCH '(' expression ')'
	  {
	    ebcscript_parser_blockstack *BS, *B;
	    ebcscript_name *N;
	    ebcscript_type *TI;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    Prs->Nest++;
	    B = Ebcscript_Parser_newBlockstack_block();
	    Ebcscript_Parser_pushBlockstack(Prs, B);

	    /* ブロック内で選択ジャンプするようにする。 */
	    /* 入口処理のコード生成 */
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_BP);
	    store_instruction(EBCSCRIPT_INSTRUCTION_MOV_SP_BP);

	    /* フレームの確保 */
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I);
	    SList_addFront(Prs->BS->As.Block.UnresolvedFrameSizeList,
	     Ebcscript_newUnresolved((void *)(Prs->TU->CP - Prs->TU->Code),
	                                                               NULL));
	    store_int(0);
	    store_instruction(EBCSCRIPT_INSTRUCTION_SUB_SP);

	    if (!Ebcscript_Type_isInteger($3->TypeTree)) {
	      log_error0("switch quantity not an integer\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    BS = Ebcscript_Parser_newBlockstack_switch();
	    Ebcscript_Parser_pushBlockstack(Prs, BS);

	    /* 前方参照されるラベルは、Nestのみ確定させておく */
	    BS->As.Switch.LBreak->As.Label.Nest = Prs->Nest;

	    /* セレクター保存用ローカル変数 */
	    N = BS->As.Switch.NSelector;
	    N->As.Variable.Linkage = EBCSCRIPT_NAME_LINKAGE_NOLINK;
	    N->As.Variable.Address =
	                    Ebcscript_Parser_Blockstack_mallocL(B, sizeof(int));
	    N->As.Variable.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONSTACKFRAME;
	    N->As.Variable.Nest = Prs->Nest;

	    TI = Ebcscript_Type_Int();
	    if (!Ebcscript_Type_equals(TI, $3->TypeTree)) {
	      $3 = Ebcscript_Parser_newExpression_cast(TI, $3);
	    }

	    Ebcscript_Parser_gencode_expr_rv(Prs, $3);
	    store_instruction(EBCSCRIPT_INSTRUCTION_POP_LMEM_I);
	    store_address(Prs->BS->As.Switch.NSelector->As.Variable.Address);
	    store_instruction(EBCSCRIPT_INSTRUCTION_JMP);
	    Ebcscript_Parser_store_address_n(
	                                Prs, Prs->BS, Prs->BS->As.Switch.LBegin);
	  }
	  compound_statement
	  {
	    ebcscript_parser_blockstack *B, *BS;
	    ebcscript_name *N;
	    int V;
	    slist_cell *P;

	    /* break文が無く、'}'に到達した場合 */
	    store_instruction(EBCSCRIPT_INSTRUCTION_JMP);
	    Ebcscript_Parser_store_address_n(
	                                Prs, Prs->BS, Prs->BS->As.Switch.LBreak);

	    /* ジャンプ先選択 */
	    N = Prs->BS->As.Switch.LBegin;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;

	  /* caseへジャンプ */
	    for (P = Prs->BS->As.Switch.LCase->Head.Next;
	         P != NULL;
	         P = P->Next) {
	      N = (ebcscript_name *)P->Datum;
	      sscanf(N->Identifier, "$%x", &V);
	      store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_LMEM_I);
	      store_address(Prs->BS->As.Switch.NSelector->As.Variable.Address);
	      store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I);
	      store_int(V);
	      store_instruction(EBCSCRIPT_INSTRUCTION_EQ_I);
	      store_instruction(EBCSCRIPT_INSTRUCTION_JMPT_I);
	      store_address((void *)
	             (N->As.Label.CodeAddress - Prs->TU->CP + Prs->TU->Code));
	    }

	    /* defaultへジャンプ */
	    N = Prs->BS->As.Switch.LDefault;
	    if (N->As.Label.CodeAddress != NULL) {
	      store_instruction(EBCSCRIPT_INSTRUCTION_JMP);
	      store_address((void *)
	             (N->As.Label.CodeAddress - Prs->TU->CP + Prs->TU->Code));
	    }

	    /* switchの出口 */
	    N = Prs->BS->As.Switch.LBreak;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;

	    Ebcscript_Parser_resolve_switch(Prs, Prs->BS);

	    BS = Ebcscript_Parser_popBlockstack(Prs);
	    Ebcscript_Parser_deleteBlockstack(BS);

	    /* 出口処理のコード生成 */
	    store_instruction(EBCSCRIPT_INSTRUCTION_MOV_BP_SP);
	    store_instruction(EBCSCRIPT_INSTRUCTION_POP_BP);

	    /* フレームサイズのバックパッチ */
	    Prs->BS->As.Block.FrameSize = (int)Prs->BS->As.Block.DP;
	    Ebcscript_Parser_resolve_block(Prs, Prs->BS);

	    Prs->Nest--;
	    B = Ebcscript_Parser_popBlockstack(Prs);
	    Ebcscript_Parser_deleteBlockstack(B);

	    Ebcscript_Parser_deleteExpression($3);
	  }
	;

_for_prefix_
	: FOR '(' expression_statement ';'
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name *N;

	    BS = Ebcscript_Parser_newBlockstack_loop();
	    Ebcscript_Parser_pushBlockstack(Prs, BS);

	    /* 前方参照されるラベルは、Nestのみ確定させておく */
	    BS->As.Loop.LBreak   ->As.Label.Nest = Prs->Nest;
	    BS->As.Loop.LContinue->As.Label.Nest = Prs->Nest;

	    /* ループの先頭 */
	    N = Prs->BS->As.Loop.LBegin;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;
	  }
	| FOR '(' expression_statement expression ';'
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name *N;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($4);
	      YYABORT;
	    }

	    switch ($4->TypeTree->Kind) {
	      case EBCSCRIPT_TYPE_KIND_VOID:
	        log_error0("void value not ignored as it ought to be\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_STRUCT:
	        log_error0("used struct type value where scalar is required\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_UNION:
	        log_error0("used union type value where scalar is required\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      default:
	        break;
	    }

	    BS = Ebcscript_Parser_newBlockstack_loop();
	    Ebcscript_Parser_pushBlockstack(Prs, BS);

	    /* 前方参照されるラベルは、Nestのみ確定させておく */
	    BS->As.Loop.LBreak   ->As.Label.Nest = Prs->Nest;
	    BS->As.Loop.LContinue->As.Label.Nest = Prs->Nest;

	    /* ループの先頭 */
	    N = Prs->BS->As.Loop.LBegin;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;

	    Ebcscript_Parser_gencode_expr_rv(Prs, $4);
	    gencode_op_numeric_ptr(JMPF, $4->TypeTree)
	    Ebcscript_Parser_store_address_n(
	                                  Prs, Prs->BS, Prs->BS->As.Loop.LBreak);

	    Ebcscript_Parser_deleteExpression($4);
	  }
	;

iteration_statement
	: WHILE '(' expression ')'
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name *N;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($3);
	      YYABORT;
	    }

	    switch ($3->TypeTree->Kind) {
	      case EBCSCRIPT_TYPE_KIND_VOID:
	        log_error0("void value not ignored as it ought to be\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_STRUCT:
	        log_error0("used struct type value where scalar is required\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_UNION:
	        log_error0("used union type value where scalar is required\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      default:
	        break;
	    }

	    BS = Ebcscript_Parser_newBlockstack_loop();
	    Ebcscript_Parser_pushBlockstack(Prs, BS);

	    /* 前方参照されるラベルは、Nestのみ確定させておく */
	    BS->As.Loop.LBreak   ->As.Label.Nest = Prs->Nest;
	    BS->As.Loop.LContinue->As.Label.Nest = Prs->Nest;

	    /* この時点のCPの値を取る */
	    N = Prs->BS->As.Loop.LContinue;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;

	    Ebcscript_Parser_gencode_expr_rv(Prs, $3);
	    gencode_op_numeric_ptr(JMPF, $3->TypeTree)
	    Ebcscript_Parser_store_address_n(
	                                  Prs, Prs->BS, Prs->BS->As.Loop.LBreak);
	  }
	  statement
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name *N;

	    store_instruction(EBCSCRIPT_INSTRUCTION_JMP);
	    Ebcscript_Parser_store_address_n(
	                               Prs, Prs->BS, Prs->BS->As.Loop.LContinue);

	    N = Prs->BS->As.Loop.LBreak;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;

	    Ebcscript_Parser_resolve_loop(Prs, Prs->BS);

	    BS = Ebcscript_Parser_popBlockstack(Prs);
	    Ebcscript_Parser_deleteBlockstack(BS);

	    Ebcscript_Parser_deleteExpression($3);
	  }
	| DO 
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name *N;

	    BS = Ebcscript_Parser_newBlockstack_loop();
	    Ebcscript_Parser_pushBlockstack(Prs, BS);

	    /* 前方参照されるラベルは、Nestのみ確定させておく */
	    BS->As.Loop.LBreak   ->As.Label.Nest = Prs->Nest;
	    BS->As.Loop.LContinue->As.Label.Nest = Prs->Nest;

	    /* ループの先頭 */
	    N = Prs->BS->As.Loop.LContinue;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;
	  }
	  statement WHILE '(' expression ')' ';'
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name *N;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($6);
	      YYABORT;
	    }

	    switch ($6->TypeTree->Kind) {
	      case EBCSCRIPT_TYPE_KIND_VOID:
	        log_error0("void value not ignored as it ought to be\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_STRUCT:
	        log_error0("used struct type value where scalar is required\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      case EBCSCRIPT_TYPE_KIND_UNION:
	        log_error0("used union type value where scalar is required\n")
	        longjmp(Prs->CheckPoint, 1);
	        break;
	      default:
	        break;
	    }

	    Ebcscript_Parser_gencode_expr_rv(Prs, $6);
	    gencode_op_numeric_ptr(JMPT, $6->TypeTree)
	    Ebcscript_Parser_store_address_n(
	                               Prs, Prs->BS, Prs->BS->As.Loop.LContinue);

	    N = Prs->BS->As.Loop.LBreak;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;

	    Ebcscript_Parser_resolve_loop(Prs, Prs->BS);

	    BS = Ebcscript_Parser_popBlockstack(Prs);
	    Ebcscript_Parser_deleteBlockstack(BS);

	    Ebcscript_Parser_deleteExpression($6);
	  }
	| _for_prefix_ ')' statement
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name *N;

	    /* 式3の直前 */
	    N = Prs->BS->As.Loop.LContinue;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;

	    /* 式3なし */

	    store_instruction(EBCSCRIPT_INSTRUCTION_JMP);
	    Ebcscript_Parser_store_address_n(
	                                  Prs, Prs->BS, Prs->BS->As.Loop.LBegin);

	    N = Prs->BS->As.Loop.LBreak;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;

	    Ebcscript_Parser_resolve_loop(Prs, Prs->BS);

	    BS = Ebcscript_Parser_popBlockstack(Prs);
	    Ebcscript_Parser_deleteBlockstack(BS);
	  }
	| _for_prefix_ expression ')' statement
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name *N;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($2);
	      YYABORT;
	    }

	    /* 式3の直前 */
	    N = Prs->BS->As.Loop.LContinue;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;

	    Ebcscript_Parser_gencode_expr_rv(Prs, $2);
	    store_instruction(EBCSCRIPT_INSTRUCTION_JMP);
	    Ebcscript_Parser_store_address_n(
	                                  Prs, Prs->BS, Prs->BS->As.Loop.LBegin);

	    N = Prs->BS->As.Loop.LBreak;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;

	    Ebcscript_Parser_resolve_loop(Prs, Prs->BS);

	    BS = Ebcscript_Parser_popBlockstack(Prs);
	    Ebcscript_Parser_deleteBlockstack(BS);

	    Ebcscript_Parser_deleteExpression($2);
	  }
	;

jump_statement
	: GOTO IDENTIFIER ';'
	  {
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name **N0, *N;
	    int Nest;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Name_free($2);
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (N0 = Ebcscript_Parser_Blockstack_findLabel(BS, $2)) {
	        N = *N0;
	        break;
	      }
	    }
	    if (BS == NULL) {
	      for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	        if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION) {
	          break;
	        }
	      }
	      if (BS == NULL) {
	        log_debug0("")
	        longjmp(Prs->CheckPoint, 1);
	      }
	      /* 仮登録 */
	      N = Ebcscript_newName_label($2);
	      N->As.Label.Nest = 1;	/* 暫定的に最低ネストレベルの1 */
	      N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_UNDEFINED;
	      N->As.Label.CodeAddress = NULL;
	      Ebcscript_Parser_Blockstack_addLabel(BS, N);
	    }

	    if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION) {

	      /* {}の中へのジャンプは禁止する。
	         SUB_SPのオペランド＝フレームサイズを計算する必要がある。*/
	      if (Prs->Nest < N->As.Label.Nest) {
	        log_error0("\'goto\' into a block\n")
	        longjmp(Prs->CheckPoint, 1);
	      }

	      if (N->As.Label.Addressing !=
	                                  EBCSCRIPT_NAME_ADDRESSING_UNDEFINED) {
	        for (Nest = Prs->Nest; Nest > N->As.Label.Nest; Nest--) {
	          store_instruction(EBCSCRIPT_INSTRUCTION_MOV_BP_SP);
	          store_instruction(EBCSCRIPT_INSTRUCTION_POP_BP);
	        }
	        store_instruction(EBCSCRIPT_INSTRUCTION_JMP);
	        store_address((void *)
	               (N->As.Label.CodeAddress - Prs->TU->CP + Prs->TU->Code));
	      } else {
	        /* gotoの前方参照でラベルが仮登録されている場合 */
	        /* アドレスだけバックパッチするのではなく、
	           JMP及びPUSH_BP, MOV_SP_BPのコード自体をバックパッチする。
	           NOPで埋めて領域確保。ブロック出口でコード生成 */
	        SList_addFront(BS->As.Function.UnresolvedGotoList,
	         Ebcscript_newUnresolved_goto(
	                  (void *)(Prs->TU->CP - Prs->TU->Code), N, Prs->Nest));
	        for (Nest = Prs->Nest; Nest > N->As.Label.Nest; Nest--) {
	          store_instruction(EBCSCRIPT_INSTRUCTION_NOP);
	          store_instruction(EBCSCRIPT_INSTRUCTION_NOP);
	        }
	        store_instruction(EBCSCRIPT_INSTRUCTION_NOP);
	        store_address(NULL);
	      }

	    }
	    Ebcscript_Name_free($2);
	  }
	| CONTINUE ';'
	  {
	    int Nest;
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name **N0, *N;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (N0 = Ebcscript_Parser_Blockstack_findLabel(BS, "$continue")) {
	        N = *N0;
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_error0("continue statement not within loop\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    for (Nest = Prs->Nest; Nest > N->As.Label.Nest; Nest--) {
	      store_instruction(EBCSCRIPT_INSTRUCTION_MOV_BP_SP);
	      store_instruction(EBCSCRIPT_INSTRUCTION_POP_BP);
	    }
	    store_instruction(EBCSCRIPT_INSTRUCTION_JMP);
	    Ebcscript_Parser_store_address_n(Prs, BS, N);
	  }
	| BREAK ';'
	  {
	    int Nest;
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name **N0, *N;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (N0 = Ebcscript_Parser_Blockstack_findLabel(BS, "$break")) {
	        N = *N0;
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_error0("break statement not within loop or switch\n")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    for (Nest = Prs->Nest; Nest > N->As.Label.Nest; Nest--) {
	      store_instruction(EBCSCRIPT_INSTRUCTION_MOV_BP_SP);
	      store_instruction(EBCSCRIPT_INSTRUCTION_POP_BP);
	    }
	    store_instruction(EBCSCRIPT_INSTRUCTION_JMP);
	    Ebcscript_Parser_store_address_n(Prs, BS, N);
	  }
	| RETURN ';'
	  {
	    int Nest;
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name **N0, *N;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (N0 = Ebcscript_Parser_Blockstack_findLabel(BS, "$return")) {
	        N = *N0;
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_debug0("")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION) {

	      if (BS->As.Function.ReturnValue->Kind !=
	                                             EBCSCRIPT_TYPE_KIND_VOID) {
	        log_warning0(
	                "\'return\' with a value, in function returning void\n")
	        longjmp(Prs->CheckPoint, 1);
	      }

/*	      N = BS->As.Function.LReturn;*/
	      for (Nest = Prs->Nest; Nest > N->As.Label.Nest; Nest--) {
	        store_instruction(EBCSCRIPT_INSTRUCTION_MOV_BP_SP);
	        store_instruction(EBCSCRIPT_INSTRUCTION_POP_BP);
	      }
	      store_instruction(EBCSCRIPT_INSTRUCTION_JMP);
	      Ebcscript_Parser_store_address_n(Prs, BS, N);

	    }
	  }
	| RETURN expression ';'
	  {
	    void *Address;
	    int Nest;
	    ebcscript_parser_blockstack *BS;
	    ebcscript_name **N0, *N;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteExpression($2);
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (N0 = Ebcscript_Parser_Blockstack_findLabel(BS, "$return")) {
	        N = *N0;
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_debug0("")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION) {

	      if (BS->As.Function.ReturnValue->Kind ==
	                                             EBCSCRIPT_TYPE_KIND_VOID) {
	        log_warning0("\'return\' with no value, "
	                                     "in function returning non-void\n")
	        longjmp(Prs->CheckPoint, 1);
	      }

	      if (!Ebcscript_Type_equals(
	                           BS->As.Function.ReturnValue, $2->TypeTree)) {
	        $2 = Ebcscript_Parser_newExpression_cast(
	                                       BS->As.Function.ReturnValue, $2);
	      }

	      Ebcscript_Parser_gencode_expr_rv(Prs, $2);

	      /* 戻り値領域の先頭 */
	      Address = (void *)(
	         sizeof(void *)	/*前BP*/
	       + sizeof(void *)	/*戻り番地*/
	       + BS->As.Function.ParametersSize );

	      if (Prs->Nest == BS->Nest) {
/*	      if (Prs->Nest == N->As.Label.Nest) {*/
	        gencode_op_storage(POP_LMEM, BS->As.Function.ReturnValue);
	        store_address(Address);
	      }
	      if (Prs->Nest > BS->Nest) {
/*	      if (Prs->Nest > N->As.Label.Nest) {*/
	        store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_BP);
	        for (Nest = Prs->Nest; Nest > BS->Nest; Nest--) {
/*	        for (Nest = Prs->Nest; Nest > N->As.Label.Nest; Nest--) {*/
	          store_instruction(EBCSCRIPT_INSTRUCTION_LOAD_P);
	        }
	        store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I);
	        store_address(Address);
	        store_instruction(EBCSCRIPT_INSTRUCTION_ADD_P);

	        gencode_op_storage(STORE, BS->As.Function.ReturnValue);
	      }

/*	      N = BS->As.Function.LReturn;*/
	      for (Nest = Prs->Nest; Nest > N->As.Label.Nest; Nest--) {
	        store_instruction(EBCSCRIPT_INSTRUCTION_MOV_BP_SP);
	        store_instruction(EBCSCRIPT_INSTRUCTION_POP_BP);
	      }
	      store_instruction(EBCSCRIPT_INSTRUCTION_JMP);
	      Ebcscript_Parser_store_address_n(Prs, BS, N);

	    }
	    Ebcscript_Parser_deleteExpression($2);
	  }
	;

/*------------------------------------------------------- External definition */
translation_unit
	: external_declaration
	| translation_unit external_declaration
	;


external_declaration
	: declaration
	| function_definition
	;

function_definition
	: declaration_specifiers declarator
	  {
	    ebcscript_parser_blockstack *BS, *F, *B;
	    ebcscript_type *T;
	    ebcscript_name *N, **N0;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteDeclaration($1);
	      Ebcscript_deleteName($2);
	      YYABORT;
	    }

	    for (BS = Prs->BS; BS != NULL; BS = BS->Prev) {
	      if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT) {
	        break;
	      }
	    }
	    if (BS == NULL) {
	      log_debug0("")
	      longjmp(Prs->CheckPoint, 1);
	    }

	    /* $<Declaration>1の情報から型表現木を作る */
	    T = Ebcscript_Parser_Declaration_toTypeTree($1);

	    /* StorageClassにより場合分け */
	    if ($1->StorageClass ==
	                       EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_TYPEDEF) {
	      log_error0("関数定義の記憶域クラスがtypedefになっている。\n")
	    }
	    if ($1->StorageClass == 0) {
	      /* 型を適用 */
	      Ebcscript_Name_applyType($2, Ebcscript_Type_dup(T));

	      /* 名前の種類の確定 */
	      Ebcscript_Name_fixKind($2);

	      if ($2->Kind == EBCSCRIPT_NAME_KIND_VARIABLE) {
	        log_debug0("an invalid kind of name\n")
	        longjmp(Prs->CheckPoint, 1);
	      }
	      if ($2->Kind == EBCSCRIPT_NAME_KIND_FUNCTION) {
	        $2->As.Function.CodeAddress =
	                                  (void *)(Prs->TU->CP - Prs->TU->Code);
	        $2->As.Function.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;
	        $2->As.Function.Linkage = EBCSCRIPT_NAME_LINKAGE_EXTERNAL;
	        $2->As.Function.FunctionID = NULL;
/*	        $2->As.Function.FunctionID = getDummy();*/

	        /* 記号表へ登録 */
	        if (N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(
	                                                  BS, $2->Identifier)) {
	          /* N0は、仮引数がabstract_declaratorである可能性 */
	          Ebcscript_Parser_checkRepetation_varfunc(Prs, *N0, $2,
	                                                             Prs->Nest);
	          $2 = *N0;
	        } else {
	          Ebcscript_Parser_Blockstack_addVarFuncTypeEnum(BS, $2);
	        }
	      }
	    }
	    if ($1->StorageClass ==
	                        EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_STATIC) {
	      /* 型を適用 */
	      Ebcscript_Name_applyType($2, Ebcscript_Type_dup(T));

	      /* 名前の種類の確定 */
	      Ebcscript_Name_fixKind($2);

	      if ($2->Kind == EBCSCRIPT_NAME_KIND_VARIABLE) {
	        log_debug0("an invalid kind of name\n")
	        longjmp(Prs->CheckPoint, 1);
	      }
	      if ($2->Kind == EBCSCRIPT_NAME_KIND_FUNCTION) {
	        $2->As.Function.CodeAddress =
	                               (void *)( Prs->TU->CP - Prs->TU->Code);
	        $2->As.Function.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;
	        $2->As.Function.Linkage = EBCSCRIPT_NAME_LINKAGE_INTERNAL;
	        $2->As.Function.FunctionID = NULL;
/*	        $2->As.Function.FunctionID = getDummy();*/

	        /* 記号表へ登録 */
	        if (N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(
	                                                BS, $2->Identifier)) {
	          Ebcscript_Parser_checkRepetation_varfunc(Prs, *N0, $2,
	                                                             Prs->Nest);
	          $2 = *N0;
	        } else {
	          Ebcscript_Parser_Blockstack_addVarFuncTypeEnum(BS, $2);
	        }
	      }
	    }
	    if ($1->StorageClass ==
	                        EBCSCRIPT_PARSER_DECLARATION_SPECIFIER_EXTERN) {
	      /* 型を適用 */
	      Ebcscript_Name_applyType($2, Ebcscript_Type_dup(T));

	      /* 名前の種類の確定 */
	      Ebcscript_Name_fixKind($2);

	      if ($2->Kind == EBCSCRIPT_NAME_KIND_VARIABLE) {
	        log_debug0("an invalid kind of name\n")
	        longjmp(Prs->CheckPoint, 1);
	      }
	      if ($2->Kind == EBCSCRIPT_NAME_KIND_FUNCTION) {
	        $2->As.Function.CodeAddress =
	                                (void *)(Prs->TU->CP - Prs->TU->Code);
	        $2->As.Function.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;
	        $2->As.Function.Linkage = EBCSCRIPT_NAME_LINKAGE_EXTERNAL;
	        $2->As.Function.FunctionID = NULL;
/*	        $2->As.Function.FunctionID = getDummy();*/

	        /* 記号表へ登録 */
	        if (N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(
	                                                  BS, $2->Identifier)) {
	          Ebcscript_Parser_checkRepetation_varfunc(Prs, *N0, $2,
	                                                             Prs->Nest);
	          $2 = *N0;
	        } else {
	          Ebcscript_Parser_Blockstack_addVarFuncTypeEnum(BS, $2);
	        }
	      }
	    }

	    Prs->Nest++;
	    B = Ebcscript_Parser_newBlockstack_block();
	    Ebcscript_Parser_pushBlockstack(Prs, B);

	    /* 入口処理のコード生成 */
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_BP);
	    store_instruction(EBCSCRIPT_INSTRUCTION_MOV_SP_BP);
	    /* フレームの確保 */
	    store_instruction(EBCSCRIPT_INSTRUCTION_PUSH_IMM_I);
	    SList_addFront(Prs->BS->As.Block.UnresolvedFrameSizeList,
	     Ebcscript_newUnresolved((void *)(Prs->TU->CP - Prs->TU->Code),
	                                                               NULL));
	    store_int(0);
	    store_instruction(EBCSCRIPT_INSTRUCTION_SUB_SP);

	    /* 仮引数をローカルの名前表に登録 */
	    {
	      ebcscript_name *Param, *Param1;
	      slist *Parameters;
	      slist_cell *P;

	      Parameters = $2->As.Function.TypeTree->As.Function.Parameters;
	      for (P = Parameters->Head.Next;
	           P != NULL;
	           P = P->Next) {
	        Param = (ebcscript_name *)P->Datum;
	        Param1 = Ebcscript_Name_dup(Param);
	        Ebcscript_Parser_Blockstack_addVarFuncTypeEnum(B, Param1);
	        Param1->As.Variable.Address +=
	                  sizeof(void *)/*復帰アドレス*/ + sizeof(void *)/*BP*/;
	        Param1->As.Variable.Nest = 1/*Prs->Nest*/;
	      }
	    }

	    /* 出口への局所ジャンプ先アドレス管理 */
	    F = Ebcscript_Parser_newBlockstack_function();
	    Ebcscript_Parser_pushBlockstack(Prs, F);
	    F->As.Function.LReturn->As.Label.Nest = Prs->Nest;	/* 1 */
	    F->As.Function.ReturnValue =
	                      $2->As.Function.TypeTree->As.Function.ReturnValue;
	    F->As.Function.ParametersSize =
	               $2->As.Function.TypeTree->As.Function.ParametersSize;
	  }
	  compound_statement
	  {
	    ebcscript_parser_blockstack *F, *B;
	    ebcscript_name *N;

	    if (setjmp(Prs->CheckPoint) != 0) {
	      Ebcscript_Parser_deleteDeclaration($1);
	      Ebcscript_deleteName($2);
	      YYABORT;
	    }

	    /* 出口への局所ジャンプ先のバックパッチ */
	    N = Prs->BS->As.Function.LReturn;
	    N->As.Label.Nest = Prs->Nest;
	    N->As.Label.CodeAddress = (void *)(Prs->TU->CP - Prs->TU->Code);
	    N->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_ONCODE;
	    Ebcscript_Parser_resolve_function(Prs, Prs->BS);
	    F = Ebcscript_Parser_popBlockstack(Prs);
	    Ebcscript_Parser_deleteBlockstack(F);

	    /* 出口処理のコード生成 */
	    store_instruction(EBCSCRIPT_INSTRUCTION_MOV_BP_SP);
	    store_instruction(EBCSCRIPT_INSTRUCTION_POP_BP);
	    store_instruction(EBCSCRIPT_INSTRUCTION_RET);

	    /* フレームサイズのバックパッチ */
	    Prs->BS->As.Block.FrameSize = (int)Prs->BS->As.Block.DP;
	    Ebcscript_Parser_resolve_block(Prs, Prs->BS);

	    Prs->Nest--;
	    B = Ebcscript_Parser_popBlockstack(Prs);
	    Ebcscript_Parser_deleteBlockstack(B);

	    Ebcscript_Parser_deleteDeclaration($1);
	  }
	;
%%

static void yyerror(ebcscript_parser *Prs, const char *S)
{
	log_error0("syntax error\n")
}
