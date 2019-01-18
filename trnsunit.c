/*******************************************************************************
   Project : C script
   File    : trnsunit.c
   Date    : 2018.6.26-
   Note    : 翻訳単位
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include "trnsunit.h"
#include "name.h"
#include "code.h"
#include "slist.h"
#include "hashmap.h"
#include "boolean.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Trnsunit_malloc(size_t Size);
void _Ebcscript_Trnsunit_free(void *P);
void _Ebcscript_Trnsunit_exit(int Status);
void _Ebcscript_Trnsunit_log(const char *Fmt, ...);

FILE *Ebcscript_Trnsunit_Fplog/* = stdout*/;
size_t Ebcscript_Trnsunit_MallocTotal = 0;
void *(*Ebcscript_Trnsunit_malloc)(size_t Size) = &_Ebcscript_Trnsunit_malloc;
void (*Ebcscript_Trnsunit_free)(void *) = &_Ebcscript_Trnsunit_free;
void (*Ebcscript_Trnsunit_exit)(int) = &_Ebcscript_Trnsunit_exit;
void (*Ebcscript_Trnsunit_log)(const char *Fmt, ...) =
                                                       &_Ebcscript_Trnsunit_log;


/*                                                                   関数定義 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Trnsunit_malloc(size_t Size)
{
	void *P;

	if ((P = malloc(Size + sizeof(size_t))) == NULL){
	  Ebcscript_Trnsunit_log(
	   "Ebcscript_Trnsunit_malloc(): "
	   "error: memory allocation error(total=%d bytes)\n",
	   Ebcscript_Trnsunit_MallocTotal);
	  Ebcscript_Trnsunit_exit(1);
	}
	*(size_t *)P = Size;
	Ebcscript_Trnsunit_MallocTotal += Size;
	return (size_t *)P + 1;
}

void _Ebcscript_Trnsunit_free(void *P)
{
	P = (size_t *)P - 1;
	Ebcscript_Trnsunit_MallocTotal -= *(size_t *)P;
	free(P);
}

void Ebcscript_Trnsunit_onexit(void)
{
	fclose(Ebcscript_Trnsunit_Fplog);
}

void _Ebcscript_Trnsunit_exit(int Status)
{
	Ebcscript_Trnsunit_onexit();
	exit(Status);
}

void _Ebcscript_Trnsunit_log(const char *Fmt, ...)
{
	va_list Args;

	fprintf(Ebcscript_Trnsunit_Fplog, "ebcscript_trnsunit: ");

	va_start(Args, Fmt);
	vfprintf(Ebcscript_Trnsunit_Fplog, Fmt, Args);
	va_end(Args);
}
/* -------------------------------------------------------------------------- */
ebcscript_trnsunit *Ebcscript_newTrnsunit(char *Filename)
{
	ebcscript_trnsunit *TU;

	TU = Ebcscript_Trnsunit_malloc(sizeof(ebcscript_trnsunit));
	TU->Filename = Ebcscript_Trnsunit_malloc(strlen(Filename) + 1);
	strcpy(TU->Filename, Filename);

	TU->CodeSize = EBCSCRIPT_TRNSUNIT_CODESIZE_INITIAL;
	TU->Code = Ebcscript_Trnsunit_malloc(TU->CodeSize);

	TU->NSVarFuncTypeEnum = newHashmap();
	TU->NSTag = newHashmap();

	TU->ConstTbl = newHashmap();
	TU->UnresolvedList = newSList();
	TU->AllocatedList = newSList();

	return TU;
}

void Ebcscript_deleteTrnsunit(ebcscript_trnsunit *TU)
{
	slist_cell *P;

	/* 変数に割り付けられたメモリの解放 */
	SList_clear(TU->AllocatedList, Ebcscript_Trnsunit_free);
	deleteSList(TU->AllocatedList);

	for (P = TU->UnresolvedList->Head.Next; P != NULL; P = P->Next) {
	  Ebcscript_deleteName(((ebcscript_unresolved *)P->Datum)->N);
	}
	SList_clear(TU->UnresolvedList,
	                          (void (*)(void *))Ebcscript_deleteUnresolved);
	deleteSList(TU->UnresolvedList);

	Hashmap_clear(TU->ConstTbl, (void (*)(void *))Ebcscript_deleteLiteral);
	deleteHashmap(TU->ConstTbl);

	Hashmap_clear(TU->NSVarFuncTypeEnum,
	                                (void (*)(void *))Ebcscript_deleteName);
	deleteHashmap(TU->NSVarFuncTypeEnum);
	Hashmap_clear(TU->NSTag, (void (*)(void *))Ebcscript_deleteName);
	deleteHashmap(TU->NSTag);

	Ebcscript_Trnsunit_free(TU->Code);

	Ebcscript_Trnsunit_free(TU->Filename);
	Ebcscript_Trnsunit_free(TU);
}

