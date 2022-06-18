/*******************************************************************************
   Project : C Script
   File    : ebcscrip.c
   Date    : 2018.4.5
   Note    : 
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include "ebcscrip.h"
#include "parser.h"
#include "trnsunit.h"
#include "decl.h"
#include "expr.h"
#include "init.h"
#include "stmt.h"
#include "name.h"
#include "code.h"
#include "functbl.h"
#include "hashmap.h"
#include "slist.h"
#include "btree.h"
#include "boolean.h"
/*
#define DEBUG
*/
/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_malloc(size_t Size);
void _Ebcscript_free(void *P);
void _Ebcscript_exit(int Status);
void _Ebcscript_log(const char *Fmt, ...);

FILE *Ebcscript_Fplog/* = stdout*/;
size_t Ebcscript_MallocTotal = 0;
void *(*Ebcscript_malloc)(size_t Size) = &_Ebcscript_malloc;
void (*Ebcscript_free)(void *) = &_Ebcscript_free;
void (*Ebcscript_exit)(int) = &_Ebcscript_exit;
void (*Ebcscript_log)(const char *Fmt, ...) = &_Ebcscript_log;

/*                                                                   関数定義 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_malloc(size_t Size)
{
	void *P;

	if ((P = malloc(Size + sizeof(size_t))) == NULL){
	  Ebcscript_log(
	   "Ebcscript_malloc(): "
	   "error: memory allocation error(total=%d bytes)\n",
	   Ebcscript_MallocTotal);
	  Ebcscript_exit(1);
	}
	*(size_t *)P = Size;
	Ebcscript_MallocTotal += Size;
	return (size_t *)P + 1;
}

void _Ebcscript_free(void *P)
{
	P = (size_t *)P - 1;
	Ebcscript_MallocTotal -= *(size_t *)P;
	free(P);
}

void Ebcscript_onexit(void)
{
	fclose(Ebcscript_Fplog);
}

void _Ebcscript_exit(int Status)
{
	Ebcscript_onexit();
	exit(Status);
}

void _Ebcscript_log(const char *Fmt, ...)
{
	va_list Args;

	fprintf(Ebcscript_Fplog, "ebcscript: ");

	va_start(Args, Fmt);
	vfprintf(Ebcscript_Fplog, Fmt, Args);
	va_end(Args);
}
/* -------------------------------------------------------------------------- */
void Ebcscript_push_char(ebcscript *Env, char C)
{
	Ebcscript_Stack_push_char(Env->Stack, C);
}

void Ebcscript_push_short(ebcscript *Env, short S)
{
	Ebcscript_Stack_push_short(Env->Stack, S);
}

void Ebcscript_push_int(ebcscript *Env, int I)
{
	Ebcscript_Stack_push_int(Env->Stack, I);
}

void Ebcscript_push_long(ebcscript *Env, long L)
{
	Ebcscript_Stack_push_long(Env->Stack, L);
}

void Ebcscript_push_uchar(ebcscript *Env, unsigned char C)
{
	Ebcscript_Stack_push_uchar(Env->Stack, C);
}

void Ebcscript_push_ushort(ebcscript *Env, unsigned short S)
{
	Ebcscript_Stack_push_ushort(Env->Stack, S);
}

void Ebcscript_push_uint(ebcscript *Env, unsigned int I)
{
	Ebcscript_Stack_push_uint(Env->Stack, I);
}

void Ebcscript_push_ulong(ebcscript *Env, unsigned long L)
{
	Ebcscript_Stack_push_ulong(Env->Stack, L);
}

void Ebcscript_push_float(ebcscript *Env, float F)
{
	Ebcscript_Stack_push_float(Env->Stack, F);
}

void Ebcscript_push_double(ebcscript *Env, double D)
{
	Ebcscript_Stack_push_double(Env->Stack, D);
}

void Ebcscript_push_address(ebcscript *Env, void *P)
{
	Ebcscript_Stack_push_address(Env->Stack, P);
}

void Ebcscript_pop_char(ebcscript *Env, char *C)
{
	Ebcscript_Stack_pop_char(Env->Stack, C);
}

void Ebcscript_pop_short(ebcscript *Env, short *S)
{
	Ebcscript_Stack_pop_short(Env->Stack, S);
}

void Ebcscript_pop_int(ebcscript *Env, int *I)
{
	Ebcscript_Stack_pop_int(Env->Stack, I);
}

void Ebcscript_pop_long(ebcscript *Env, long *L)
{
	Ebcscript_Stack_pop_long(Env->Stack, L);
}

void Ebcscript_pop_uchar(ebcscript *Env, unsigned char *C)
{
	Ebcscript_Stack_pop_uchar(Env->Stack, C);
}

void Ebcscript_pop_ushort(ebcscript *Env, unsigned short *S)
{
	Ebcscript_Stack_pop_ushort(Env->Stack, S);
}

void Ebcscript_pop_uint(ebcscript *Env, unsigned int *I)
{
	Ebcscript_Stack_pop_uint(Env->Stack, I);
}

void Ebcscript_pop_ulong(ebcscript *Env, unsigned long *L)
{
	Ebcscript_Stack_pop_ulong(Env->Stack, L);
}

void Ebcscript_pop_float(ebcscript *Env, float *F)
{
	Ebcscript_Stack_pop_float(Env->Stack, F);
}

void Ebcscript_pop_double(ebcscript *Env, double *D)
{
	Ebcscript_Stack_pop_double(Env->Stack, D);
}

void Ebcscript_pop_address(ebcscript *Env, void **P)
{
	Ebcscript_Stack_pop_address(Env->Stack, P);
}

void Ebcscript_add_sp(ebcscript *Env, int I)
{
	Ebcscript_Stack_add_sp(Env->Stack, I);
}

void Ebcscript_sub_sp(ebcscript *Env, int I)
{
	Ebcscript_Stack_sub_sp(Env->Stack, I);
}

