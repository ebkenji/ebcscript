/*******************************************************************************
   Project : C script
   File    : name.h
   Date    : 2018.3.23-
   Note    : 記号表
*******************************************************************************/
#ifndef EBCSCRIPT_NAME
#define EBCSCRIPT_NAME

#include "btree.h"
#include "slist.h"
#include "boolean.h"

/*
名前空間
(1)ラベル名
(2)構造体、共用体及び列挙体のタグ名
(3)構造体又は共用体のメンバ名
(4)その他。変数名、関数名、typedef名及び列挙定数名
*/

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                                 構造体宣言 */
/* -------------------------------------------------------------------------- */
/* 型の構造を表現する木。葉は必ず基本データ型 */
enum ebcscript_type_kind {
	EBCSCRIPT_TYPE_KIND_VOID,

	EBCSCRIPT_TYPE_KIND_CHAR,
	EBCSCRIPT_TYPE_KIND_UCHAR,
	EBCSCRIPT_TYPE_KIND_SHORT,
	EBCSCRIPT_TYPE_KIND_USHORT,
	EBCSCRIPT_TYPE_KIND_INT,
	EBCSCRIPT_TYPE_KIND_UINT,
	EBCSCRIPT_TYPE_KIND_LONG,
	EBCSCRIPT_TYPE_KIND_ULONG,
	EBCSCRIPT_TYPE_KIND_FLOAT,
	EBCSCRIPT_TYPE_KIND_DOUBLE,

	EBCSCRIPT_TYPE_KIND_ENUMERATOR,
	EBCSCRIPT_TYPE_KIND_ENUMERATION,
	EBCSCRIPT_TYPE_KIND_POINTER,
	EBCSCRIPT_TYPE_KIND_ARRAY,
	EBCSCRIPT_TYPE_KIND_STRUCT,
	EBCSCRIPT_TYPE_KIND_UNION,
	EBCSCRIPT_TYPE_KIND_FUNCTION,
};

typedef struct ebcscript_type ebcscript_type;
struct ebcscript_type {
	enum ebcscript_type_kind Kind;
	union {

	  struct ebcscript_type_primvoid {
	    size_t Size;
	  } Primvoid;

	  struct ebcscript_type_primchar {
	    size_t Size;
	  } Primchar;

	  struct ebcscript_type_primshort {
	    size_t Size;
	  } Primshort;

	  struct ebcscript_type_primint {
	    size_t Size;
	  } Primint;

	  struct ebcscript_type_primlong {
	    size_t Size;
	  } Primlong;

	  struct ebcscript_type_primuchar {
	    size_t Size;
	  } Primuchar;

	  struct ebcscript_type_primushort {
	    size_t Size;
	  } Primushort;

	  struct ebcscript_type_primuint {
	    size_t Size;
	  } Primuint;

	  struct ebcscript_type_primulong {
	    size_t Size;
	  } Primulong;

	  struct ebcscript_type_primfloat {
	    size_t Size;
	  } Primfloat;

	  struct ebcscript_type_primdouble {
	    size_t Size;
	  } Primdouble;

	  struct ebcscript_type_enumerator {
	    size_t Size;
	  } Enumerator;

	  struct ebcscript_type_enumeration {
	    boolean IsEmpty;	/* true: 不完全型 */
	    size_t Size;
	  } Enumeration;

	  struct ebcscript_type_pointer {
	    ebcscript_type *Value;
	    size_t Size;
	  } Pointer;

	  struct ebcscript_type_array {
	    size_t Size;	/* sizeof()の返す値 */
	    int Length;		/* 要素数 -1:未設定, >=0:固定サイズ */
	    size_t Align;
	    ebcscript_type *Value;
	  } Array;

	  struct ebcscript_type_struct {
	    size_t Size;
	    size_t Align;
	    slist/*<ebcscript_name *>*/ *Members; /* empty: 不完全型 */
	  } Struct;

	  struct ebcscript_type_union {
	    size_t Size;
	    size_t Align;
	    slist/*<ebcscript_name *>*/ *Members; /* empty: 不完全型 */
	  } Union;