void *Ebcscript_Trnsunit_mallocG(ebcscript_trnsunit *TU, size_t Size)
{
	void *P;

	P = Ebcscript_Trnsunit_malloc(Size);
	SList_addFront(TU->AllocatedList, P);
	return P;
}

void *Ebcscript_Trnsunit_getCPOffset(ebcscript_trnsunit *TU)
{
	return (void *)(TU->CP - TU->Code);
}

static
void resizeCode(ebcscript_trnsunit *TU)
{
	void *New, *Old;
	int NewSize, OldSize;
	void *NewCP, *OldCP;
	ptrdiff_t Diff;

	Old = TU->Code;
	OldSize = TU->CodeSize;
	OldCP = TU->CP;

	New = Ebcscript_Trnsunit_malloc(OldSize * 2);
	NewSize = OldSize * 2;
	NewCP = OldCP + (Diff = New - Old);

	memcpy(New, Old, OldSize);

	Ebcscript_Trnsunit_free(Old);

	TU->Code = New;
	TU->CodeSize = NewSize;
	TU->CP = NewCP;
}

void Ebcscript_Trnsunit_store_instruction(ebcscript_trnsunit *TU,
                                                    ebcscript_instruction Inst)
{
	if (TU->CP + sizeof(ebcscript_instruction) >=
	                                              TU->Code + TU->CodeSize) {
	  resizeCode(TU);
	}
	*(ebcscript_instruction *)TU->CP = Inst;
	TU->CP += sizeof(ebcscript_instruction);
}

void Ebcscript_Trnsunit_store_address(ebcscript_trnsunit *TU, void *P)
{
	if (TU->CP + sizeof(void *) >=  TU->Code + TU->CodeSize) {
	  resizeCode(TU);
	}
	*(void **)TU->CP = P;
	TU->CP += sizeof(void *);
}

void Ebcscript_Trnsunit_store_address_n(ebcscript_trnsunit *TU,
                                                             ebcscript_name *N)
{
	enum ebcscript_name_addressing Addressing;
	void *Address;
	ebcscript_name *N1;

	switch (N->Kind) {
	  case EBCSCRIPT_NAME_KIND_LABEL:
	    Address    = N->As.Label.CodeAddress;
	    Addressing = N->As.Label.Addressing;
	    break;
	  case EBCSCRIPT_NAME_KIND_VARIABLE:
	    Address    = N->As.Variable.Address;
	    Addressing = N->As.Variable.Addressing;
	    break;
	  case EBCSCRIPT_NAME_KIND_FUNCTION:
	    Address    = N->As.Function.CodeAddress;
	    Addressing = N->As.Function.Addressing;
	    break;
	  case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	  case EBCSCRIPT_NAME_KIND_TYPEDEF:
	  case EBCSCRIPT_NAME_KIND_STRUCT:
	  case EBCSCRIPT_NAME_KIND_UNION:
	  case EBCSCRIPT_NAME_KIND_ENUMERATION:
	  default:
	    fprintf(Ebcscript_Trnsunit_Fplog, 
	     "Ebcscript_Trnsunit_store_address_n(): error: "
	     "an invalid kind of name\n"
	    );
	    break;
	}

	if (Addressing == EBCSCRIPT_NAME_ADDRESSING_UNDEFINED) {
	  N1 = Ebcscript_Name_dup(N);
		/* ローカルのexternなどブロックを出ると消えてしまうので */
	  SList_addFront(TU->UnresolvedList,
	              Ebcscript_newUnresolved((void *)(TU->CP - TU->Code), N1));
	  Ebcscript_Trnsunit_store_address(TU, NULL);
	}
	if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE) {
	  Ebcscript_Trnsunit_store_address(TU, Address);
	}
	if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ONSTACKFRAME) {
	  Ebcscript_Trnsunit_store_address(TU, Address);
	}
	if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ONCODE) {
	  N1 = Ebcscript_Name_dup(N);
	  SList_addFront(TU->UnresolvedList,
	              Ebcscript_newUnresolved((void *)(TU->CP - TU->Code), N1));
	  Ebcscript_Trnsunit_store_address(TU, NULL);
	}
}