static
void Ebcscript_execute(ebcscript *Env)
{
	typedef void * address;	/* マクロ内でpush, pop関数名を生成するため */
	typedef unsigned char uchar;
	typedef unsigned short ushort;
	typedef unsigned int uint;
	typedef unsigned long ulong;

	ebcscript_instruction Inst;
	char C, C1, C2;
	short S, S1, S2;
	int I, I1, I2;
	long L, L1, L2;
	uchar UC, UC1, UC2;
	ushort US, US1, US2;
	uint UI, UI1, UI2;
	ulong UL, UL1, UL2;
	float F, F1, F2;
	double D, D1, D2;
	address P, P1, P2;
	void *Q;

#define CP (Env->CP)
#define SP (Env->Stack->Pointer)
#define BP (Env->BP)

	Inst = *(ebcscript_instruction *)CP;
	CP += sizeof(ebcscript_instruction);
	switch (Inst) {
	  case EBCSCRIPT_INSTRUCTION_EXIT:
	    break;

	  case EBCSCRIPT_INSTRUCTION_NOP:
	    break;

	  case EBCSCRIPT_INSTRUCTION_PUSH_BP:
	    Ebcscript_push_address(Env, BP);
	    break;

	  case EBCSCRIPT_INSTRUCTION_POP_BP:
	    Ebcscript_pop_address(Env, &BP);
	    break;

	  case EBCSCRIPT_INSTRUCTION_ADD_SP:
	    Ebcscript_pop_int(Env, &I);
	    Ebcscript_add_sp(Env, I);
	    break;

	  case EBCSCRIPT_INSTRUCTION_SUB_SP:
	    Ebcscript_pop_int(Env, &I);
	    Ebcscript_sub_sp(Env, I);
	    break;

	  case EBCSCRIPT_INSTRUCTION_MOV_SP_BP:
	    BP = SP;
	    break;

	  case EBCSCRIPT_INSTRUCTION_MOV_BP_SP:
	    SP = BP;
	    break;

	  /* スタック操作 */
#define case_push_imm(T, Type)						\
	  case EBCSCRIPT_INSTRUCTION_PUSH_IMM_ ## T:			\
	    Ebcscript_push_ ## Type(Env, *(Type *)CP);			\
	    CP += sizeof(Type);						\
/*	    SP -= sizeof(Type);						\
	    *(Type *)SP = *(Type *)CP;					\
	    CP += sizeof(Type);	*/					\
	    break;

#define case_push_gmem(T, Type)						\
	  case EBCSCRIPT_INSTRUCTION_PUSH_GMEM_ ## T:			\
	    Ebcscript_push_ ## Type(Env, *(Type *)(*(void **)CP));	\
	    CP += sizeof(void *);					\
/*	    SP -= sizeof(Type);						\
	    *(Type *)SP = *(Type *)(*(void **)CP);			\
	    CP += sizeof(void *);	*/				\
	    break;

#define case_push_lmem(T, Type)						\
	  case EBCSCRIPT_INSTRUCTION_PUSH_LMEM_ ## T:			\
	    Ebcscript_push_ ## Type(Env,				\
	                              *(Type *)(BP + *(ptrdiff_t *)CP));\
	    CP += sizeof(ptrdiff_t);					\
/*	    SP -= sizeof(Type);						\
	    *(Type *)SP = *(Type *)(BP + (long)*(void **)CP);		\
	    CP += sizeof(void *);	*/				\
	    break;

#define case_pop_gmem(T, Type)						\
	  case EBCSCRIPT_INSTRUCTION_POP_GMEM_ ## T:			\
	    Ebcscript_pop_ ## Type(Env, (Type *)(*(void **)CP));	\
	    CP += sizeof(void *);					\
/*	    *(Type *)(*(void **)CP) = *(Type *)SP;			\
	    SP += sizeof(Type);						\
	    CP += sizeof(void *);	*/				\
	    break;

#define case_pop_lmem(T, Type)						\
	  case EBCSCRIPT_INSTRUCTION_POP_LMEM_ ## T:			\
	    Ebcscript_pop_ ## Type(Env,					\
	                               (Type *)(BP + *(ptrdiff_t *)CP));\
	    CP += sizeof(ptrdiff_t);					\
/*	    *(Type *)(BP + (long)*(void **)CP) = *(Type *)SP;		\
	    SP += sizeof(Type);						\
	    CP += sizeof(void *);	*/				\
	    break;

#define case_load(T, Type)						\
	  case EBCSCRIPT_INSTRUCTION_LOAD_ ## T:			\
	    Ebcscript_pop_address(Env, &Q);				\
	    Ebcscript_push_ ## Type(Env, *(Type *)Q);			\
/*	    Q = *(void **)SP;						\
	    SP += sizeof(void *);					\
	    SP -= sizeof(Type);						\
	    *(Type *)SP = *(Type *)Q;	*/				\
	    break;

#define case_store(T, Type)						\
	  case EBCSCRIPT_INSTRUCTION_STORE_ ## T:			\
	    Ebcscript_pop_address(Env, &Q);				\
	    Ebcscript_pop_ ## Type(Env, &(T));				\
	    *(Type *)Q = T;						\
/*	    Q = *(void **)SP;						\
	    SP += sizeof(void *);					\
	    T = *(Type *)SP;						\
	    SP += sizeof(Type);						\
	    *(Type *)Q = T;	*/					\
	    break;

#define case_dup(T, Type)						\
	  case EBCSCRIPT_INSTRUCTION_DUP_ ## T:				\
	    Q = SP;							\
	    Ebcscript_push_ ## Type(Env, *(Type *)Q);			\