	  struct ebcscript_type_function {
/*	    size_t ReturnValueSize;*/
	    size_t ParametersSize;
	    ebcscript_type *ReturnValue;
	    slist/*<ebcscript_name *>*/ *Parameters;
	  } Function;

	} As;
};

/* 名前情報 */
enum ebcscript_name_kind {
	EBCSCRIPT_NAME_KIND_LABEL,

	EBCSCRIPT_NAME_KIND_VARIABLE,
	EBCSCRIPT_NAME_KIND_FUNCTION,
	EBCSCRIPT_NAME_KIND_ENUMERATOR,
	EBCSCRIPT_NAME_KIND_TYPEDEF,

	EBCSCRIPT_NAME_KIND_STRUCT,
	EBCSCRIPT_NAME_KIND_UNION,
	EBCSCRIPT_NAME_KIND_ENUMERATION,
};

enum ebcscript_name_addressing {
	EBCSCRIPT_NAME_ADDRESSING_UNDEFINED,
	EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE,
	EBCSCRIPT_NAME_ADDRESSING_FUNCTIONID,
					/* コードセグメント上の絶対アドレス */
	EBCSCRIPT_NAME_ADDRESSING_ONSTACKFRAME,
	EBCSCRIPT_NAME_ADDRESSING_ONCODE,
};

enum ebcscript_name_linkage {
	EBCSCRIPT_NAME_LINKAGE_NOLINK,
	EBCSCRIPT_NAME_LINKAGE_EXTERNAL,
	EBCSCRIPT_NAME_LINKAGE_INTERNAL,
};

typedef struct ebcscript_name ebcscript_name;
struct ebcscript_name {
	enum ebcscript_name_kind Kind;
	char *Identifier;

	union {

	  /* ラベル名 */
	  struct ebcscript_name_label {
	    void *CodeAddress;
	    enum ebcscript_name_addressing Addressing;
	    int Nest;
	  } Label;

	  /* 構造体の変数や関数ポインタも含まれる */
	  struct ebcscript_name_variable {
	    ebcscript_type *TypeTree;
	    void *Address;
	    enum ebcscript_name_addressing Addressing;
	    enum ebcscript_name_linkage Linkage;
	    int Nest;	/* -1:未設定, 0:大域的, 1:局所的、引数, 2<:ブロック */
/*	    void *VariableID;*/
	  } Variable;

	  struct ebcscript_name_function {
	    ebcscript_type *TypeTree;
	    void *CodeAddress;
	    enum ebcscript_name_addressing Addressing;
	    enum ebcscript_name_linkage Linkage;
	    void *FunctionID;
	  } Function;

	  struct ebcscript_name_enumerator {
	    ebcscript_type *TypeTree;
	    int Value;
	  } Enumerator;


	  struct ebcscript_name_typedef {
	    ebcscript_type *TypeTree;
	  } Typedef;

	  /* 構造体、共用体及び列挙体のタグ名 */
	  struct ebcscript_name_struct {
	    ebcscript_type *TypeTree;
	  } Struct;

	  struct ebcscript_name_union {
	    ebcscript_type *TypeTree;
	  } Union;

	  struct ebcscript_name_enumeration {
	    ebcscript_type *TypeTree;
	  } Enumeration;

	} As;
};

/* アドレス未解決 */
typedef struct ebcscript_unresolved ebcscript_unresolved;
struct ebcscript_unresolved {
	void *CP;
	int Nest;
	ebcscript_name *N;
};

/* 定数情報 */
enum ebcscript_literal_kind {
	EBCSCRIPT_LITERAL_KIND_INTEGER,
	EBCSCRIPT_LITERAL_KIND_FLOATING,
	EBCSCRIPT_LITERAL_KIND_CHARACTER,
	EBCSCRIPT_LITERAL_KIND_STRING,
};