void Ebcscript_Trnsunit_store_char(ebcscript_trnsunit *TU, char C)
{
	if (TU->CP + sizeof(char) >= TU->Code + TU->CodeSize) {
	  resizeCode(TU);
	}

	*(char *)TU->CP = C;
	TU->CP += sizeof(char);
}

void Ebcscript_Trnsunit_store_short(ebcscript_trnsunit *TU, short S)
{
	if (TU->CP + sizeof(short) >= TU->Code + TU->CodeSize) {
	  resizeCode(TU);
	}

	*(short *)TU->CP = S;
	TU->CP += sizeof(short);
}

void Ebcscript_Trnsunit_store_int(ebcscript_trnsunit *TU, int I)
{
	if (TU->CP + sizeof(int) >= TU->Code + TU->CodeSize) {
	  resizeCode(TU);
	}
	*(int *)TU->CP = I;
	TU->CP += sizeof(int);
}

void Ebcscript_Trnsunit_store_long(ebcscript_trnsunit *TU, long L)
{
	if (TU->CP + sizeof(long) >= TU->Code + TU->CodeSize) {
	  resizeCode(TU);
	}
	*(long *)TU->CP = L;
	TU->CP += sizeof(long);
}

void Ebcscript_Trnsunit_store_float(ebcscript_trnsunit *TU, float F)
{
	if (TU->CP + sizeof(float) >= TU->Code + TU->CodeSize) {
	  resizeCode(TU);
	}
	*(float *)TU->CP = F;
	TU->CP += sizeof(float);
}

void Ebcscript_Trnsunit_store_double(ebcscript_trnsunit *TU, double D)
{
	if (TU->CP + sizeof(double) >= TU->Code + TU->CodeSize) {
	  resizeCode(TU);
	}
	*(double *)TU->CP = D;
	TU->CP += sizeof(double);
}

