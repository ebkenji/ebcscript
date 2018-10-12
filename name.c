/*******************************************************************************
   Project : C script
   File    : name.c
   Date    : 2018.3.23-
   Note    : 名前表
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "name.h"
#include "slist.h"
#include "boolean.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Name_malloc(size_t Size);
void _Ebcscript_Name_free(void *P);
void _Ebcscript_Name_exit(int Status);
void _Ebcscript_Name_log(const char *Fmt, ...);

FILE *Ebcscript_Name_Fplog/* = stdout*/;
size_t Ebcscript_Name_MallocTotal = 0;
void *(*Ebcscript_Name_malloc)(size_t Size) = &_Ebcscript_Name_malloc;
void (*Ebcscript_Name_free)(void *) = &_Ebcscript_Name_free;
void (*Ebcscript_Name_exit)(int) = &_Ebcscript_Name_exit;
void (*Ebcscript_Name_log)(const char *Fmt, ...) = &_Ebcscript_Name_log;

/*                                                                   関数定義 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Name_malloc(size_t Size)
{
	void *P;

	if ((P = malloc(Size + sizeof(size_t))) == NULL){
	  Ebcscript_Name_log(
	   "Ebcscript_Name_malloc(): "
	   "error: memory allocation error(total=%d bytes)\n",
	   Ebcscript_Name_MallocTotal);
	  Ebcscript_Name_exit(0);
	}
	*(size_t *)P = Size;
	Ebcscript_Name_MallocTotal += Size;
	return (size_t *)P + 1;
}

void _Ebcscript_Name_free(void *P)
{
	P = (size_t *)P - 1;
	Ebcscript_Name_MallocTotal -= *(size_t *)P;
	free(P);
	return;
}

void Ebcscript_Name_onexit(void)
{
	fclose(Ebcscript_Name_Fplog);
}

void _Ebcscript_Name_exit(int Status)
{
	Ebcscript_Name_onexit();
	exit(Status);
}

void _Ebcscript_Name_log(const char *Fmt, ...)
{
	va_list Args;

	fprintf(Ebcscript_Name_Fplog, "ebcscript_name: ");

	va_start(Args, Fmt);
	vfprintf(Ebcscript_Name_Fplog, Fmt, Args);
	va_end(Args);
}
/* -------------------------------------------------------------------------- */
ebcscript_type *Ebcscript_newType(void)
{
	ebcscript_type *T;

	T = Ebcscript_Name_malloc(sizeof(ebcscript_type));
	return T;
}

ebcscript_type *Ebcscript_Type_Void(void)
{
	static ebcscript_type T;

	T.Kind = EBCSCRIPT_TYPE_KIND_VOID;
	T.As.Primvoid.Size = sizeof(void);	/* sizeof(void)は1になる */
	return &T;
}

ebcscript_type *Ebcscript_newType_void(void)
{
/*	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_VOID;
	T->As.Primvoid.Size = sizeof(void);
	return T;
*/
	return Ebcscript_Type_Void();
}

void Ebcscript_deleteType_void(ebcscript_type *T)
{
/*	Ebcscript_Name_free(T);*/
}

ebcscript_type *Ebcscript_Type_Char(void)
{
	static ebcscript_type T;

	T.Kind = EBCSCRIPT_TYPE_KIND_CHAR;
	T.As.Primchar.Size = sizeof(char);
	return &T;
}

ebcscript_type *Ebcscript_newType_char(void)
{
/*	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_CHAR;
	T->As.Primchar.Size = sizeof(char);
	return T;
*/
	return Ebcscript_Type_Char();
}

void Ebcscript_deleteType_char(ebcscript_type *T)
{
/*	Ebcscript_Name_free(T);*/
}

ebcscript_type *Ebcscript_Type_Short(void)
{
	static ebcscript_type T;

	T.Kind = EBCSCRIPT_TYPE_KIND_SHORT;
	T.As.Primshort.Size = sizeof(short);
	return &T;
}

ebcscript_type *Ebcscript_newType_short(void)
{
/*	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_SHORT;
	T->As.Primshort.Size = sizeof(short);
	return T;
*/
	return Ebcscript_Type_Short();
}

void Ebcscript_deleteType_short(ebcscript_type *T)
{
/*	Ebcscript_Name_free(T);*/
}

ebcscript_type *Ebcscript_Type_Int(void)
{
	static ebcscript_type T;

	T.Kind = EBCSCRIPT_TYPE_KIND_INT;
	T.As.Primint.Size = sizeof(int);
	return &T;
}

ebcscript_type *Ebcscript_newType_int(void)
{
/*	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_INT;
	T->As.Primint.Size = sizeof(int);
	return T;
*/
	return Ebcscript_Type_Int();
}

void Ebcscript_deleteType_int(ebcscript_type *T)
{
/*	Ebcscript_Name_free(T);*/
}

ebcscript_type *Ebcscript_Type_Long(void)
{
	static ebcscript_type T;

	T.Kind = EBCSCRIPT_TYPE_KIND_LONG;
	T.As.Primlong.Size = sizeof(long);
	return &T;
}

ebcscript_type *Ebcscript_newType_long(void)
{
/*	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_LONG;
	T->As.Primlong.Size = sizeof(long);
	return T;
*/
	return Ebcscript_Type_Long();
}

void Ebcscript_deleteType_long(ebcscript_type *T)
{
/*	Ebcscript_Name_free(T);*/
}

ebcscript_type *Ebcscript_Type_UChar(void)
{
	static ebcscript_type T;

	T.Kind = EBCSCRIPT_TYPE_KIND_UCHAR;
	T.As.Primuchar.Size = sizeof(unsigned char);
	return &T;
}

ebcscript_type *Ebcscript_newType_uchar(void)
{
/*	ebcscript_type T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_UCHAR;
	T->As.Primuchar.Size = sizeof(unsigned char);
	return T;
*/
	return Ebcscript_Type_UChar();
}

void Ebcscript_deleteType_uchar(ebcscript_type *T)
{
/*	Ebcscript_Name_free(T);*/
}

ebcscript_type *Ebcscript_Type_UShort(void)
{
	static ebcscript_type T;

	T.Kind = EBCSCRIPT_TYPE_KIND_USHORT;
	T.As.Primushort.Size = sizeof(unsigned short);
	return &T;
}

ebcscript_type *Ebcscript_newType_ushort(void)
{
/*	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_USHORT;
	T->As.Primushort.Size = sizeof(unsigned short);
	return T;
*/
	return Ebcscript_Type_UShort();
}

void Ebcscript_deleteType_ushort(ebcscript_type *T)
{
/*	Ebcscript_Name_free(T);*/
}

ebcscript_type *Ebcscript_Type_UInt(void)
{
	static ebcscript_type T;

	T.Kind = EBCSCRIPT_TYPE_KIND_UINT;
	T.As.Primuint.Size = sizeof(unsigned int);
	return &T;
}

ebcscript_type *Ebcscript_newType_uint(void)
{
/*	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_UINT;
	T->As.Primuint.Size = sizeof(unsigned int);
	return T;
*/
	return Ebcscript_Type_UInt();
}

void Ebcscript_deleteType_uint(ebcscript_type *T)
{
/*	Ebcscript_Name_free(T);*/
}

ebcscript_type *Ebcscript_Type_ULong(void)
{
	static ebcscript_type T;

	T.Kind = EBCSCRIPT_TYPE_KIND_ULONG;
	T.As.Primulong.Size = sizeof(unsigned long);
	return &T;
}

ebcscript_type *Ebcscript_newType_ulong(void)
{
/*	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_ULONG;
	T->As.Primulong.Size = sizeof(unsigned long);
	return T;
*/
	return Ebcscript_Type_ULong();
}