typedef struct ebcscript_literal ebcscript_literal;
struct ebcscript_literal {
	enum ebcscript_literal_kind Kind;
	ebcscript_type *TypeTree;
	char *Text;
	union {
	  long Integer;
	  double Floating;
	  char Character;
	  char *String;
	} As;
};

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
extern FILE *Ebcscript_Name_Fplog;
extern size_t Ebcscript_Name_MallocTotal;
extern void *(*Ebcscript_Name_malloc)(size_t Size);
extern void (*Ebcscript_Name_free)(void *);
extern void (*Ebcscript_Name_exit)(int);
extern void (*Ebcscript_Name_log)(const char *Fmt, ...);

/*                                                                   関数宣言 */
/* -------------------------------------------------------------------------- */
void Ebcscript_Name_onexit(void);

ebcscript_type *Ebcscript_newType();
void Ebcscript_deleteType(ebcscript_type *);

void Ebcscript_deleteType_void(ebcscript_type *);
void Ebcscript_deleteType_char(ebcscript_type *);
void Ebcscript_deleteType_short(ebcscript_type *);
void Ebcscript_deleteType_int(ebcscript_type *);
void Ebcscript_deleteType_long(ebcscript_type *);
void Ebcscript_deleteType_uchar(ebcscript_type *);
void Ebcscript_deleteType_ushort(ebcscript_type *);
void Ebcscript_deleteType_uint(ebcscript_type *);
void Ebcscript_deleteType_ulong(ebcscript_type *);
void Ebcscript_deleteType_float(ebcscript_type *);
void Ebcscript_deleteType_double(ebcscript_type *);
void Ebcscript_deleteType_pointer(ebcscript_type *);
void Ebcscript_deleteType_array(ebcscript_type *);
void Ebcscript_deleteType_function(ebcscript_type *);
void Ebcscript_deleteType_struct(ebcscript_type *);
void Ebcscript_deleteType_union(ebcscript_type *);
void Ebcscript_deleteType_enumeration(ebcscript_type *);
void Ebcscript_deleteType_enumerator(ebcscript_type *);

ebcscript_type *Ebcscript_Type_Void(void);
ebcscript_type *Ebcscript_Type_Char(void);
ebcscript_type *Ebcscript_Type_Short(void);
ebcscript_type *Ebcscript_Type_Int(void);
ebcscript_type *Ebcscript_Type_Long(void);
ebcscript_type *Ebcscript_Type_UChar(void);
ebcscript_type *Ebcscript_Type_UShort(void);
ebcscript_type *Ebcscript_Type_UInt(void);
ebcscript_type *Ebcscript_Type_ULong(void);
ebcscript_type *Ebcscript_Type_Float(void);
ebcscript_type *Ebcscript_Type_Double(void);

ebcscript_type *Ebcscript_newType_void(void);
ebcscript_type *Ebcscript_newType_char(void);
ebcscript_type *Ebcscript_newType_short(void);
ebcscript_type *Ebcscript_newType_int(void);
ebcscript_type *Ebcscript_newType_long(void);
ebcscript_type *Ebcscript_newType_uchar(void);
ebcscript_type *Ebcscript_newType_ushort(void);
ebcscript_type *Ebcscript_newType_uint(void);
ebcscript_type *Ebcscript_newType_ulong(void);
ebcscript_type *Ebcscript_newType_float(void);
ebcscript_type *Ebcscript_newType_double(void);
ebcscript_type *Ebcscript_newType_pointer(void);
ebcscript_type *Ebcscript_newType_array(void);
ebcscript_type *Ebcscript_newType_function(void);
ebcscript_type *Ebcscript_newType_struct(void);
ebcscript_type *Ebcscript_newType_union(void);
ebcscript_type *Ebcscript_newType_enumeration(void);
ebcscript_type *Ebcscript_newType_enumerator(void);

ebcscript_type *Ebcscript_makeType_function(
                                  ebcscript_type *ReturnValue, int NParam, ...);
ebcscript_type *Ebcscript_makeType_pointer(ebcscript_type *Value);
ebcscript_type *Ebcscript_makeType_array(ebcscript_type *Value);

ebcscript_name *Ebcscript_Type_Function_findParameter(
                         struct ebcscript_type_function *TF, const char *Key);