/*	    Q = SP;							\
	    SP -= sizeof(Type);						\
	    *(Type *)SP = *(Type *)Q;	*/				\
	    break;

	  case_push_imm(C, char)
	  case_push_imm(S, short)
	  case_push_imm(I, int)
	  case_push_imm(L, long)
	  case_push_imm(F, float)
	  case_push_imm(D, double)
	  case_push_imm(P, address)

	  case_push_gmem(C, char)
	  case_push_gmem(S, short)
	  case_push_gmem(I, int)
	  case_push_gmem(L, long)
	  case_push_gmem(F, float)
	  case_push_gmem(D, double)
	  case_push_gmem(P, address)

	  case_push_lmem(C, char)
	  case_push_lmem(S, short)
	  case_push_lmem(I, int)
	  case_push_lmem(L, long)
	  case_push_lmem(F, float)
	  case_push_lmem(D, double)
	  case_push_lmem(P, address)

	  case_pop_gmem(C, char)
	  case_pop_gmem(S, short)
	  case_pop_gmem(I, int)
	  case_pop_gmem(L, long)
	  case_pop_gmem(F, float)
	  case_pop_gmem(D, double)
	  case_pop_gmem(P, address)

	  case_pop_lmem(C, char)
	  case_pop_lmem(S, short)
	  case_pop_lmem(I, int)
	  case_pop_lmem(L, long)
	  case_pop_lmem(F, float)
	  case_pop_lmem(D, double)
	  case_pop_lmem(P, address)

	  case_load(C, char)
	  case_load(S, short)
	  case_load(I, int)
	  case_load(L, long)
	  case_load(F, float)
	  case_load(D, double)
	  case_load(P, address)

	  case_store(C, char)
	  case_store(S, short)
	  case_store(I, int)
	  case_store(L, long)
	  case_store(F, float)
	  case_store(D, double)
	  case_store(P, address)

	  case_dup(C, char)
	  case_dup(S, short)
	  case_dup(I, int)
	  case_dup(L, long)
	  case_dup(F, float)
	  case_dup(D, double)
	  case_dup(P, address)

	  case EBCSCRIPT_INSTRUCTION_PUSH_IMM_OBJECT:
	    /* 使用しない命令 */
	    break;

	  case EBCSCRIPT_INSTRUCTION_PUSH_GMEM_OBJECT:
	    I = *(int *)CP;	/* サイズ */
	    CP += sizeof(int);
	    P = *(void **)CP;
	    CP += sizeof(void *);
	    if (SP - I < Env->Stack->Head) {
	      Ebcscript_log("Ebcscript_execute(): error: stack overflow\n");
	      Ebcscript_exit(1);
	    }
	    SP -= I;
	    memcpy(SP, P, I);
	    break;

	  case EBCSCRIPT_INSTRUCTION_PUSH_LMEM_OBJECT:
	    I = *(int *)CP;	/* サイズ */
	    CP += sizeof(int);
	    P = BP + *(ptrdiff_t *)CP;
	    CP += sizeof(ptrdiff_t);
	    if (SP - I < Env->Stack->Head) {
	      Ebcscript_log("Ebcscript_execute(): error: stack overflow\n");
	      Ebcscript_exit(1);
	    }
	    SP -= I;
	    memcpy(SP, P, I);
	    break;

	  case EBCSCRIPT_INSTRUCTION_POP_GMEM_OBJECT:
	    I = *(int *)CP;	/* サイズ */
	    CP += sizeof(int);
	    P = *(void **)CP;
	    CP += sizeof(void *);
	    if (SP + I > Env->Stack->Head + Env->Stack->Size) {
	      Ebcscript_log("Ebcscript_execute(): error: stack overrun\n");
	      Ebcscript_exit(1);
	    }
	    memcpy(P, SP, I);
	    SP += I;
	    break;

	  case EBCSCRIPT_INSTRUCTION_POP_LMEM_OBJECT:
	    I = *(int *)CP;	/* サイズ */
	    CP += sizeof(int);
	    P = BP + *(ptrdiff_t *)CP;
	    CP += sizeof(void *);
	    if (SP + I > Env->Stack->Head + Env->Stack->Size) {
	      Ebcscript_log("Ebcscript_execute(): error: stack overrun\n");
	      Ebcscript_exit(1);
	    }
	    memcpy(P, SP, I);
	    SP += I;
	    break;

	  case EBCSCRIPT_INSTRUCTION_LOAD_OBJECT:
	    I = *(int *)CP;	/* サイズ */
	    CP += sizeof(int);
	    Ebcscript_pop_address(Env, &P);
	    if (SP - I < Env->Stack->Head) {
	      Ebcscript_log("Ebcscript_execute(): error: stack overflow\n");
	      Ebcscript_exit(1);
	    }
	    SP -= I;
	    memcpy(SP, P, I);
	    break;

	  case EBCSCRIPT_INSTRUCTION_STORE_OBJECT:
	    I = *(int *)CP;	/* サイズ */
	    CP += sizeof(int);
	    Ebcscript_pop_address(Env, &P);
	    if (SP + I > Env->Stack->Head + Env->Stack->Size) {
	      Ebcscript_log("Ebcscript_execute(): error: stack overrun\n");
	      Ebcscript_exit(1);
	    }
	    memcpy(P, SP, I);
	    SP += I;
	    break;

	  case EBCSCRIPT_INSTRUCTION_DUP_OBJECT:
	    I = *(int *)CP;	/* サイズ */
	    CP += sizeof(int);
	    P = SP;
	    if (SP - I < Env->Stack->Head) {
	      Ebcscript_log("Ebcscript_execute(): error: stack overflow\n");
	      Ebcscript_exit(1);
	    }
	    SP -= I;
	    memcpy(SP, P, I);
	    break;

	  /* 型キャスト */
#define case_t1_to_t2(T1, Type1, T2, Type2)				\
	  case EBCSCRIPT_INSTRUCTION_## T1 ## _TO_ ## T2:		\
	    Ebcscript_pop_ ## Type1(Env, &(T1));			\
	    Ebcscript_push_ ## Type2(Env, (Type2)T1);			\
/*	    T1 = *(Type1 *)SP;						\
	    SP += sizeof(Type1);					\
	    SP -= sizeof(Type2);					\
	    *(Type2 *)SP = (Type2)T1;	*/				\
	    break;

#define case_t1_to_(T1, Type1)						\
	  case_t1_to_t2(T1, Type1, C,  char)				\
	  case_t1_to_t2(T1, Type1, S,  short)				\
	  case_t1_to_t2(T1, Type1, I,  int)				\
	  case_t1_to_t2(T1, Type1, L,  long)				\
	  case_t1_to_t2(T1, Type1, UC, uchar)				\
	  case_t1_to_t2(T1, Type1, US, ushort)				\
	  case_t1_to_t2(T1, Type1, UI, uint)				\
	  case_t1_to_t2(T1, Type1, UL, ulong)				\
	  case_t1_to_t2(T1, Type1, F,  float)				\
	  case_t1_to_t2(T1, Type1, D,  double)

	  case_t1_to_(C, char)
	  case_t1_to_(S, short)
	  case_t1_to_(I, int)
	  case_t1_to_(L, long)
	  case_t1_to_(UC, uchar)
	  case_t1_to_(US, ushort)
	  case_t1_to_(UI, uint)
	  case_t1_to_(UL, ulong)
	  case_t1_to_(F, float)
	  case_t1_to_(D, double)

	  /* 算術演算 */
#define case_op2(Operation, Op, T, Type)				\
	  case EBCSCRIPT_INSTRUCTION_ ## Operation ## _ ## T:		\
	    Ebcscript_pop_ ## Type(Env, &(T ## 2));			\
	    Ebcscript_pop_ ## Type(Env, &(T ## 1));			\
	    Ebcscript_push_ ## Type(Env, (T ## 1) Op (T ## 2));		\
/*	    *((Type *)SP + 1) = *((Type *)SP + 1) Op *(Type *)SP;	\
	    SP += sizeof(Type);	*/					\
	    break;

#define case_op2_numeric(Operation, Op)					\
	  case_op2(Operation, Op, C, char)				\
	  case_op2(Operation, Op, S, short)				\
	  case_op2(Operation, Op, I, int)				\
	  case_op2(Operation, Op, L, long)				\
	  case_op2(Operation, Op, UC, uchar)				\
	  case_op2(Operation, Op, US, ushort)				\
	  case_op2(Operation, Op, UI, uint)				\
	  case_op2(Operation, Op, UL, ulong)				\
	  case_op2(Operation, Op, F, float)				\
	  case_op2(Operation, Op, D, double)

#define case_op2_integer(Operation, Op)					\
	  case_op2(Operation, Op, C, char)				\
	  case_op2(Operation, Op, S, short)				\
	  case_op2(Operation, Op, I, int)				\
	  case_op2(Operation, Op, L, long)				\
	  case_op2(Operation, Op, UC, uchar)				\
	  case_op2(Operation, Op, US, ushort)				\
	  case_op2(Operation, Op, UI, uint)				\
	  case_op2(Operation, Op, UL, ulong)

#define case_op2_bit(Operation, Op)					\
	  case_op2(Operation, Op, C, char)				\
	  case_op2(Operation, Op, S, short)				\
	  case_op2(Operation, Op, I, int)				\
	  case_op2(Operation, Op, L, long)