boolean Ebcscript_Trnsunit_resolve(ebcscript_trnsunit *TU)
{
	ebcscript_unresolved *UR;
	ebcscript_name **N0, *N;
	void *Address;
	int Addressing, Linkage;
	slist_cell *P, *Q;
	boolean B = true;

	/* 未解決箇所の指定方法を相対アドレスから絶対アドレスへ */
	for (P = TU->UnresolvedList->Head.Next;
	     P != NULL;
	     P = P->Next) {
	  UR = (ebcscript_unresolved *)P->Datum;
	  UR->CP += (ptrdiff_t)TU->Code;
	}

	/* 名前表に登録されている関数のアドレスを絶対アドレスにする */
	Hashmap_foreach(TU->NSVarFuncTypeEnum, N, {
	  switch (N->Kind) {
	    case EBCSCRIPT_NAME_KIND_FUNCTION:
	      if (N->As.Function.Addressing ==
	                                     EBCSCRIPT_NAME_ADDRESSING_ONCODE) {
	        N->As.Function.CodeAddress += (ptrdiff_t)TU->Code;
	        N->As.Function.Addressing = EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE;
	      }
	      break;
	    case EBCSCRIPT_NAME_KIND_VARIABLE:
	    case EBCSCRIPT_NAME_KIND_TYPEDEF:
	    case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	    default:
	      break;
	  }
	})

	/* 未解決リストに対して */
	/* 参照時点でAddressingがUNDEFINEDかONCODEのものがリストに載っている。
	   その中で、パース終了時点でUNDEFINEDが解消されたものが
	   バックパッチ対象となる。 */
	for (Q = &TU->UnresolvedList->Head, P = Q->Next;
	     P != NULL;
	     Q = P, P = P->Next) {
	  UR = (ebcscript_unresolved *)P->Datum;

	  /* 無ければそのままにしておく */
	  /* ローカルでextern宣言されているが、グローバルにない場合等 */
	  if (!(N0 = (ebcscript_name **)Hashmap_find(
	                           TU->NSVarFuncTypeEnum, UR->N->Identifier))) {
	    continue;
	  }

	  /* ローカルでextern宣言されていて、グローバルにある場合等 */
	  /* 名前の種類 */
	  if ((*N0)->Kind != UR->N->Kind) {
	    Ebcscript_Trnsunit_log(
	     "Ebcscript_Trnsunit_resolve(): error: "
	     "\'%s\' redeclared as different kind of symbol\n",
	     UR->N->Identifier);
	    B &= false;
	    continue;
	  }

	  /* 型 */
	  switch ((*N0)->Kind) {
	    case EBCSCRIPT_NAME_KIND_VARIABLE:
	      if (!Ebcscript_Type_equals((*N0)->As.Variable.TypeTree,
	                                         UR->N->As.Variable.TypeTree)) {
	        Ebcscript_Trnsunit_log(
	         "Ebcscript_Trnsunit_resolve(): error: "
	         "conflicting types for \'%s\'\n",
	         UR->N->Identifier);
	        B &= false;
	        continue;
	      }
	      break;
	    case EBCSCRIPT_NAME_KIND_FUNCTION:
	      if (!Ebcscript_Type_equals((*N0)->As.Function.TypeTree,
	                                         UR->N->As.Function.TypeTree)) {
	        Ebcscript_Trnsunit_log(
	         "Ebcscript_Trnsunit_resolve(): error: "
	         "conflicting types for \'%s\'\n",
	         UR->N->Identifier);
	        B &= false;
	        continue;
	      }
	      break;
	    case EBCSCRIPT_NAME_KIND_TYPEDEF:
	    case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	    default:
	      B &= false;
	      continue;
	  }

	  switch ((*N0)->Kind) {
	    case EBCSCRIPT_NAME_KIND_VARIABLE:
	      Address    = (*N0)->As.Variable.Address;
	      Addressing = (*N0)->As.Variable.Addressing;
	      break;
	    case EBCSCRIPT_NAME_KIND_FUNCTION:
	      Address    = (*N0)->As.Function.CodeAddress;
	      Addressing = (*N0)->As.Function.Addressing;
	      break;
	    case EBCSCRIPT_NAME_KIND_TYPEDEF:
	    case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	    default:
	      B &= false;
	      break;
	  }

	  if (Addressing == EBCSCRIPT_NAME_ADDRESSING_UNDEFINED) {
	    /* extern宣言なのでそのまま */
	    continue;
	  }
	  if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE) {
	    *(void **)UR->CP = Address;

	    /* リストから削除 */
	    Ebcscript_deleteName(UR->N);
	    Ebcscript_deleteUnresolved(UR);
	    Q->Next = P->Next; deleteSListCell(P);

	    P = Q;
	    continue;
	  }
	  if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ONSTACKFRAME) {
	    /* ファイルスコープでONSTACKFRAMEはない。来ないはず */
	    B &= false;
	    continue;
	  }
	  if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ONCODE) {
	    /* 上で絶対アドレスにしているので来ないはず */
	    B &= false;
	    continue;
	  }
	}
	return B;
}