void Ebcscript_deleteType_ulong(ebcscript_type *T)
{
/*	Ebcscript_Name_free(T);*/
}

ebcscript_type *Ebcscript_Type_Float(void)
{
	static ebcscript_type T;

	T.Kind = EBCSCRIPT_TYPE_KIND_FLOAT;
	T.As.Primfloat.Size = sizeof(float);
	return &T;
}

ebcscript_type *Ebcscript_newType_float(void)
{
/*	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_FLOAT;
	T->As.Primfloat.Size = sizeof(float);
	return T;
*/
	return Ebcscript_Type_Float();
}

void Ebcscript_deleteType_float(ebcscript_type *T)
{
/*	Ebcscript_Name_free(T);*/
}

ebcscript_type *Ebcscript_Type_Double(void)
{
	static ebcscript_type T;

	T.Kind = EBCSCRIPT_TYPE_KIND_DOUBLE;
	T.As.Primdouble.Size = sizeof(double);
	return &T;
}

ebcscript_type *Ebcscript_newType_double(void)
{
/*	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_DOUBLE;
	T->As.Primdouble.Size = sizeof(double);
	return T;
*/
	return Ebcscript_Type_Double();
}

void Ebcscript_deleteType_double(ebcscript_type *T)
{
/*	Ebcscript_Name_free(T);*/
}

ebcscript_type *Ebcscript_newType_pointer(void)
{
	ebcscript_type *P;

	P = Ebcscript_newType();
	P->Kind = EBCSCRIPT_TYPE_KIND_POINTER;
	P->As.Pointer.Value = NULL;
	P->As.Pointer.Size = sizeof(void *);
	return P;
}

void Ebcscript_deleteType_pointer(ebcscript_type *T)
{
	Ebcscript_deleteType(T->As.Pointer.Value);
	Ebcscript_Name_free(T);
}

ebcscript_type *Ebcscript_newType_array(void)
{
	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_ARRAY;
	T->As.Array.Value = NULL;
	T->As.Array.Size = 0;
	T->As.Array.Length = -1;	/* 要素数 -1:未設定, >=0:固定サイズ */
	T->As.Array.Align = 1;
	return T;
}

void Ebcscript_deleteType_array(ebcscript_type *T)
{
	Ebcscript_deleteType(T->As.Array.Value);
	Ebcscript_Name_free(T);
}

ebcscript_type *Ebcscript_newType_function(void)
{
	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_FUNCTION;
/*	T->As.Function.ReturnValueSize = 0;*/
	T->As.Function.ParametersSize = 0;
	T->As.Function.ReturnValue = NULL;
	T->As.Function.Parameters = newSList();
	return T;
}

void Ebcscript_deleteType_function(ebcscript_type *T)
{
	Ebcscript_deleteType(T->As.Function.ReturnValue);
	SList_clear(T->As.Function.Parameters,
	                                (void (*)(void *))Ebcscript_deleteName);
	deleteSList(T->As.Function.Parameters);
	Ebcscript_Name_free(T);
}

ebcscript_type *Ebcscript_newType_struct(void)
{
	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_STRUCT;
	T->As.Struct.Size = 0;
	T->As.Struct.Align = 1;
	T->As.Struct.Members = newSList();
	return T;
}

void Ebcscript_deleteType_struct(ebcscript_type *T)
{
	SList_clear(T->As.Struct.Members,
	                                (void (*)(void *))Ebcscript_deleteName);
	deleteSList(T->As.Struct.Members);
	Ebcscript_Name_free(T);
}

ebcscript_type *Ebcscript_newType_union(void)
{
	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_UNION;
	T->As.Union.Size = 0;
	T->As.Union.Align = 1;
	T->As.Union.Members = newSList();
	return T;
}

void Ebcscript_deleteType_union(ebcscript_type *T)
{
	SList_clear(T->As.Union.Members,
	                                (void (*)(void *))Ebcscript_deleteName);
	deleteSList(T->As.Union.Members);
	Ebcscript_Name_free(T);
}

ebcscript_type *Ebcscript_newType_enumeration(void)
{
	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_ENUMERATION;
	T->As.Enumeration.IsEmpty = false;	/* false: 不完全型でない */
	T->As.Enumeration.Size = sizeof(int);
	return T;
}

void Ebcscript_deleteType_enumeration(ebcscript_type *T)
{
	Ebcscript_Name_free(T);
}

ebcscript_type *Ebcscript_newType_enumerator(void)
{
	ebcscript_type *T;

	T = Ebcscript_newType();
	T->Kind = EBCSCRIPT_TYPE_KIND_ENUMERATOR;
	T->As.Enumerator.Size = sizeof(int);
	return T;
}

void Ebcscript_deleteType_enumerator(ebcscript_type *T)
{
	Ebcscript_Name_free(T);
}

ebcscript_name *Ebcscript_Type_Function_findParameter(
                            struct ebcscript_type_function *TF, const char *Key)
{
	slist_cell *P;
	ebcscript_name *N;

	for (P = TF->Parameters->Head.Next; P != NULL; P = P->Next) {
	  N = (ebcscript_name *)P->Datum;
	  if (!strcmp(N->Identifier, Key)) {
	    return N;
	  }
	}
	return NULL;
/*
	ebcscript_name *N;

	SList_foreach(TF->Parameters, N, {
	  if (!strcmp(N->Identifier, Key)) {
	    return N;
	  }
	})
	return NULL;
*/
}

boolean Ebcscript_Type_Function_addParameter(
                     struct ebcscript_type_function *TF, ebcscript_name *NParam)
{
	ebcscript_name *N;
	size_t Offset;

	/* 引数名の重複チェック */
	if (Ebcscript_Type_Function_findParameter(TF, NParam->Identifier)) {
	  return false;
	}

	/* 引数の追加 */
	SList_addLast(TF->Parameters, NParam);

	/* オフセットの決定 */
	Offset = TF->ParametersSize;

	NParam->As.Variable.Address = (void *)Offset;
	NParam->As.Variable.Addressing =
	                                 EBCSCRIPT_NAME_ADDRESSING_ONSTACKFRAME;

	Offset += Ebcscript_Type_getSize(NParam->As.Variable.TypeTree);
	TF->ParametersSize = Offset;
	return true;
}

size_t Ebcscript_Name_align(size_t Offset, size_t Alignment)
{
	/* 例
	   Offset , Alignment -> Offset
	   0, 4 -> 0
	   1, 4 -> 4
	   2, 4 -> 4
	   3, 4 -> 4
	   4, 4 -> 4
	   5, 4 -> 8 */
	return (Alignment - 1) - (Offset + Alignment - 1) % Alignment;
}

ebcscript_name *Ebcscript_Type_Struct_findMember(
                              struct ebcscript_type_struct *TS, const char *Key)
{
	slist_cell *P;
	ebcscript_name *N;

	for (P = TS->Members->Head.Next; P != NULL; P = P->Next) {
	  N = (ebcscript_name *)P->Datum;
	  if (!strcmp(N->Identifier, Key)) {
	    return N;
	  }
	}
	return NULL;
}

boolean Ebcscript_Type_Struct_addMember(struct ebcscript_type_struct *TS,
                                                        ebcscript_name *NMember)
{
	ebcscript_name *N;
	size_t Offset, Align;

	/* メンバ名の重複チェック */
	if (Ebcscript_Type_Struct_findMember(TS, NMember->Identifier)) {
	  return false;
	}

	/* メンバの追加 */
	SList_addLast(TS->Members, NMember);

	/* オフセット、パディングの決定 */
	Offset = TS->Size;

	Align = Ebcscript_Type_getAlign(NMember->As.Variable.TypeTree);
	Offset += Ebcscript_Name_align(Offset, Align);

	NMember->As.Variable.Address = (void *)Offset;
	NMember->As.Variable.Addressing =
	                                 EBCSCRIPT_NAME_ADDRESSING_ONSTACKFRAME;

	Offset += Ebcscript_Type_getSize(NMember->As.Variable.TypeTree);
	TS->Size = Offset;
	TS->Align = (Align > TS->Align) ? Align : TS->Align;
	return true;
}