boolean Ebcscript_Type_Function_addParameter(
                         struct ebcscript_type_function *TF, ebcscript_name *N);

ebcscript_name *Ebcscript_Type_Struct_findMember(
                           struct ebcscript_type_struct *TS, const char *Key);
boolean Ebcscript_Type_Struct_addMember(
                           struct ebcscript_type_struct *TS, ebcscript_name *N);

ebcscript_name *Ebcscript_Type_Union_findMember(
                            struct ebcscript_type_union *TU, const char *Key);
boolean Ebcscript_Type_Union_addMember(
                            struct ebcscript_type_union *TU, ebcscript_name *N);

/* 構造体型, 共用体型及び関数型では、名前も複製する。 */
ebcscript_type *Ebcscript_Type_dup(ebcscript_type *);

void Ebcscript_Type_addLast(ebcscript_type **, ebcscript_type *);
size_t Ebcscript_Type_getAlign(ebcscript_type *);
size_t Ebcscript_Type_getSize(ebcscript_type *);
boolean Ebcscript_Type_equals(ebcscript_type *, ebcscript_type *);
boolean Ebcscript_Type_isInteger(ebcscript_type *);
boolean Ebcscript_Type_isNumeric(ebcscript_type *);
boolean Ebcscript_Type_isPointer(ebcscript_type *);
boolean Ebcscript_Type_isStructOrUnion(ebcscript_type *);

/* 型を数値に対応させる。
   char 0, short 2, int 4, long 6, float 8, double 10
   uchar 1, ushort 3, uint 5, ulon 7 */
int Ebcscript_Type_toRank(ebcscript_type *);

/* 型を表す数値から型表現木へ変換 */
ebcscript_type *Ebcscript_Type_rankToType(int);

/* 二項演算のノードの型を決定する */
ebcscript_type *Ebcscript_Type_balance_numeric(ebcscript_type *,
                                                              ebcscript_type *);
ebcscript_type *Ebcscript_Type_balance_integer(ebcscript_type *,
                                                              ebcscript_type *);

void Ebcscript_Type_print(ebcscript_type *);

ebcscript_name *Ebcscript_newName(char *);
ebcscript_name *Ebcscript_newName_label(char *);
ebcscript_name *Ebcscript_newName_variable(char *);
ebcscript_name *Ebcscript_newName_struct(char *);
ebcscript_name *Ebcscript_newName_union(char *);
ebcscript_name *Ebcscript_newName_enumeration(char *);
ebcscript_name *Ebcscript_newName_enumerator(char *);
ebcscript_name *Ebcscript_newName_function(char *);

/* TypeTreeは消さない。 */
void Ebcscript_deleteName(ebcscript_name *);

/* TypeTreeも複製する。 */
ebcscript_name *Ebcscript_Name_dup(ebcscript_name *);

/* 型の適用。nameが持つ型表現木（実際には(1)ポインタ、(2)関数、
   (3)配列、を要素とするリスト）の末尾にtypeを追加する。 */
void Ebcscript_Name_applyType(ebcscript_name *, ebcscript_type *);

/* 名前の種類をTypeTreeの先頭要素の種類を見て確定させる。 */
void Ebcscript_Name_fixKind(ebcscript_name *);

/* 加算すべき値を返す。Offset += align(Offset, Alignment) */
size_t Ebcscript_Name_align(size_t Offset, size_t Alignment);

void Ebcscript_Name_print(ebcscript_name *);

ebcscript_unresolved *Ebcscript_newUnresolved(void *CP, ebcscript_name *N);
ebcscript_unresolved *Ebcscript_newUnresolved_goto(
                                         void *CP, ebcscript_name *N, int Nest);
void Ebcscript_deleteUnresolved(ebcscript_unresolved *);

ebcscript_literal *Ebcscript_newLiteral(char *);
ebcscript_literal *Ebcscript_newLiteral_string(char *);
void Ebcscript_deleteLiteral(ebcscript_literal *);
void Ebcscript_Literal_print(ebcscript_literal *);

#endif