#define case_op1(Operation, Op, T, Type)				\
	  case EBCSCRIPT_INSTRUCTION_ ## Operation ## _ ## T:		\
	    Ebcscript_pop_ ## Type(Env, &(T));				\
	    Ebcscript_push_ ## Type(Env, Op (T));			\
/*	    *(Type *)SP = Op ( *(Type *)SP );	*/			\
	    break;

#define case_op1_numeric(Operation, Op)					\
	  case_op1(Operation, Op, C, char)				\
	  case_op1(Operation, Op, S, short)				\
	  case_op1(Operation, Op, I, int)				\
	  case_op1(Operation, Op, L, long)				\
	  case_op1(Operation, Op, UC, uchar)				\
	  case_op1(Operation, Op, US, ushort)				\
	  case_op1(Operation, Op, UI, uint)				\
	  case_op1(Operation, Op, UL, ulong)				\
	  case_op1(Operation, Op, F, float)				\
	  case_op1(Operation, Op, D, double)

#define case_op1_bit(Operation, Op)					\
	  case_op1(Operation, Op, C, char)				\
	  case_op1(Operation, Op, S, short)				\
	  case_op1(Operation, Op, I, int)				\
	  case_op1(Operation, Op, L, long)

	  case_op2_numeric(ADD, +)
	  case_op2_numeric(SUB, -)
	  case_op2_numeric(MUL, *)
	  case_op2_numeric(DIV, /)
	  case_op2_integer(MOD, %)
	  case_op1_numeric(INC, ++)
	  case_op1_numeric(DEC, --)
	  case_op1_numeric(NEG, -)

	  case EBCSCRIPT_INSTRUCTION_ADD_P:
	    Ebcscript_pop_long(Env, &L2);
	    Ebcscript_pop_address(Env, &P1);
	    Ebcscript_push_address(Env, P1 + L2);
	    break;

	  case EBCSCRIPT_INSTRUCTION_SUB_P:
	    Ebcscript_pop_long(Env, &L2);
	    Ebcscript_pop_address(Env, &P1);
	    Ebcscript_push_address(Env, P1 - L2);
	    break;

	  case EBCSCRIPT_INSTRUCTION_INC_P:
	    /* 使用しない。ADD_Pを使う。 */
	    Ebcscript_pop_address(Env, &P);
	    Ebcscript_push_address(Env, ++P);
	    break;

	  case EBCSCRIPT_INSTRUCTION_DEC_P:
	    /* 使用しない。SUB_Pを使う。 */
	    Ebcscript_pop_address(Env, &P);
	    Ebcscript_push_address(Env, --P);
	    break;

	  /* ビット演算 */
	  case_op2_bit(AND, &)
	  case_op2_bit(OR, |)
	  case_op2_bit(XOR, ^)
	  case_op1_bit(NOT, ~)
	  case_op2_bit(SHR, >>)
	  case_op2_bit(SHL, <<)

	  /* 比較演算 */
#define case_compare(Operation, Op, T, Type)				\
	  case EBCSCRIPT_INSTRUCTION_ ## Operation ## _ ## T:		\
	    Ebcscript_pop_ ## Type(Env, &(T ## 2));			\
	    Ebcscript_pop_ ## Type(Env, &(T ## 1));			\
	    Ebcscript_push_int(Env, (T ## 1) Op (T ## 2));		\
/*	    I = (int)(*((Type *)SP + 1) Op *(Type *)SP);		\
	    SP += sizeof(Type) + sizeof(Type) - sizeof(int);		\
	    *(int *)SP = I;	*/					\
	    break;

#define case_compare_numeric_ptr(Operation, Op)				\
	  case_compare(Operation, Op, C, char)				\
	  case_compare(Operation, Op, S, short)				\
	  case_compare(Operation, Op, I, int)				\
	  case_compare(Operation, Op, L, long)				\
	  case_compare(Operation, Op, UC, uchar)			\
	  case_compare(Operation, Op, US, ushort)			\
	  case_compare(Operation, Op, UI, uint)				\
	  case_compare(Operation, Op, UL, ulong)			\
	  case_compare(Operation, Op, F, float)				\
	  case_compare(Operation, Op, D, double)			\
	  case_compare(Operation, Op, P, address)

	  case_compare_numeric_ptr(LT, <)
	  case_compare_numeric_ptr(LE, <=)
	  case_compare_numeric_ptr(GT, >)
	  case_compare_numeric_ptr(GE, >=)
	  case_compare_numeric_ptr(EQ, ==)
	  case_compare_numeric_ptr(NE, !=)

	  /* 代入式 */
#define case_assign(T, Type)						\
	  case EBCSCRIPT_INSTRUCTION_ASSGN_ ## T:			\
	    Ebcscript_pop_address(Env, &P2);				\
	    Ebcscript_pop_ ## Type(Env, &(T ## 1));			\
	    Ebcscript_push_ ## Type(Env, *(Type *)P2 = (T ## 1));	\
/*	    Q = *(void **)SP;						\
	    SP += sizeof(void *);					\
	    *(Type *)Q = *(Type *)SP;	*/				\
	    break;

	  case_assign(C, char)
	  case_assign(S, short)
	  case_assign(I, int)
	  case_assign(L, long)
	  case_assign(UC, uchar)
	  case_assign(US, ushort)
	  case_assign(UI, uint)
	  case_assign(UL, ulong)
	  case_assign(F, float)
	  case_assign(D, double)
	  case_assign(P, address)

	  case EBCSCRIPT_INSTRUCTION_ASSGN_OBJECT:
	    I = *(int *)CP;	/* サイズ */
	    CP += sizeof(int);
	    Ebcscript_pop_address(Env, &P2);
	    memcpy(P2, SP, I);
	    break;

	  /* 局所ジャンプ */
#define case_jump_t(T, Type)						\
	  case EBCSCRIPT_INSTRUCTION_JMPT_ ## T:			\
	    Ebcscript_pop_ ## Type(Env, &(T));				\
	    if (T) {							\
	      CP = CP + *(ptrdiff_t *)CP;				\
	    } else {							\
	      CP += sizeof(ptrdiff_t);					\
	    }								\
/*	    if (*(Type *)SP) {						\
	      CP = CP + *(int *)CP;					\
	    } else {							\
	      CP += sizeof(int);					\
	    }								\
	    SP += sizeof(Type);	*/					\
	    break;

#define case_jump_f(T, Type)						\
	  case EBCSCRIPT_INSTRUCTION_JMPF_ ## T:			\
	    Ebcscript_pop_ ## Type(Env, &(T));				\
	    if (!T) {							\
	      CP = CP + *(ptrdiff_t *)CP;				\
	    } else {							\
	      CP += sizeof(ptrdiff_t);					\
	    }								\
/*	    if (!(*(Type *)SP)) {					\
	      CP = CP + *(int *)CP;					\
	    } else {							\
	      CP += sizeof(int);					\
	    }								\
	    SP += sizeof(Type);	*/					\
	    break;

	  case_jump_t(C, char)
	  case_jump_t(S, short)
	  case_jump_t(I, int)
	  case_jump_t(L, long)
	  case_jump_t(UC, uchar)
	  case_jump_t(US, ushort)
	  case_jump_t(UI, uint)
	  case_jump_t(UL, ulong)
	  case_jump_t(F, float)
	  case_jump_t(D, double)
	  case_jump_t(P, address)

	  case_jump_f(C, char)
	  case_jump_f(S, short)
	  case_jump_f(I, int)
	  case_jump_f(L, long)
	  case_jump_f(UC, uchar)
	  case_jump_f(US, ushort)
	  case_jump_f(UI, uint)
	  case_jump_f(UL, ulong)
	  case_jump_f(F, float)
	  case_jump_f(D, double)
	  case_jump_f(P, address)

	  case EBCSCRIPT_INSTRUCTION_JMP:
	    CP = CP + *(ptrdiff_t *)CP;
	    break;

	  /* 呼び出し・復帰 */
	  case EBCSCRIPT_INSTRUCTION_CALL_IMM:
	    P = *(void **)CP;
	    CP += sizeof(void *);
	    {
	      ebcscript_functionmap_entry *FInfo;
	      void *CPReturn;

	      /* ネイティブ関数かスクリプト関数か */
	      FInfo = Ebcscript_Functionmap_find(Env->FMap, P);
	      if (FInfo->IsNative) {
	        CPReturn = CP;
	        (*(void (*)(void *))FInfo->CodeAddress)(Env->Stack->Pointer);
	        CP = CPReturn;
	      } else {
	        Ebcscript_push_address(Env, CP);
	        CP = FInfo->CodeAddress;
	      }
	    }
	    break;

	  case EBCSCRIPT_INSTRUCTION_CALL_PTR:
	    Ebcscript_pop_address(Env, &P);
	    {
	      ebcscript_functionmap_entry *FInfo;
	      void *CPReturn;

	      /* ネイティブ関数かスクリプト関数か */
	      FInfo = Ebcscript_Functionmap_find(Env->FMap, P);
	      if (FInfo->IsNative) {
	        CPReturn = CP;
	        (*(void (*)(void *))FInfo->CodeAddress)(Env->Stack->Pointer);
	        CP = CPReturn;
	      } else {
	        Ebcscript_push_address(Env, CP);
	        CP = FInfo->CodeAddress;
	      }
	    }
	    break;

	  case EBCSCRIPT_INSTRUCTION_RET:
	    Ebcscript_pop_address(Env, &CP);
	    break;

	  default:
	    break;
	}
#undef CP
#undef SP
#undef BP
#undef case_push_imm
#undef case_push_gmem
#undef case_push_lmem
#undef case_pop_gmem
#undef case_pop_lmem
#undef case_load
#undef case_store
#undef case_dup
#undef case_t1_to_t2
#undef case_t1_to_
#undef case_op2
#undef case_op2_numeric
#undef case_op2_integer
#undef case_op2_bit
#undef case_op1
#undef case_op1_numeric
#undef case_op1_bit
#undef case_compare
#undef case_compare_numeric_ptr
#undef case_assign
#undef case_jump_t
#undef case_jump_f
}