ebcscript_name *Ebcscript_Type_Union_findMember(
                               struct ebcscript_type_union *TU, const char *Key)
{
	slist_cell *P;
	ebcscript_name *N;

	for (P = TU->Members->Head.Next; P != NULL; P = P->Next) {
	  N = (ebcscript_name *)P->Datum;
	  if (!strcmp(N->Identifier, Key)) {
	    return N;
	  }
	}
	return NULL;
}

boolean Ebcscript_Type_Union_addMember(struct ebcscript_type_union *TU,
                                                        ebcscript_name *NMember)
{
	ebcscript_name *N;
	size_t Align, Size;

	/* メンバ名の重複チェック */
	if (Ebcscript_Type_Union_findMember(TU, NMember->Identifier)) {
	  return false;
	}

	/* メンバ名の追加 */
	SList_addLast(TU->Members, NMember);

	/* オフセット、パディングの決定 */
	Align = Ebcscript_Type_getAlign(NMember->As.Variable.TypeTree);
	Size  = Ebcscript_Type_getSize (NMember->As.Variable.TypeTree);

	NMember->As.Variable.Address = (void *)0;
	NMember->As.Variable.Addressing =
	                                 EBCSCRIPT_NAME_ADDRESSING_ONSTACKFRAME;

	TU->Align = (Align > TU->Align) ? Align : TU->Align;
	TU->Size  = (Size  > TU->Size)  ? Size  : TU->Size;
	return true;
}