void Ebcscript_Trnsunit_dumpCode(ebcscript_trnsunit *TU)
{
	void *CP;

	CP = TU->Code;
	fprintf(Ebcscript_Trnsunit_Fplog, "--- Code\n");
	for (CP = TU->Code; CP < TU->CP;) {
	  fprintf(Ebcscript_Trnsunit_Fplog, "%p : ", CP);

	  switch (*(ebcscript_instruction *)CP) {

#define case_op_0(Op)							\
	    case EBCSCRIPT_INSTRUCTION_ ## Op:				\
	      fprintf(Ebcscript_Trnsunit_Fplog, #Op);			\
	      CP += sizeof(ebcscript_instruction);			\
	      break;

#define case_op_1(Op, Type)						\
	    case EBCSCRIPT_INSTRUCTION_ ## Op:				\
	      fprintf(Ebcscript_Trnsunit_Fplog, #Op);			\
	      CP += sizeof(ebcscript_instruction);			\
	      fprintf(Ebcscript_Trnsunit_Fplog, " %p", *(Type *)CP);	\
	      CP += sizeof(Type);					\
	      break;

#define case_op_2(Op, Type1, Type2)					\
	    case EBCSCRIPT_INSTRUCTION_ ## Op:				\
	      fprintf(Ebcscript_Trnsunit_Fplog, #Op);			\
	      CP += sizeof(ebcscript_instruction);			\
	      fprintf(Ebcscript_Trnsunit_Fplog, " %p", *(Type1 *)CP);	\
	      CP += sizeof(Type1);					\
	      fprintf(Ebcscript_Trnsunit_Fplog, " %p", *(Type2 *)CP);	\
	      CP += sizeof(Type2);					\
	      break;

#define case_op_0_numeric_ptr(Op)					\
	    case_op_0(Op ## _C)						\
	    case_op_0(Op ## _S)						\
	    case_op_0(Op ## _I)						\
	    case_op_0(Op ## _L)						\
	    case_op_0(Op ## _UC)					\
	    case_op_0(Op ## _US)					\
	    case_op_0(Op ## _UI)					\
	    case_op_0(Op ## _UL)					\
	    case_op_0(Op ## _F)						\
	    case_op_0(Op ## _D)						\
	    case_op_0(Op ## _P)

#define case_op_0_numeric(Op)						\
	    case_op_0(Op ## _C)						\
	    case_op_0(Op ## _S)						\
	    case_op_0(Op ## _I)						\
	    case_op_0(Op ## _L)						\
	    case_op_0(Op ## _UC)					\
	    case_op_0(Op ## _US)					\
	    case_op_0(Op ## _UI)					\
	    case_op_0(Op ## _UL)					\
	    case_op_0(Op ## _F)						\
	    case_op_0(Op ## _D)

#define case_op_0_integer(Op)						\
	    case_op_0(Op ## _C)						\
	    case_op_0(Op ## _S)						\
	    case_op_0(Op ## _I)						\
	    case_op_0(Op ## _L)						\
	    case_op_0(Op ## _UC)					\
	    case_op_0(Op ## _US)					\
	    case_op_0(Op ## _UI)					\
	    case_op_0(Op ## _UL)

#define case_op_0_bit(Op)						\
	    case_op_0(Op ## _C)						\
	    case_op_0(Op ## _S)						\
	    case_op_0(Op ## _I)						\
	    case_op_0(Op ## _L)

	    case_op_0(EXIT)
	    case_op_0(NOP)
	    case_op_0(PUSH_BP)
	    case_op_0(POP_BP)
	    case_op_0(ADD_SP)
	    case_op_0(SUB_SP)
	    case_op_0(MOV_SP_BP)
	    case_op_0(MOV_BP_SP)

	    /* スタック操作 */
	    case_op_1(PUSH_IMM_C, char)
	    case_op_1(PUSH_IMM_S, short)
	    case_op_1(PUSH_IMM_I, int)
	    case_op_1(PUSH_IMM_L, long)
	    case_op_1(PUSH_IMM_F, float)
	    case_op_1(PUSH_IMM_D, double)
	    case_op_1(PUSH_IMM_P, void *)
	    case_op_2(PUSH_IMM_OBJECT, void *, int)

	    case_op_1(PUSH_GMEM_C, void *)
	    case_op_1(PUSH_GMEM_S, void *)
	    case_op_1(PUSH_GMEM_I, void *)
	    case_op_1(PUSH_GMEM_L, void *)
	    case_op_1(PUSH_GMEM_F, void *)
	    case_op_1(PUSH_GMEM_D, void *)
	    case_op_1(PUSH_GMEM_P, void *)
	    case_op_2(PUSH_GMEM_OBJECT, void *, int)

	    case_op_1(PUSH_LMEM_C, void *)
	    case_op_1(PUSH_LMEM_S, void *)
	    case_op_1(PUSH_LMEM_I, void *)
	    case_op_1(PUSH_LMEM_L, void *)
	    case_op_1(PUSH_LMEM_F, void *)
	    case_op_1(PUSH_LMEM_D, void *)
	    case_op_1(PUSH_LMEM_P, void *)
	    case_op_2(PUSH_LMEM_OBJECT, void *, int)

	    case_op_0(DUP_C)
	    case_op_0(DUP_S)
	    case_op_0(DUP_I)
	    case_op_0(DUP_L)
	    case_op_0(DUP_F)
	    case_op_0(DUP_D)
	    case_op_0(DUP_P)
	    case_op_1(DUP_OBJECT, int)

	    case_op_0(LOAD_C)
	    case_op_0(LOAD_S)
	    case_op_0(LOAD_I)
	    case_op_0(LOAD_L)
	    case_op_0(LOAD_F)
	    case_op_0(LOAD_D)
	    case_op_0(LOAD_P)
	    case_op_1(LOAD_OBJECT, int)

	    case_op_1(POP_GMEM_C, void *)
	    case_op_1(POP_GMEM_S, void *)
	    case_op_1(POP_GMEM_I, void *)
	    case_op_1(POP_GMEM_L, void *)
	    case_op_1(POP_GMEM_F, void *)
	    case_op_1(POP_GMEM_D, void *)
	    case_op_1(POP_GMEM_P, void *)
	    case_op_2(POP_GMEM_OBJECT, void *, int)

	    case_op_1(POP_LMEM_C, void *)
	    case_op_1(POP_LMEM_S, void *)
	    case_op_1(POP_LMEM_I, void *)
	    case_op_1(POP_LMEM_L, void *)
	    case_op_1(POP_LMEM_F, void *)
	    case_op_1(POP_LMEM_D, void *)
	    case_op_1(POP_LMEM_P, void *)
	    case_op_2(POP_LMEM_OBJECT, void *, int)

	    case_op_0(STORE_C)
	    case_op_0(STORE_S)
	    case_op_0(STORE_I)
	    case_op_0(STORE_L)
	    case_op_0(STORE_F)
	    case_op_0(STORE_D)
	    case_op_0(STORE_P)
	    case_op_1(STORE_OBJECT, int)

	    /* 型キャスト */
	    case_op_0_numeric_ptr(C_TO)
	    case_op_0_numeric_ptr(UC_TO)
	    case_op_0_numeric_ptr(S_TO)
	    case_op_0_numeric_ptr(US_TO)
	    case_op_0_numeric_ptr(I_TO)
	    case_op_0_numeric_ptr(UI_TO)
	    case_op_0_numeric_ptr(L_TO)
	    case_op_0_numeric_ptr(UL_TO)
	    case_op_0_numeric_ptr(F_TO)
	    case_op_0_numeric_ptr(D_TO)
	    case_op_0_numeric_ptr(P_TO)

	    /* 算術演算 */
	    case_op_0_numeric_ptr(ADD)
	    case_op_0_numeric_ptr(SUB)
	    case_op_0_numeric(MUL)
	    case_op_0_numeric(DIV)
	    case_op_0_integer(MOD)
	    case_op_0_numeric_ptr(INC)
	    case_op_0_numeric_ptr(DEC)
	    case_op_0_numeric(NEG)

	    /* ビット演算 */
	    case_op_0_bit(AND)
	    case_op_0_bit(OR)
	    case_op_0_bit(XOR)
	    case_op_0_bit(NOT)
	    case_op_0_bit(SHL)
	    case_op_0_bit(SHR)

	    /* 比較演算 */
	    case_op_0_numeric_ptr(LT)
	    case_op_0_numeric_ptr(LE)
	    case_op_0_numeric_ptr(GT)
	    case_op_0_numeric_ptr(GE)
	    case_op_0_numeric_ptr(EQ)
	    case_op_0_numeric_ptr(NE)

	    /* 代入式 */
	    case_op_0_numeric_ptr(ASSGN)
	    case_op_1(ASSGN_OBJECT, int)

	    /* 局所ジャンプ */
	    case_op_1(JMP,  void *)

	    case_op_1(JMPT_C,  void *)
	    case_op_1(JMPT_UC, void *)
	    case_op_1(JMPT_S,  void *)
	    case_op_1(JMPT_US, void *)
	    case_op_1(JMPT_I,  void *)
	    case_op_1(JMPT_UI, void *)
	    case_op_1(JMPT_L,  void *)
	    case_op_1(JMPT_UL, void *)
	    case_op_1(JMPT_F,  void *)
	    case_op_1(JMPT_D,  void *)
	    case_op_1(JMPT_P,  void *)

	    case_op_1(JMPF_C,  void *)
	    case_op_1(JMPF_UC, void *)
	    case_op_1(JMPF_S,  void *)
	    case_op_1(JMPF_US, void *)
	    case_op_1(JMPF_I,  void *)
	    case_op_1(JMPF_UI, void *)
	    case_op_1(JMPF_L,  void *)
	    case_op_1(JMPF_UL, void *)
	    case_op_1(JMPF_F,  void *)
	    case_op_1(JMPF_D,  void *)
	    case_op_1(JMPF_P,  void *)

	    /* 呼び出し・復帰 */
	    case_op_1(CALL_IMM,  void *)
	    case_op_0(CALL_PTR)
	    case_op_0(RET)

	    default:
	      fprintf(Ebcscript_Trnsunit_Fplog,
	       "\?\?\?(%d)", *(ebcscript_instruction *)CP);
	      CP += sizeof(ebcscript_instruction);
	      break;
	  }
	  fprintf(Ebcscript_Trnsunit_Fplog, "\n");
	}
#undef case_op_0
#undef case_op_0_numeric_ptr
#undef case_op_0_numeric
#undef case_op_0_integer
#undef case_op_0_bit
#undef case_op_1
#undef case_op_2
}

void Ebcscript_Trnsunit_dumpUnresolvedList(ebcscript_trnsunit *TU)
{
	slist_cell *P;
	ebcscript_unresolved *UR;

	fprintf(Ebcscript_Trnsunit_Fplog, "--- Unresolved List\n");
	for (P = TU->UnresolvedList->Head.Next;
	     P != NULL;
	     P = P->Next) {
	  UR = (ebcscript_unresolved *)P->Datum;
	  fprintf(Ebcscript_Trnsunit_Fplog,
	   "CP, Name: %p, %s\n",
	   (ptrdiff_t)UR->CP + TU->Code, UR->N->Identifier);
	}
	fprintf(Ebcscript_Trnsunit_Fplog, "\n");
}

void Ebcscript_Trnsunit_dumpConstTbl(ebcscript_trnsunit *TU)
{
	ebcscript_literal *L;

	fprintf(Ebcscript_Trnsunit_Fplog, "--- Constant Table\n");
	Hashmap_foreach(TU->ConstTbl, L, {
	  Ebcscript_Literal_print(L);
	})
}

void Ebcscript_Trnsunit_dumpNS(ebcscript_trnsunit *TU)
{
	ebcscript_name *N;

	fprintf(Ebcscript_Trnsunit_Fplog, "--- Name Table\n");
	Hashmap_foreach(TU->NSVarFuncTypeEnum, N, {
	  Ebcscript_Name_print(N);
	})
}

void Ebcscript_Trnsunit_dump(ebcscript_trnsunit *TU)
{
	Ebcscript_Trnsunit_dumpNS(TU);
	Ebcscript_Trnsunit_dumpConstTbl(TU);
	Ebcscript_Trnsunit_dumpCode(TU);
	Ebcscript_Trnsunit_dumpUnresolvedList(TU);
}

ebcscript_name **Ebcscript_Trnsunit_findVarFuncTypeEnum_global(
                                             ebcscript_trnsunit *TU, char *Key)
{
	ebcscript_name **N0;
	int Linkage, Addressing;

	if (!(N0 = (ebcscript_name **)Hashmap_find(TU->NSVarFuncTypeEnum,
	                                                                Key))) {
	  return NULL;
	}

	/* static変数・関数、及びextern変数・関数は無視する */
	switch ((*N0)->Kind) {
	  case EBCSCRIPT_NAME_KIND_VARIABLE:
	    Linkage    = (*N0)->As.Variable.Linkage;
	    Addressing = (*N0)->As.Variable.Addressing;
	    break;
	  case EBCSCRIPT_NAME_KIND_FUNCTION:
	    Linkage    = (*N0)->As.Function.Linkage;
	    Addressing = (*N0)->As.Function.Addressing;
	    break;
	  case EBCSCRIPT_NAME_KIND_TYPEDEF:
	  case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	  default:
	    Linkage    = -1;
	    Addressing = -1;
	    break;
	}
	if (Linkage == EBCSCRIPT_NAME_LINKAGE_INTERNAL) {
	  return NULL;
	}
	if (Addressing == EBCSCRIPT_NAME_ADDRESSING_UNDEFINED) {
	  return NULL;
	}

	return N0;
}

/* -------------------------------------------------------------------------- */
#ifdef DEBUG
int main(int argc, char *argv[])
{
	return 0;
}
#endif