boolean Ebcscript_addTrnsunit(ebcscript *Env, char *Filename)
{
	extern FILE *yyin;
	extern int Ebcscript_Parser_yyparse(ebcscript_parser *);
	ebcscript_trnsunit *TU;
	boolean B = true;

	if ((yyin = fopen(Filename, "r")) == NULL) {
	  Ebcscript_log(
	   "Ebcscript_addTrnsunit(): error: can't open \"%s\"\n", Filename);
	  return false;
	}

	TU = Ebcscript_newTrnsunit(Filename);
	{
	  ebcscript_parser *Prs;
	  ebcscript_parser_blockstack *BS;

	  /* 作業用変数の初期化 */
	  Prs = Ebcscript_newParser(Filename);
	  Prs->TU = TU;
	  Prs->TU->CP = Prs->TU->Code + sizeof(ebcscript_instruction);
				/* 相対アドレス0はNULL値として用いるため */
	  Prs->BS = NULL;	/* 末尾 */
	  Prs->Nest = 0;
	  Prs->AnonymousNum = 0;
	  Prs->Stack = Env->Stack;

	  BS = Ebcscript_Parser_newBlockstack_trnsunit();
	  BS->As.Trnsunit.TU = TU;
	  Ebcscript_Parser_pushBlockstack(Prs, BS);

	  /* 構文解析の実行 */
	  B &= !Ebcscript_Parser_yyparse(Prs);

	  Ebcscript_Parser_resolve_trnsunit(Prs, BS);
	  BS = Ebcscript_Parser_popBlockstack(Prs);
	  Ebcscript_Parser_deleteBlockstack(BS);

	  Ebcscript_deleteParser(Prs);
	}

	/* バックパッチ */
	B &= Ebcscript_Trnsunit_resolve(TU);

	if (!B) {
	  Ebcscript_log(
	   "Ebcscript_addTrnsunit(): error: failed to parse \"%s\"\n",
	   Filename);
	  Ebcscript_deleteTrnsunit(TU);
	} else {
	  SList_addFront(Env->Trnsunits, (void *)TU);
	  Env->IsResolved = false;	/* 名前の再解決を要求 */

	  /* 全ての関数に対してダミー関数を割り当てる。関数表に登録 */
	  {
	    ebcscript_name *N;

	    Hashmap_foreach(TU->NSVarFuncTypeEnum, N, {
	      if (N->Kind == EBCSCRIPT_NAME_KIND_FUNCTION) {
	        /* removeTrnsunit()でreleaseDummy()する */
	        N->As.Function.FunctionID = Ebcscript_Dummyfunction_getDummy();

	        /* 関数表に登録 */
	        Ebcscript_Functionmap_add(Env->FMap,
	          N->As.Function.FunctionID, N->As.Function.CodeAddress, false);
	      }
	    })
	  }

	}
	fclose(yyin);
	return B;
}

boolean Ebcscript_removeTrnsunit(ebcscript *Env, char *Filename)
{
	ebcscript_trnsunit *TU;
	slist_cell *P, *Q;

	/* Filenameを探す */
	for (Q = &Env->Trnsunits->Head, P = Q->Next;
	     P != NULL;
	     Q = P, P = P->Next) {
	  TU = (ebcscript_trnsunit *)P->Datum;
	  if (!strcmp(TU->Filename, Filename))
	    break;
	}
	if (P == NULL) {
	  Ebcscript_log(
	   "Ebcscript_removeTrnsunit(): "
	   "warning: \"%s\" does not exist in a list of translation-unit\n",
	   Filename);
	  return false;
	}

	/* TUの記号表に登録されている関数を関数表から取り除く */
	{
	  ebcscript_name *N;

	  Hashmap_foreach(TU->NSVarFuncTypeEnum, N, {
	    if (N->Kind == EBCSCRIPT_NAME_KIND_FUNCTION) {
	      /* 関数表から取り除く */
	      Ebcscript_Functionmap_remove(Env->FMap,
	                                             N->As.Function.FunctionID);

	      /* ダミー関数を返却 */
	      Ebcscript_Dummyfunction_releaseDummy(N->As.Function.FunctionID);
	    }
	  })
	}

	Ebcscript_deleteTrnsunit(TU);
	Q->Next = P->Next; deleteSListCell(P);

	/* 名前の再解決を要求 */
	Env->IsResolved = false;

	return true;
}