void Ebcscript_deleteType(ebcscript_type *T)
{
	if (T == NULL)
	  return;

	switch (T->Kind) {
	  case EBCSCRIPT_TYPE_KIND_VOID:
	    Ebcscript_deleteType_void(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_CHAR:
	    Ebcscript_deleteType_char(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_UCHAR:
	    Ebcscript_deleteType_uchar(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_SHORT:
	    Ebcscript_deleteType_short(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_USHORT:
	    Ebcscript_deleteType_ushort(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_INT:
	    Ebcscript_deleteType_int(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_UINT:
	    Ebcscript_deleteType_uint(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	    Ebcscript_deleteType_enumerator(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_LONG:
	    Ebcscript_deleteType_long(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_ULONG:
	    Ebcscript_deleteType_ulong(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_FLOAT:
	    Ebcscript_deleteType_float(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:
	    Ebcscript_deleteType_double(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	    Ebcscript_deleteType_enumeration(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_POINTER:
	    Ebcscript_deleteType_pointer(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_ARRAY:
	    Ebcscript_deleteType_array(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_STRUCT:
	    Ebcscript_deleteType_struct(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_UNION:
	    Ebcscript_deleteType_union(T);
	    break;
	  case EBCSCRIPT_TYPE_KIND_FUNCTION:
	    Ebcscript_deleteType_function(T);
	    break;
	  default:
	    Ebcscript_Name_log(
	     "Ebcscript_deleteType(): debug: an invalid value\n"
	    );
	    Ebcscript_Name_exit(1);
	    break;
	}
}

void Ebcscript_Type_addLast(ebcscript_type **P, ebcscript_type *A)
{
	while (*P != NULL) {
	  switch ((*P)->Kind) {
	    case EBCSCRIPT_TYPE_KIND_POINTER:
	      P = &(*P)->As.Pointer.Value;
	      break;
	    case EBCSCRIPT_TYPE_KIND_ARRAY:
	      P = &(*P)->As.Array.Value;
	      break;
	    case EBCSCRIPT_TYPE_KIND_FUNCTION:
	      P = &(*P)->As.Function.ReturnValue;
	      break;
	    default:
	      Ebcscript_Name_log(
	       "Ebcscript_Type_addLast(): debug: an invalid value\n"
	      );
	      Ebcscript_Name_exit(1);
	      break;
	  }
	}
	*P = A;
	return;
}

size_t Ebcscript_Type_getAlign(ebcscript_type *T)
{
	size_t Align;

	if (T == NULL)
	  return 0;

	switch (T->Kind) {
	  case EBCSCRIPT_TYPE_KIND_VOID:
	    Align = T->As.Primvoid.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_CHAR:
	    Align = T->As.Primchar.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_SHORT:
	    Align = T->As.Primshort.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_INT:
	    Align = T->As.Primint.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_LONG:
	    Align = T->As.Primlong.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_UCHAR:
	    Align = T->As.Primuchar.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_USHORT:
	    Align = T->As.Primushort.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_UINT:
	    Align = T->As.Primuint.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_ULONG:
	    Align = T->As.Primulong.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_FLOAT:
	    Align = T->As.Primfloat.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:
	    Align = T->As.Primdouble.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	    Align = T->As.Enumerator.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	    Align = T->As.Enumeration.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_POINTER:
	    Align = T->As.Pointer.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_ARRAY:
	    Align = T->As.Array.Align;
	    break;
	  case EBCSCRIPT_TYPE_KIND_STRUCT:
	    Align = T->As.Struct.Align;
	    break;
	  case EBCSCRIPT_TYPE_KIND_UNION:
	    Align = T->As.Union.Align;
	    break;
	  case EBCSCRIPT_TYPE_KIND_FUNCTION:
	    Align = 0;
	    break;
	  default:
	    Align = 0;
	    break;
	}
	return Align;
}

size_t Ebcscript_Type_getSize(ebcscript_type *T)
{
	size_t Size;

	if (T == NULL)
	  return 0;

	switch (T->Kind) {
	  case EBCSCRIPT_TYPE_KIND_VOID:
	    Size = T->As.Primvoid.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_CHAR:
	    Size = T->As.Primchar.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_SHORT:
	    Size = T->As.Primshort.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_INT:
	    Size = T->As.Primint.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_LONG:
	    Size = T->As.Primlong.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_UCHAR:
	    Size = T->As.Primuchar.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_USHORT:
	    Size = T->As.Primushort.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_UINT:
	    Size = T->As.Primuint.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_ULONG:
	    Size = T->As.Primulong.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_FLOAT:
	    Size = T->As.Primfloat.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:
	    Size = T->As.Primdouble.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	    Size = T->As.Enumerator.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	    Size = T->As.Enumeration.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_POINTER:
	    Size = T->As.Pointer.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_ARRAY:
	    Size = T->As.Array.Length *
	                              Ebcscript_Type_getSize(T->As.Array.Value);
	    T->As.Array.Size = Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_STRUCT:
	    Size = T->As.Struct.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_UNION:
	    Size = T->As.Union.Size;
	    break;
	  case EBCSCRIPT_TYPE_KIND_FUNCTION:
	    Size = 0;
	    break;
	  default:
	    Size = 0;
	    break;
	}
	return Size;
}

ebcscript_type *Ebcscript_Type_dup(ebcscript_type *T0)
{
	ebcscript_type *T;
	ebcscript_name *N0, *N;
	slist_cell *P;

	if (T0 == NULL)
	  return NULL;

	switch (T0->Kind) {
	  case EBCSCRIPT_TYPE_KIND_VOID:
	    T = Ebcscript_newType_void();
	    break;
	  case EBCSCRIPT_TYPE_KIND_CHAR:
	    T = Ebcscript_newType_char();
	    break;
	  case EBCSCRIPT_TYPE_KIND_SHORT:
	    T = Ebcscript_newType_short();
	    break;
	  case EBCSCRIPT_TYPE_KIND_INT:
	    T = Ebcscript_newType_int();
	    break;
	  case EBCSCRIPT_TYPE_KIND_LONG:
	    T = Ebcscript_newType_long();
	    break;
	  case EBCSCRIPT_TYPE_KIND_UCHAR:
	    T = Ebcscript_newType_uchar();
	    break;
	  case EBCSCRIPT_TYPE_KIND_USHORT:
	    T = Ebcscript_newType_ushort();
	    break;
	  case EBCSCRIPT_TYPE_KIND_UINT:
	    T = Ebcscript_newType_uint();
	    break;
	  case EBCSCRIPT_TYPE_KIND_ULONG:
	    T = Ebcscript_newType_ulong();
	    break;
	  case EBCSCRIPT_TYPE_KIND_FLOAT:
	    T = Ebcscript_newType_float();
	    break;
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:
	    T = Ebcscript_newType_double();
	    break;
	  case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	    T = Ebcscript_newType_enumerator();
	    break;
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	    T = Ebcscript_newType_enumeration();
	    T->As.Enumeration.IsEmpty = T0->As.Enumeration.IsEmpty;
	    break;
	  case EBCSCRIPT_TYPE_KIND_POINTER:
	    T = Ebcscript_newType_pointer();
	    T->As.Pointer.Value = Ebcscript_Type_dup(T0->As.Pointer.Value);
	    break;
	  case EBCSCRIPT_TYPE_KIND_ARRAY:
	    T = Ebcscript_newType_array();
	    T->As.Array.Size   = T0->As.Array.Size;
	    T->As.Array.Length = T0->As.Array.Length;
	    T->As.Array.Align  = T0->As.Array.Align;
	    T->As.Array.Value  = Ebcscript_Type_dup(T0->As.Array.Value);
	    break;

	  case EBCSCRIPT_TYPE_KIND_STRUCT:
	    T = Ebcscript_newType_struct();
	    T->As.Struct.Size   = T0->As.Struct.Size;
	    T->As.Struct.Align  = T0->As.Struct.Align;

	    for (P = T0->As.Struct.Members->Head.Next;
	         P != NULL;
	         P = P->Next) {
	      N0 = (ebcscript_name *)P->Datum;
	      N = Ebcscript_Name_dup(N0);
	      SList_addLast(T->As.Struct.Members, N);
	    }
	    break;

	  case EBCSCRIPT_TYPE_KIND_UNION:
	    T = Ebcscript_newType_union();
	    T->As.Union.Size   = T0->As.Union.Size;
	    T->As.Union.Align  = T0->As.Union.Align;

	    for (P = T0->As.Union.Members->Head.Next;
	         P != NULL;
	         P = P->Next) {
	      N0 = (ebcscript_name *)P->Datum;
	      N = Ebcscript_Name_dup(N0);
	      SList_addLast(T->As.Union.Members, N);
	    }
	    break;

	  case EBCSCRIPT_TYPE_KIND_FUNCTION:
	    T = Ebcscript_newType_function();
	    T->As.Function.ParametersSize = T0->As.Function.ParametersSize;
	    T->As.Function.ReturnValue =
	                        Ebcscript_Type_dup(T0->As.Function.ReturnValue);

	    for (P = T0->As.Function.Parameters->Head.Next;
	         P != NULL;
	         P = P->Next) {
	      N0 = (ebcscript_name *)P->Datum;
	      N = Ebcscript_Name_dup(N0);
	      SList_addLast(T->As.Function.Parameters, N);
	    }
	    break;

	  default:
	    break;
	}
	return T;
}

boolean Ebcscript_Type_equals(ebcscript_type *T1, ebcscript_type *T2)
{
	slist_cell *P1, *P2;
	ebcscript_name *N1, *N2;
	boolean B;

	if (T1->Kind != T2->Kind) {
	  return false;
	}

	switch (T1->Kind) {
	  case EBCSCRIPT_TYPE_KIND_VOID:
	  case EBCSCRIPT_TYPE_KIND_CHAR:
	  case EBCSCRIPT_TYPE_KIND_SHORT:
	  case EBCSCRIPT_TYPE_KIND_INT:
	  case EBCSCRIPT_TYPE_KIND_LONG:
	  case EBCSCRIPT_TYPE_KIND_UCHAR:
	  case EBCSCRIPT_TYPE_KIND_USHORT:
	  case EBCSCRIPT_TYPE_KIND_UINT:
	  case EBCSCRIPT_TYPE_KIND_ULONG:
	  case EBCSCRIPT_TYPE_KIND_FLOAT:
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:
	  case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	    return true;
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	    return
	         (T1->As.Enumeration.IsEmpty == T2->As.Enumeration.IsEmpty);
	  case EBCSCRIPT_TYPE_KIND_POINTER:
	    return Ebcscript_Type_equals(T1->As.Pointer.Value,
	                                                T2->As.Pointer.Value);
	  case EBCSCRIPT_TYPE_KIND_ARRAY:
	    return T1->As.Array.Size == T2->As.Array.Size
	     && Ebcscript_Type_equals(T1->As.Array.Value,
	                                                  T2->As.Array.Value);
	  case EBCSCRIPT_TYPE_KIND_STRUCT:
	    /* 全メンバの型が一致していても無名ならfalse */
	    /* 無名かどうかの判定は今のところ不可能 */
	    B = true;
	    P1 = T1->As.Struct.Members->Head.Next;
	    P2 = T2->As.Struct.Members->Head.Next;
	    while (P1 != NULL && P2 != NULL) {
	      N1 = (ebcscript_name *)P1->Datum;
	      N2 = (ebcscript_name *)P2->Datum;
	      if (N1->Kind != N2->Kind) {
	        B &= false;
	      }
	      if (N1->Kind == EBCSCRIPT_NAME_KIND_VARIABLE
	       && N2->Kind == EBCSCRIPT_NAME_KIND_VARIABLE) {
	        B &= Ebcscript_Type_equals(N1->As.Variable.TypeTree,
	                                           N2->As.Variable.TypeTree);
	      }
	      if (N1->Kind == EBCSCRIPT_NAME_KIND_FUNCTION
	       && N2->Kind == EBCSCRIPT_NAME_KIND_FUNCTION) {
	        B &= Ebcscript_Type_equals(N1->As.Function.TypeTree,
	                                           N2->As.Function.TypeTree);
	      }
	      P1 = P1->Next;
	      P2 = P2->Next;
	    }
	    B &= (P1 == NULL && P2 == NULL);
	    return B;
	  case EBCSCRIPT_TYPE_KIND_UNION:
	    /* 全メンバの型が一致していても無名ならfalse */
	    /* 無名かどうかの判定は今のところ不可能 */
	    B = true;
	    P1 = T1->As.Union.Members->Head.Next;
	    P2 = T2->As.Union.Members->Head.Next;
	    while (P1 != NULL && P2 != NULL) {
	      N1 = (ebcscript_name *)P1->Datum;
	      N2 = (ebcscript_name *)P2->Datum;
	      if (N1->Kind != N2->Kind) {
	        B &= false;
	      }
	      if (N1->Kind == EBCSCRIPT_NAME_KIND_VARIABLE
	       && N2->Kind == EBCSCRIPT_NAME_KIND_VARIABLE) {
	        B &= Ebcscript_Type_equals(N1->As.Variable.TypeTree,
	                                           N2->As.Variable.TypeTree);
	      }
	      if (N1->Kind == EBCSCRIPT_NAME_KIND_FUNCTION
	       && N2->Kind == EBCSCRIPT_NAME_KIND_FUNCTION) {
	        B &= Ebcscript_Type_equals(N1->As.Function.TypeTree,
	                                           N2->As.Function.TypeTree);
	      }
	      P1 = P1->Next;
	      P2 = P2->Next;
	    }
	    B &= (P1 == NULL && P2 == NULL);
	    return B;
	  case EBCSCRIPT_TYPE_KIND_FUNCTION:
	    /* 戻り値の型が一致、並びが一致、各引数の型が一致 */
	    B = true;
	    B &= Ebcscript_Type_equals(T1->As.Function.ReturnValue,
	                                           T2->As.Function.ReturnValue);
	    P1 = T1->As.Function.Parameters->Head.Next;
	    P2 = T2->As.Function.Parameters->Head.Next;
	    while (P1 != NULL && P2 != NULL) {
	      N1 = (ebcscript_name *)P1->Datum;
	      N2 = (ebcscript_name *)P2->Datum;
	      if (N1->Kind != N2->Kind) {
	        B &= false;
	      }
	      if (N1->Kind == EBCSCRIPT_NAME_KIND_VARIABLE
	       && N2->Kind == EBCSCRIPT_NAME_KIND_VARIABLE) {
	        B &= Ebcscript_Type_equals(N1->As.Variable.TypeTree,
	                                           N2->As.Variable.TypeTree);
	      }
	      if (N1->Kind == EBCSCRIPT_NAME_KIND_FUNCTION
	       && N2->Kind == EBCSCRIPT_NAME_KIND_FUNCTION) {
	        B &= Ebcscript_Type_equals(N1->As.Function.TypeTree,
	                                           N2->As.Function.TypeTree);
	      }
	      P1 = P1->Next;
	      P2 = P2->Next;
	    }
	    B &= (P1 == NULL && P2 == NULL);
	    return B;
	  default:
	    break;
	}
	return false;
}

boolean Ebcscript_Type_isInteger(ebcscript_type *T)
{
	switch (T->Kind) {
	  case EBCSCRIPT_TYPE_KIND_CHAR:
	  case EBCSCRIPT_TYPE_KIND_SHORT:
	  case EBCSCRIPT_TYPE_KIND_INT:
	  case EBCSCRIPT_TYPE_KIND_LONG:
	  case EBCSCRIPT_TYPE_KIND_UCHAR:
	  case EBCSCRIPT_TYPE_KIND_USHORT:
	  case EBCSCRIPT_TYPE_KIND_UINT:
	  case EBCSCRIPT_TYPE_KIND_ULONG:
	    return true;
	  case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	    return true;
	}
	return false;
}

boolean Ebcscript_Type_isNumeric(ebcscript_type *T)
{
	switch (T->Kind) {
	  case EBCSCRIPT_TYPE_KIND_CHAR:
	  case EBCSCRIPT_TYPE_KIND_SHORT:
	  case EBCSCRIPT_TYPE_KIND_INT:
	  case EBCSCRIPT_TYPE_KIND_LONG:
	  case EBCSCRIPT_TYPE_KIND_UCHAR:
	  case EBCSCRIPT_TYPE_KIND_USHORT:
	  case EBCSCRIPT_TYPE_KIND_UINT:
	  case EBCSCRIPT_TYPE_KIND_ULONG:
	    return true;
	  case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	    return true;
	  case EBCSCRIPT_TYPE_KIND_FLOAT:
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:
	    return true;
	}
	return false;
}

boolean Ebcscript_Type_isPointer(ebcscript_type *T)
{
	switch (T->Kind) {
	  case EBCSCRIPT_TYPE_KIND_POINTER:
	    return true;
	  case EBCSCRIPT_TYPE_KIND_ARRAY:
	    return true;
	  case EBCSCRIPT_TYPE_KIND_FUNCTION:
	    return true;
	}
	return false;
}

boolean Ebcscript_Type_isStructOrUnion(ebcscript_type *T)
{
	switch (T->Kind) {
	  case EBCSCRIPT_TYPE_KIND_STRUCT:
	  case EBCSCRIPT_TYPE_KIND_UNION:
	    return true;
	}
	return false;
}

int Ebcscript_Type_toRank(ebcscript_type *T)
{
	switch (T->Kind) {
	  case EBCSCRIPT_TYPE_KIND_CHAR:
	    return 0;
	  case EBCSCRIPT_TYPE_KIND_SHORT:
	    return 2;
	  case EBCSCRIPT_TYPE_KIND_INT:
	  case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	    return 4;
	  case EBCSCRIPT_TYPE_KIND_LONG:
	    return 6;
	  case EBCSCRIPT_TYPE_KIND_UCHAR:
	    return 1;
	  case EBCSCRIPT_TYPE_KIND_USHORT:
	    return 3;
	  case EBCSCRIPT_TYPE_KIND_UINT:
	    return 5;
	  case EBCSCRIPT_TYPE_KIND_ULONG:
	    return 7;
	  case EBCSCRIPT_TYPE_KIND_FLOAT:
	    return 8;
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:
	    return 10;
	}
	return 0;
}

ebcscript_type *Ebcscript_Type_rankToType(int R)
{
	switch (R) {
	  case 0:
	    return Ebcscript_Type_Char();
	  case 1:
	    return Ebcscript_Type_UChar();
	  case 2:
	    return Ebcscript_Type_Short();
	  case 3:
	    return Ebcscript_Type_UShort();
	  case 4:
	    return Ebcscript_Type_Int();
	  case 5:
	    return Ebcscript_Type_UInt();
	  case 6:
	    return Ebcscript_Type_Long();
	  case 7:
	    return Ebcscript_Type_ULong();
	  case 8:
	    return Ebcscript_Type_Float();
	  case 10:
	    return Ebcscript_Type_Double();
	}
	return NULL;
}

ebcscript_type *Ebcscript_Type_balance_numeric(ebcscript_type *T1,
                                                             ebcscript_type *T2)
{
	    int R, R1, R2;

	    /* 型を数値に対応させる */
	    R1 = Ebcscript_Type_toRank(T1);
	    R2 = Ebcscript_Type_toRank(T2);
	    R = (R1 >= R2) ? R1 : R2;

	    /* unsignedの格上げ。両方unsignedの場合はしない。*/
	    R += (R % 2 && (R1 + R2) % 2) ? 1 : 0;

	    /* rankからtypeへ変換 */
	    return Ebcscript_Type_rankToType(R);
}

ebcscript_type *Ebcscript_Type_balance_integer(ebcscript_type *T1,
                                                             ebcscript_type *T2)
{
	    int R, R1, R2;

	    /* 型を数値に対応させる */
	    R1 = Ebcscript_Type_toRank(T1);
	    R2 = Ebcscript_Type_toRank(T2);
	    R = (R1 >= R2) ? R1 : R2;

	    /* rankからtypeへ変換 */
	    return Ebcscript_Type_rankToType(R);
}

void Ebcscript_Type_print(ebcscript_type *T)
{
	switch (T->Kind) {
	  case EBCSCRIPT_TYPE_KIND_VOID:
	    fprintf(Ebcscript_Name_Fplog, "void");
	    break;
	  case EBCSCRIPT_TYPE_KIND_CHAR:
	    fprintf(Ebcscript_Name_Fplog, "char");
	    break;
	  case EBCSCRIPT_TYPE_KIND_SHORT:
	    fprintf(Ebcscript_Name_Fplog, "short");
	    break;
	  case EBCSCRIPT_TYPE_KIND_INT:
	    fprintf(Ebcscript_Name_Fplog, "int");
	    break;
	  case EBCSCRIPT_TYPE_KIND_LONG:
	    fprintf(Ebcscript_Name_Fplog, "long");
	    break;
	  case EBCSCRIPT_TYPE_KIND_UCHAR:
	    fprintf(Ebcscript_Name_Fplog, "uchar");
	    break;
	  case EBCSCRIPT_TYPE_KIND_USHORT:
	    fprintf(Ebcscript_Name_Fplog, "ushort");
	    break;
	  case EBCSCRIPT_TYPE_KIND_UINT:
	    fprintf(Ebcscript_Name_Fplog, "uint");
	    break;
	  case EBCSCRIPT_TYPE_KIND_ULONG:
	    fprintf(Ebcscript_Name_Fplog, "ulong");
	    break;
	  case EBCSCRIPT_TYPE_KIND_FLOAT:
	    fprintf(Ebcscript_Name_Fplog, "float");
	    break;
	  case EBCSCRIPT_TYPE_KIND_DOUBLE:
	    fprintf(Ebcscript_Name_Fplog, "double");
	    break;
	  case EBCSCRIPT_TYPE_KIND_STRUCT:
	    fprintf(Ebcscript_Name_Fplog, "struct");
	    break;
	  case EBCSCRIPT_TYPE_KIND_UNION:
	    fprintf(Ebcscript_Name_Fplog, "union");
	    break;
	  case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	    fprintf(Ebcscript_Name_Fplog, "enumeration");
	    break;
	  case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	    fprintf(Ebcscript_Name_Fplog, "enumerator");
	    break;
	  case EBCSCRIPT_TYPE_KIND_POINTER:
	    fprintf(Ebcscript_Name_Fplog, "pointer to ");
	    Ebcscript_Type_print(T->As.Pointer.Value);
	    break;
	  case EBCSCRIPT_TYPE_KIND_ARRAY:
	    fprintf(Ebcscript_Name_Fplog, "array[%d] of ", T->As.Array.Size);
	    Ebcscript_Type_print(T->As.Array.Value);
	    break;
	  case EBCSCRIPT_TYPE_KIND_FUNCTION:
	    fprintf(Ebcscript_Name_Fplog, "function returning ");
	    Ebcscript_Type_print(T->As.Function.ReturnValue);
	    break;
	  default:
	    break;
	}
}

/* -------------------------------------------------------------------------- */
ebcscript_name *Ebcscript_newName(char *Name)
{
	ebcscript_name *P;

	P = Ebcscript_Name_malloc(sizeof(ebcscript_name));
	P->Identifier = Ebcscript_Name_malloc(strlen(Name) + 1);
	strcpy(P->Identifier, Name);
	return P;
}

ebcscript_name *Ebcscript_newName_label(char *Name)
{
	ebcscript_name *P;

	P = Ebcscript_newName(Name);
	P->Kind = EBCSCRIPT_NAME_KIND_LABEL;
	P->As.Label.CodeAddress = NULL;
	P->As.Label.Addressing = EBCSCRIPT_NAME_ADDRESSING_UNDEFINED;
	P->As.Label.Nest = -1;
	return P;
}

ebcscript_name *Ebcscript_newName_variable(char *Name)
{
	ebcscript_name *P;

	P = Ebcscript_newName(Name);
	P->Kind = EBCSCRIPT_NAME_KIND_VARIABLE;
	P->As.Variable.TypeTree = NULL;
	P->As.Variable.Address = NULL;
	P->As.Variable.Addressing = EBCSCRIPT_NAME_ADDRESSING_UNDEFINED;
	P->As.Variable.Linkage = EBCSCRIPT_NAME_LINKAGE_EXTERNAL;
	P->As.Variable.Nest = -1;
	return P;
}

ebcscript_name *Ebcscript_newName_struct(char *Name)
{
	ebcscript_name *P;

	P = Ebcscript_newName(Name);
	P->Kind = EBCSCRIPT_NAME_KIND_STRUCT;
	P->As.Struct.TypeTree = NULL;
	return P;
}

ebcscript_name *Ebcscript_newName_union(char *Name)
{
	ebcscript_name *P;

	P = Ebcscript_newName(Name);
	P->Kind = EBCSCRIPT_NAME_KIND_UNION;
	P->As.Union.TypeTree = NULL;
	return P;
}

ebcscript_name *Ebcscript_newName_enumeration(char *Name)
{
	ebcscript_name *P;

	P = Ebcscript_newName(Name);
	P->Kind = EBCSCRIPT_NAME_KIND_ENUMERATION;
	P->As.Enumeration.TypeTree = NULL;
	return P;
}

ebcscript_name *Ebcscript_newName_enumerator(char *Name)
{
	ebcscript_name *P;

	P = Ebcscript_newName(Name);
	P->Kind = EBCSCRIPT_NAME_KIND_ENUMERATOR;
	P->As.Enumerator.TypeTree = NULL;
	P->As.Enumerator.Value = 0;
	return P;
}

void Ebcscript_deleteName(ebcscript_name *N)
{
	if (N == NULL)
	  return;

	switch (N->Kind) {
	  case EBCSCRIPT_NAME_KIND_LABEL:
	    break;
	  case EBCSCRIPT_NAME_KIND_VARIABLE:
	    Ebcscript_deleteType(N->As.Variable.TypeTree);
	    break;
	  case EBCSCRIPT_NAME_KIND_FUNCTION:
	    Ebcscript_deleteType(N->As.Function.TypeTree);
	    break;
	  case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	    Ebcscript_deleteType(N->As.Enumerator.TypeTree);
	    break;
	  case EBCSCRIPT_NAME_KIND_TYPEDEF:
	    Ebcscript_deleteType(N->As.Typedef.TypeTree);
	    break;

	  case EBCSCRIPT_NAME_KIND_STRUCT:
	    Ebcscript_deleteType(N->As.Struct.TypeTree);
	    break;
	  case EBCSCRIPT_NAME_KIND_UNION:
	    Ebcscript_deleteType(N->As.Union.TypeTree);
	    break;
	  case EBCSCRIPT_NAME_KIND_ENUMERATION:
	    Ebcscript_deleteType(N->As.Enumeration.TypeTree);
	    break;

	  defualt:
	    break;
	}
	Ebcscript_Name_free(N->Identifier);
	Ebcscript_Name_free(N);
}

ebcscript_name *Ebcscript_Name_dup(ebcscript_name *N0)
{
	ebcscript_name *N;

	if (N0 == NULL)
	  return NULL;

	switch (N0->Kind) {
	  case EBCSCRIPT_NAME_KIND_LABEL:
	    N = Ebcscript_newName_label(N0->Identifier);
	    N->As.Label.CodeAddress = N0->As.Label.CodeAddress;
	    N->As.Label.Addressing  = N0->As.Label.Addressing;
	    N->As.Label.Nest        = N0->As.Label.Nest;
	    break;
	  case EBCSCRIPT_NAME_KIND_VARIABLE:
	    N = Ebcscript_newName_variable(N0->Identifier);
	    N->As.Variable.Address    = N0->As.Variable.Address;
	    N->As.Variable.Addressing = N0->As.Variable.Addressing;
	    N->As.Variable.Linkage    = N0->As.Variable.Linkage;
	    N->As.Variable.Nest       = N0->As.Variable.Nest;
	    N->As.Variable.TypeTree =
	                           Ebcscript_Type_dup(N0->As.Variable.TypeTree);
	    break;
	  case EBCSCRIPT_NAME_KIND_FUNCTION:
	    N = Ebcscript_newName_variable(N0->Identifier);
	    N->Kind = N0->Kind;
	    N->As.Function.CodeAddress = N0->As.Function.CodeAddress;
	    N->As.Function.Addressing  = N0->As.Function.Addressing;
	    N->As.Function.Linkage     = N0->As.Function.Linkage;
	    N->As.Function.TypeTree =
	                           Ebcscript_Type_dup(N0->As.Function.TypeTree);
	    break;
	  case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	    N = Ebcscript_newName_enumerator(N0->Identifier);
	    N->As.Enumerator.Value = N0->As.Enumerator.Value;
	    N->As.Enumerator.TypeTree =
	                         Ebcscript_Type_dup(N0->As.Enumerator.TypeTree);
	    break;
	  case EBCSCRIPT_NAME_KIND_TYPEDEF:
	    N = Ebcscript_newName_variable(N0->Identifier);
	    N->Kind = N0->Kind;
	    N->As.Typedef.TypeTree =
	                            Ebcscript_Type_dup(N0->As.Typedef.TypeTree);
	    break;

	  case EBCSCRIPT_NAME_KIND_STRUCT:
	    N = Ebcscript_newName_struct(N0->Identifier);
	    N->As.Struct.TypeTree = Ebcscript_Type_dup(N0->As.Struct.TypeTree);
	    break;
	  case EBCSCRIPT_NAME_KIND_UNION:
	    N = Ebcscript_newName_union(N0->Identifier);
	    N->As.Union.TypeTree = Ebcscript_Type_dup(N0->As.Union.TypeTree);
	    break;
	  case EBCSCRIPT_NAME_KIND_ENUMERATION:
	    N = Ebcscript_newName_enumeration(N0->Identifier);
	    N->As.Enumeration.TypeTree =
	                      Ebcscript_Type_dup(N0->As.Enumeration.TypeTree);
	    break;

	  defualt:
	    break;
	}
	return N;
}

void Ebcscript_Name_applyType(ebcscript_name *N, ebcscript_type *T)
{
	ebcscript_type *T0;

	switch (N->Kind) {
	  case EBCSCRIPT_NAME_KIND_LABEL:
	    break;
	  case EBCSCRIPT_NAME_KIND_VARIABLE:
	    Ebcscript_Type_addLast(&N->As.Variable.TypeTree, T);
	    break;
	  case EBCSCRIPT_NAME_KIND_FUNCTION:
	    Ebcscript_Type_addLast(&N->As.Function.TypeTree, T);
	    T0 = N->As.Function.TypeTree;
	    break;
	  case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	    break;
	  case EBCSCRIPT_NAME_KIND_TYPEDEF:
	    Ebcscript_Type_addLast(&N->As.Typedef.TypeTree, T);
	    break;
	  case EBCSCRIPT_NAME_KIND_STRUCT:
	    Ebcscript_Type_addLast(&N->As.Struct.TypeTree, T);
	    break;
	  case EBCSCRIPT_NAME_KIND_UNION:
	    Ebcscript_Type_addLast(&N->As.Union.TypeTree, T);
	    break;
	  case EBCSCRIPT_NAME_KIND_ENUMERATION:
	    break;
	  default:
	    Ebcscript_Name_log("Ebcscript_Name_applyType(): "
	     "debug: an invalid kind of name %s\n", N->Identifier);
	    break;
	}
}

void Ebcscript_Name_fixKind(ebcscript_name *N)
{
	/* 暫定的にKindにはVARIABLEが設定されている前提 */
	if (N->Kind != EBCSCRIPT_NAME_KIND_VARIABLE) {
	  return;
	}

	if (N->As.Variable.TypeTree != NULL) {
	  switch (N->As.Variable.TypeTree->Kind) {
	    case EBCSCRIPT_TYPE_KIND_VOID:
	    case EBCSCRIPT_TYPE_KIND_CHAR:
	    case EBCSCRIPT_TYPE_KIND_SHORT:
	    case EBCSCRIPT_TYPE_KIND_INT:
	    case EBCSCRIPT_TYPE_KIND_LONG:
	    case EBCSCRIPT_TYPE_KIND_UCHAR:
	    case EBCSCRIPT_TYPE_KIND_USHORT:
	    case EBCSCRIPT_TYPE_KIND_UINT:
	    case EBCSCRIPT_TYPE_KIND_ULONG:
	    case EBCSCRIPT_TYPE_KIND_FLOAT:
	    case EBCSCRIPT_TYPE_KIND_DOUBLE:
	      N->Kind = EBCSCRIPT_NAME_KIND_VARIABLE;
	      break;
	    case EBCSCRIPT_TYPE_KIND_STRUCT:
	    case EBCSCRIPT_TYPE_KIND_UNION:
	    case EBCSCRIPT_TYPE_KIND_ENUMERATION:
	      N->Kind = EBCSCRIPT_NAME_KIND_VARIABLE;
	      break;
	    case EBCSCRIPT_TYPE_KIND_ENUMERATOR:
	      N->Kind = EBCSCRIPT_NAME_KIND_ENUMERATOR;
	      break;
	    case EBCSCRIPT_TYPE_KIND_POINTER:
	    case EBCSCRIPT_TYPE_KIND_ARRAY:
	      N->Kind = EBCSCRIPT_NAME_KIND_VARIABLE;
	      break;
	    case EBCSCRIPT_TYPE_KIND_FUNCTION:
	      N->Kind = EBCSCRIPT_NAME_KIND_FUNCTION;
	      {
	        ebcscript_name Tmp = *N;

	        N->As.Function.TypeTree    = Tmp.As.Variable.TypeTree;
	        N->As.Function.CodeAddress = Tmp.As.Variable.Address;
	        N->As.Function.Addressing  = Tmp.As.Variable.Addressing;
	        N->As.Function.Linkage     = Tmp.As.Variable.Linkage;
	      }
	      break;
	    default:
	      Ebcscript_Name_log("Ebcscript_Name_fixKind(): "
	       "debug: an invalid kind of name\n");
	      break;
	  }
	}
}

void Ebcscript_Name_print(ebcscript_name *N)
{
	static char *SA[] = {"UNDEFINED", "ABSOLUTE", "ONSTACKFRAME", "ONCODE"};
	static char *SL[] = {"NOLINK", "EXTERNAL", "INTERNAL"};

	fprintf(Ebcscript_Name_Fplog, "Name: %s\n", N->Identifier);

	switch (N->Kind) {
	  case EBCSCRIPT_NAME_KIND_LABEL:
	    break;

	  case EBCSCRIPT_NAME_KIND_VARIABLE:
	    fprintf(Ebcscript_Name_Fplog,
	     " Kind: variable\n"
	     " Address: %p\n"
	     " Addressing: %s\n"
	     " Linkage: %s\n"
	     " Nest: %d\n",
	     N->As.Variable.Address,
	     SA[N->As.Variable.Addressing],
	     SL[N->As.Variable.Linkage],
	     N->As.Variable.Nest
	    );
	    fprintf(Ebcscript_Name_Fplog, " Type: ");
	    Ebcscript_Type_print(N->As.Variable.TypeTree);
	    fprintf(Ebcscript_Name_Fplog, "\n");
	    break;
	  case EBCSCRIPT_NAME_KIND_FUNCTION:
	    fprintf(Ebcscript_Name_Fplog,
	     " Kind: function\n"
	     " CodeAddress: %p\n"
	     " Addressing: %s\n"
	     " Linkage: %s\n",
	     N->As.Function.CodeAddress,
	     SA[N->As.Function.Addressing],
	     SL[N->As.Function.Linkage]
	    );
	    fprintf(Ebcscript_Name_Fplog, " Type: ");
	    Ebcscript_Type_print(N->As.Function.TypeTree);
	    fprintf(Ebcscript_Name_Fplog, "\n");
	    break;
	  case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	    fprintf(Ebcscript_Name_Fplog,
	     " Kind: enumerator\n"
	     " Value: %d\n",
	     N->As.Enumerator.Value
	    );
	    break;
	  case EBCSCRIPT_NAME_KIND_TYPEDEF:
	    fprintf(Ebcscript_Name_Fplog,
	     " Kind: typedef\n"
	    );
	    break;

	  case EBCSCRIPT_NAME_KIND_STRUCT:
	    break;
	  case EBCSCRIPT_NAME_KIND_UNION:
	    break;
	  case EBCSCRIPT_NAME_KIND_ENUMERATION:
	    break;

	  defualt:
	    break;
	}

	fprintf(Ebcscript_Name_Fplog, "\n");
}
/* -------------------------------------------------------------------------- */
ebcscript_unresolved *Ebcscript_newUnresolved(
                                      void *CP, ebcscript_name *N/*, int Nest*/)
{
	ebcscript_unresolved *UR;

	UR = Ebcscript_Name_malloc(sizeof(ebcscript_unresolved));
	UR->CP = CP;
	UR->N = N;
/*	UR->Nest = Nest;*/
	return UR;
}

ebcscript_unresolved *Ebcscript_newUnresolved_goto(
                                          void *CP, ebcscript_name *N, int Nest)
{
	ebcscript_unresolved *UR;

	UR = Ebcscript_Name_malloc(sizeof(ebcscript_unresolved));
	UR->CP = CP;
	UR->N = N;
	UR->Nest = Nest;
	return UR;
}

void Ebcscript_deleteUnresolved(ebcscript_unresolved *UR)
{
	Ebcscript_Name_free(UR);
}

/* -------------------------------------------------------------------------- */
ebcscript_literal *Ebcscript_newLiteral(char *Text)
{
	ebcscript_literal *P;

	P = Ebcscript_Name_malloc(sizeof(ebcscript_literal));
	P->Text = Ebcscript_Name_malloc(strlen(Text) + 1);
	strcpy(P->Text, Text);
	return P;
}

void Ebcscript_deleteLiteral(ebcscript_literal *L)
{
	Ebcscript_deleteType(L->TypeTree);
	Ebcscript_Name_free(L->Text);
	Ebcscript_Name_free(L);
}

static int isoctdigit(char C)
{
	return '0' <= C && C <= '7';
}

ebcscript_literal *Ebcscript_newLiteral_string(char *Text)
{
	ebcscript_literal *P;
	int I, J, K;
	boolean IsInGap;
	char C, H, O;

	IsInGap = false;
	for (I = 1, J = 0; Text[I] != '\0'; I++) {
	  if (IsInGap == false) {
	    if (Text[I] == '\\' && Text[I + 1] == 'x' &&
	                                                isxdigit(Text[I + 2])) {
	      C = Text[I + 2];
	      C = ('0' <= C && C <= '9') ? C - '0' : C;
	      C = ('A' <= C && C <= 'F') ? C - 'A' + 10 : C;
	      C = ('a' <= C && C <= 'f') ? C - 'a' + 10 : C;
	      H = C;
	      if (isxdigit(Text[I + 3])) {
	        C = Text[I + 3];
	        C = ('0' <= C && C <= '9') ? C - '0' : C;
	        C = ('A' <= C && C <= 'F') ? C - 'A' + 10 : C;
	        C = ('a' <= C && C <= 'f') ? C - 'a' + 10 : C;
	        H = H * 16 + C;
	        I++;
	      }
	      I += 2; Text[J++] = H;
	    } else
	    if (Text[I] == '\\' && isoctdigit(Text[I + 1])) {
	      O = Text[I + 1] - '0';
	      for (K = 0; K < 3 && isoctdigit(Text[I + 2 + K]); K++) {
	        C = Text[I + 2 + K] - '0';
	        O = O * 8 + C;
	        I++;
	      }
	      I++; Text[J++] = O;
	    } else
	    if (Text[I] == '\\' && Text[I + 1] == 'n') {
	      I++; Text[J++] = '\n';
	    } else
	    if (Text[I] == '\\' && Text[I + 1] == 't') {
	      I++; Text[J++] = '\t';
	    } else
	    if (Text[I] == '\\' && Text[I + 1] == 'v') {
	      I++; Text[J++] = '\v';
	    } else
	    if (Text[I] == '\\' && Text[I + 1] == 'b') {
	      I++; Text[J++] = '\b';
	    } else
	    if (Text[I] == '\\' && Text[I + 1] == 'f') {
	      I++; Text[J++] = '\f';
	    } else
	    if (Text[I] == '\\' && Text[I + 1] == 'r') {
	      I++; Text[J++] = '\r';
	    } else
	    if (Text[I] == '\\' && Text[I + 1] == '\\') {
	      I++; Text[J++] = '\\';
	    } else
	    if (Text[I] == '\\' && Text[I + 1] == '\?') {
	      I++; Text[J++] = '\?';
	    } else
	    if (Text[I] == '\\' && Text[I + 1] == '\"') {
	      I++; Text[J++] = '\"';
	    } else
	    if (Text[I] == '\\' && Text[I + 1] == '\'') {
	      I++; Text[J++] = '\'';
	    } else
	    if (Text[I] == '\"') {
	      IsInGap = true;
	    } else {
	      Text[J++] = Text[I];
	    }
	  } else if (IsInGap == true) {
	    if (Text[I] == '\"') {
	      IsInGap = false;
	    } else {
	      /* 何もしない */
	    }
	  }
	}
	Text[J++] = '\0';

	P = Ebcscript_Name_malloc(sizeof(ebcscript_literal));
	P->Text = Ebcscript_Name_malloc(J);
	strcpy(P->Text, Text);
	return P;
}

void Ebcscript_Literal_print(ebcscript_literal *L)
{
	fprintf(Ebcscript_Name_Fplog, "Text: %s\n", L->Text);

	switch (L->Kind) {
	  case EBCSCRIPT_LITERAL_KIND_INTEGER:
	    fprintf(Ebcscript_Name_Fplog,
	     " Kind: integer\n"
	     " Value: %d\n",
	     L->As.Integer
	    );
	    fprintf(Ebcscript_Name_Fplog, " Type: ");
	    Ebcscript_Type_print(L->TypeTree);
	    fprintf(Ebcscript_Name_Fplog, "\n");
	    break;
	  case EBCSCRIPT_LITERAL_KIND_FLOATING:
	    fprintf(Ebcscript_Name_Fplog,
	     " Kind: floating\n"
	     " Value: %d\n",
	     L->As.Floating
	    );
	    fprintf(Ebcscript_Name_Fplog, " Type: ");
	    Ebcscript_Type_print(L->TypeTree);
	    fprintf(Ebcscript_Name_Fplog, "\n");
	    break;
	  case EBCSCRIPT_LITERAL_KIND_CHARACTER:
	    fprintf(Ebcscript_Name_Fplog,
	     " Kind: character\n"
	     " Value: %c\n",
	     L->As.Character
	    );
	    fprintf(Ebcscript_Name_Fplog, " Type: ");
	    Ebcscript_Type_print(L->TypeTree);
	    fprintf(Ebcscript_Name_Fplog, "\n");
	    break;
	  case EBCSCRIPT_LITERAL_KIND_STRING:
	    fprintf(Ebcscript_Name_Fplog,
	     " Kind: string\n"
	     " Value: %s\n",
	     L->As.String
	    );
	    fprintf(Ebcscript_Name_Fplog, " Type: ");
	    Ebcscript_Type_print(L->TypeTree);
	    fprintf(Ebcscript_Name_Fplog, "\n");
	    break;
	  defualt:
	    break;
	}
}