void Ebcscript_dump(ebcscript *Env)
{
	ebcscript_trnsunit *TU;
	slist_cell *P;

	for (P = Env->Trnsunits->Head.Next;
	     P != NULL;
	     P = P->Next) {
	  TU = (ebcscript_trnsunit *)P->Datum;
	  Ebcscript_Trnsunit_dump(TU);
	}
}

boolean Ebcscript_resolve(ebcscript *Env)
{
	ebcscript_trnsunit *TU, *TU1;
	ebcscript_unresolved *UR;
	ebcscript_name **N0;
	void *Address, *FunctionID;
	int Addressing;
	boolean B, IsFound;
	slist_cell *P, *Q, *R;

	B = true;

	/* 名前の一意性のチェック */
	for (P = Env->Trnsunits->Head.Next; P != NULL; P = P->Next) {
	  ebcscript_name *N;

	  TU = (ebcscript_trnsunit *)P->Datum;

	  Hashmap_foreach(TU->NSVarFuncTypeEnum, N, {
	    if (!Ebcscript_Trnsunit_findVarFuncTypeEnum_global(TU,
	                                                       N->Identifier))
	      continue;

	    for (R = Env->Trnsunits->Head.Next; R != NULL; R = R->Next) {
	      TU1 = (ebcscript_trnsunit *)R->Datum;

	      if (TU == TU1)
	        continue;

	      if (Ebcscript_Trnsunit_findVarFuncTypeEnum_global(TU1,
	                                                       N->Identifier)) {
	        Ebcscript_log(
	         "Ebcscript_resolve(): error: "
	         "the definition of name \'%s\' used in \"%s\" exists "
	         "more than once\n",
	         N->Identifier, TU->Filename);
	        B &= false;
	      }
	    }
	  })
	}

	/* 全翻訳単位の未解決リストに対して */
	for (P = Env->Trnsunits->Head.Next; P != NULL; P = P->Next) {
	  TU = (ebcscript_trnsunit *)P->Datum;

	  for (Q = TU->UnresolvedList->Head.Next; Q != NULL; Q = Q->Next) {
	    UR = (ebcscript_unresolved *)Q->Datum;

	    IsFound = false;
	    for (R = Env->Trnsunits->Head.Next; R != NULL; R = R->Next) {
	      ebcscript_name **N;

	      TU1 = (ebcscript_trnsunit *)R->Datum;

/*	      if (TU == TU1)
	        continue;*/	/* 2019.1.26 */

	      if (!(N = Ebcscript_Trnsunit_findVarFuncTypeEnum_global(TU1,
	                                                  UR->N->Identifier))) {
	        continue;
	      }

	      /* 名前の重複 */
	      if (IsFound) {
	        Ebcscript_log(
	         "Ebcscript_resolve(): error: "
	         "the definition of name \'%s\' used in \"%s\" exists "
	         "more than once\n",
	         UR->N->Identifier, TU->Filename);
	        B &= false;
	        break;
	      }

	      N0 = N;
	      IsFound = true;
	    }

	    /* 見つからない */
	    if (!IsFound) {
	      Ebcscript_log(
	       "Ebcscript_resolve(): error: "
	       "the definition of name \'%s\' used in \"%s\" is not found\n",
	       UR->N->Identifier, TU->Filename);
	      B &= false;
	      continue;
	    }

	    /* 名前の種類 */
	    if ((*N0)->Kind != UR->N->Kind) {
	      Ebcscript_log(
	       "Ebcscript_resolve(): error: "
	       "the kind of name \'%s\' used in \"%s\" doesn't "
	       "match definition\n",
	       UR->N->Identifier, TU->Filename);
	      B &= false;
	      continue;
	    }

	    /* 型 */
	    switch ((*N0)->Kind) {
	      case EBCSCRIPT_NAME_KIND_VARIABLE:
	        if (!Ebcscript_Type_equals((*N0)->As.Variable.TypeTree,
	                                       UR->N->As.Variable.TypeTree)) {
	          Ebcscript_log(
	           "Ebcscript_resolve(): error: "
	           "the type of name \'%s\' used in \"%s\" doesn't "
	           "match declaration\n",
	           UR->N->Identifier, TU->Filename);
	          B &= false;
	          continue;
	        }
	        break;
	      case EBCSCRIPT_NAME_KIND_FUNCTION:
	        if (!Ebcscript_Type_equals((*N0)->As.Function.TypeTree,
	                                       UR->N->As.Function.TypeTree)) {
	          Ebcscript_log(
	           "Ebcscript_resolve(): error: "
	           "the type of name \'%s\' used in \"%s\" doesn't "
	           "match declaration\n",
	           UR->N->Identifier, TU->Filename);
	          B &= false;
	          continue;
	        }
	        break;
	      case EBCSCRIPT_NAME_KIND_TYPEDEF:
	      case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	      default:
	        Ebcscript_log(
	         "Ebcscript_resolve(): error: "
	         "the kind of name \'%s\' used in \"%s\" is illeagal value\n",
	         UR->N->Identifier, TU->Filename);
	        B &= false;
	        continue;
	    }

	    switch ((*N0)->Kind) {
	      case EBCSCRIPT_NAME_KIND_VARIABLE:
	        Address    = (*N0)->As.Variable.Address;
	        Addressing = (*N0)->As.Variable.Addressing;
	        break;
	      case EBCSCRIPT_NAME_KIND_FUNCTION:
	        Address    = (*N0)->As.Function.CodeAddress;/* 絶対化済み */
	        Addressing = (*N0)->As.Function.Addressing;
	        FunctionID = (*N0)->As.Function.FunctionID;
	        break;
	      default:
	        break;
	    }

	    if (Addressing == EBCSCRIPT_NAME_ADDRESSING_UNDEFINED) {
	      /* 上のTrnsunit_find()でNULLが返されるのでスキップされている */
	    }
	    if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE) {
	      *(void **)UR->CP = Address;	/* UR->CPは絶対化済み */
	    }
	    if (Addressing == EBCSCRIPT_NAME_ADDRESSING_FUNCTIONID) {
/*	      *(void **)UR->CP = Address;*/
	      *(void **)UR->CP = FunctionID;
	    }
	    if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ONSTACKFRAME) {
	      *(void **)UR->CP = Address;	/* UR->CPは絶対化済み */
	    }
	    if (Addressing == EBCSCRIPT_NAME_ADDRESSING_ONCODE) {
	      /* Trnsunit_resolve()でABSOLUTEに変更済み */
	    }
	  }
	}
	return Env->IsResolved = B;
}

boolean Ebcscript_call(ebcscript *Env, char *FuncName)
{
	static ebcscript_instruction Trap[] = {EBCSCRIPT_INSTRUCTION_EXIT, 0,};
	ebcscript_trnsunit *TU;
	void *FunctionID;
	ebcscript_functionmap_entry *FInfo;

	if (!Env->IsResolved) {
	  return false;
	}

	if (!(FunctionID = Ebcscript_address(Env, FuncName))) {
	  return false;
	}

	if (!(FInfo = Ebcscript_Functionmap_find(Env->FMap, FunctionID))) {
	  return false;
	}

	/* ネイティブ関数かスクリプト関数か */
	if (FInfo->IsNative) {
	  (*(void (*)(void *))FInfo->CodeAddress)(Env->Stack->Pointer);
	} else {
	  /* 関数の入口アドレスをセット */
	  Env->CP = FInfo->CodeAddress;

	  /* 復帰アドレスのプッシュ */
	  Ebcscript_push_address(Env, Trap);

	  /* BPは、関数に入ったらすぐにプッシュされるので設定する必要はない */

	  /* returnまで実行 */
	  while (*(ebcscript_instruction *)Env->CP !=
	                                           EBCSCRIPT_INSTRUCTION_EXIT) {
	    Ebcscript_execute(Env);
/*	    printf("%p\n", Env->CP);*/
	  }
	}
	return true;
}

/* 全翻訳単位の記号表から検索。外部リンケージの名前のみ。 */
static
ebcscript_name **Ebcscript_findVarFuncTypeEnum_global(ebcscript *Env,
                                                                    char *Name)
{
	ebcscript_trnsunit *TU;
	ebcscript_name **N;
	slist_cell *P;

	/* 全ての翻訳単位を検索 */
	for (P = Env->Trnsunits->Head.Next;
	     P != NULL;
	     P = P->Next) {
	  TU = (ebcscript_trnsunit *)P->Datum;

	  if (N = Ebcscript_Trnsunit_findVarFuncTypeEnum_global(TU, Name)) {
	    return N;	/* IsResolvedフラグが一意性を保証 */
	  }
	}
	return NULL;
}

void *Ebcscript_address(ebcscript *Env, char *Name)
{
	ebcscript_name **N;
	void *Address;

	if (!Env->IsResolved) {
	  return NULL;
	}

	if (!(N = Ebcscript_findVarFuncTypeEnum_global(Env, Name))) {
	  Ebcscript_log(
	   "Ebcscript_address(): error: \'%s\' is not defined\n", Name);
	  return NULL;
	}

	switch ((*N)->Kind) {
	  case EBCSCRIPT_NAME_KIND_VARIABLE:
	    Address = (*N)->As.Variable.Address;
	    break;
	  case EBCSCRIPT_NAME_KIND_FUNCTION:
/*	    Address = (*N)->As.Function.CodeAddress;*/
	    Address = (*N)->As.Function.FunctionID;
	    break;
	  case EBCSCRIPT_NAME_KIND_ENUMERATOR:
	  case EBCSCRIPT_NAME_KIND_TYPEDEF:
	  default:
	    Address = NULL;
	    break;
	}
	return Address;
}

boolean Ebcscript_addVariable(
 ebcscript *Env,
 char *VarName, ebcscript_type *Type, void *Address
)
{
	ebcscript_name *N;

	if (Ebcscript_findVarFuncTypeEnum_global(Env, VarName)) {
	  Ebcscript_log(
	   "Ebcscript_addVariable(): error: \'%s\' has already defined\n",
	   VarName);
	  return false;
	}

	/* 名前を作成 */
	N = Ebcscript_newName_variable(VarName);
	N->As.Variable.TypeTree   = Type;
	N->As.Variable.Address    = Address;
	N->As.Variable.Addressing = EBCSCRIPT_NAME_ADDRESSING_ABSOLUTE;
	N->As.Variable.Linkage    = EBCSCRIPT_NAME_LINKAGE_EXTERNAL;
	N->As.Variable.Nest       = 0;

	/* 記号表に名前を登録 */
	Hashmap_add(Env->TUNative->NSVarFuncTypeEnum, N->Identifier, N);

	Env->IsResolved = false;
	return true;
}

boolean Ebcscript_addFunction(
 ebcscript *Env,
 char *FuncName, ebcscript_type *Type, void *Address, void *Mediator
)
{
	ebcscript_name *N, **N0;

	if (Ebcscript_findVarFuncTypeEnum_global(Env, FuncName)) {
	  Ebcscript_log(
	   "Ebcscript_addFunction(): error: \'%s\' has already defined\n",
	   FuncName);
	  return false;
	}

	/* 名前を作成 */
	N = Ebcscript_newName_function(FuncName);
	N->As.Function.TypeTree    = Type;
	N->As.Function.CodeAddress = Mediator;
	N->As.Function.Addressing  = EBCSCRIPT_NAME_ADDRESSING_FUNCTIONID;
	N->As.Function.Linkage     = EBCSCRIPT_NAME_LINKAGE_EXTERNAL;
	N->As.Function.FunctionID  = Address;

	/* 記号表に名前を登録 */
	Hashmap_add(Env->TUNative->NSVarFuncTypeEnum, N->Identifier, N);

	/* 関数表に登録 */
	if (!Ebcscript_Functionmap_add(Env->FMap,
	         N->As.Function.FunctionID, N->As.Function.CodeAddress, true)) {
	  return false;
	}

	Env->IsResolved = false;
	return true;
}

boolean Ebcscript_removeVariable(ebcscript *Env, char *VarName)
{
	ebcscript_name **N;

	if (!(N = (ebcscript_name **)
	             Hashmap_find(Env->TUNative->NSVarFuncTypeEnum, VarName))) {
	  Ebcscript_log(
	   "Ebcscript_removeVariable(): error: \'%s\' does not exist\n",
	                                                               VarName);
	  return false;
	}
	Hashmap_remove(Env->TUNative->NSVarFuncTypeEnum, (*N)->Identifier);
	Ebcscript_deleteName(*N);
	Env->IsResolved = false;
	return true;
}

boolean Ebcscript_removeFunction(ebcscript *Env, char *FuncName)
{
	ebcscript_name **N;

	if (!(N = (ebcscript_name **)
	            Hashmap_find(Env->TUNative->NSVarFuncTypeEnum, FuncName))) {
	  Ebcscript_log(
	   "Ebcscript_removeFunction(): error: \'%s\' does not exist\n",
	                                                              FuncName);
	  return false;
	}

	/* 関数表から取り除く */
	Ebcscript_Functionmap_remove(Env->FMap, (*N)->As.Function.FunctionID);

	Hashmap_remove(Env->TUNative->NSVarFuncTypeEnum, (*N)->Identifier);
	Ebcscript_deleteName(*N);

	Env->IsResolved = false;
	return true;
}

boolean Ebcscript_bindFunction(
 ebcscript *Env,
 char *Filename, char *FuncName, void *Mediator
)
{
	slist_cell *P;
	ebcscript_trnsunit *TU;
	ebcscript_name **N;
	void *FunctionID;

	/* Filenameを探す */
	for (P = Env->Trnsunits->Head.Next; P != NULL; P = P->Next) {
	  TU = (ebcscript_trnsunit *)P->Datum;
	  if (!strcmp(TU->Filename, Filename))
	    break;
	}
	if (P == NULL) {
	  Ebcscript_log(
	   "Ebcscript_bindFunction(): "
	   "warning: a translation-unit \"%s\" does not exist\n",
	   Filename);
	  return false;
	}

	if (!(N = (ebcscript_name **)
	                       Hashmap_find(TU->NSVarFuncTypeEnum, FuncName))) {
	  Ebcscript_log(
	   "Ebcscript_bindFunction(): "
	   "warning: a function \"%s\" does not exist\n",
	   FuncName);
	  return false;
	}

	if ((*N)->Kind != EBCSCRIPT_NAME_KIND_FUNCTION) {
	  Ebcscript_log(
	   "Ebcscript_bindFunction(): "
	   "warning: a kind of name \"%s\" is not function\n",
	   FuncName);
	  return false;
	}

	Ebcscript_Functionmap_remove(
	 Env->FMap,
	 FunctionID = (*N)->As.Function.FunctionID
	);

	if (Ebcscript_Dummyfunction_isDummy(FunctionID)) {
	  Ebcscript_Dummyfunction_releaseDummy(FunctionID);
	}

	Ebcscript_Functionmap_add(
	 Env->FMap,
	 (*N)->As.Function.FunctionID = Mediator,
	 (*N)->As.Function.CodeAddress,
	 false
	);
	return true;
}

void Ebcscript_setFplog(FILE *Fp)
{
	Hashmap_Fplog =
	SList_Fplog =
	BTree_Fplog =
	Ebcscript_Name_Fplog =
	Ebcscript_Code_Fplog =
	Ebcscript_Functionmap_Fplog =
	Ebcscript_Parser_Declaration_Fplog =
	Ebcscript_Parser_Expression_Fplog =
	Ebcscript_Parser_Initializer_Fplog =
	Ebcscript_Parser_Statement_Fplog =
	Ebcscript_Parser_Fplog =
	Ebcscript_Trnsunit_Fplog =
	Ebcscript_Fplog = Fp;

	Hashmap_log =
	SList_log =
	BTree_log =
	Ebcscript_Name_log =
	Ebcscript_Code_log =
	Ebcscript_Functionmap_log =
	Ebcscript_Parser_Declaration_log =
	Ebcscript_Parser_Expression_log =
	Ebcscript_Parser_Initializer_log =
	Ebcscript_Parser_Statement_log =
	Ebcscript_Parser_log =
	Ebcscript_Trnsunit_log =
	Ebcscript_log;

	Hashmap_exit =
	SList_exit =
	BTree_exit =
	Ebcscript_Name_exit =
	Ebcscript_Code_exit =
	Ebcscript_Functionmap_exit =
	Ebcscript_Parser_Declaration_exit =
	Ebcscript_Parser_Expression_exit =
	Ebcscript_Parser_Initializer_exit =
	Ebcscript_Parser_Statement_exit =
	Ebcscript_Parser_exit =
	Ebcscript_Trnsunit_exit =
	Ebcscript_exit;
}

ebcscript *newEbcscript(int StackSize)
{
	ebcscript *Env;

	Env = Ebcscript_malloc(sizeof(ebcscript));
	Env->Trnsunits = newSList();
	SList_addFront(Env->Trnsunits,
	 Env->TUNative = Ebcscript_newTrnsunit("$native")
	);
	Env->IsResolved = false;
	Env->FMap = Ebcscript_newFunctionmap();
	Env->Stack = Ebcscript_newStack(StackSize);
	Env->BP = Env->Stack->Head + Env->Stack->Size;
	return Env;
}

void deleteEbcscript(ebcscript *Env)
{
	Ebcscript_deleteStack(Env->Stack);
	Ebcscript_deleteFunctionmap(Env->FMap);
	SList_clear(Env->Trnsunits, (void (*)(void *))Ebcscript_deleteTrnsunit);
	deleteSList(Env->Trnsunits);
	Ebcscript_free(Env);
}

/* -------------------------------------------------------------------------- */
#ifdef DEBUG

int main(int argc, char *argv[])
{
	ebcscript *Env;

	Ebcscript_setFplog(stdout);

	Env = newEbcscript(1024);
	Ebcscript_addTrnsunit(Env, argv[1]);

	if (!Ebcscript_resolve(Env)) {
	  printf("名前の解決に失敗\n");
	}

	Ebcscript_dump(Env);

	/* I = f(N) */
	{
	  int I = 0, N = 5;

	  Ebcscript_sub_sp(Env, sizeof(int));
	  Ebcscript_push_int(Env, N);

	  Ebcscript_call(Env, "f");

	  Ebcscript_add_sp(Env, sizeof(int));
	  Ebcscript_pop_int(Env, &I);
	  printf("f(N) = %d\n", I);
	}

	Ebcscript_removeTrnsunit(Env, argv[1]);
	deleteEbcscript(Env);

	printf("Hashmap_MallocTotal = %d\n", Hashmap_MallocTotal);
	printf("SList_MallocTotal = %d\n", SList_MallocTotal);
	printf("BTree_MallocTotal = %d\n", BTree_MallocTotal);
	printf("Ebcscript_Name_MallocTotal = %d\n", Ebcscript_Name_MallocTotal);
	printf("Ebcscript_Code_MallocTotal = %d\n", Ebcscript_Code_MallocTotal);
	printf("Ebcscript_Functionmap_MallocTotal = %d\n",
	                                     Ebcscript_Functionmap_MallocTotal);
	printf("Ebcscript_Parser_Declaration_MallocTotal = %d\n",
	                              Ebcscript_Parser_Declaration_MallocTotal);
	printf("Ebcscript_Parser_Expression_MallocTotal = %d\n",
	                               Ebcscript_Parser_Expression_MallocTotal);
	printf("Ebcscript_Parser_Initializer_MallocTotal = %d\n",
	                              Ebcscript_Parser_Initializer_MallocTotal);
	printf("Ebcscript_Parser_Statement_MallocTotal = %d\n",
	                                Ebcscript_Parser_Statement_MallocTotal);
	printf("Ebcscript_Parser_MallocTotal = %d\n",
	                                          Ebcscript_Parser_MallocTotal);
	printf("Ebcscript_Trnsunit_MallocTotal = %d\n",
	                                        Ebcscript_Trnsunit_MallocTotal);
	printf("Ebcscript_MallocTotal = %d\n", Ebcscript_MallocTotal);


	return 0;
}
#endif
