/*******************************************************************************
   Project : C script
   File    : code.c
   Date    : 2018.5.7-
   Note    : 実行環境の作業用スタック、中間コード領域
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "code.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Code_malloc(size_t Size);
void _Ebcscript_Code_free(void *P);
void _Ebcscript_Code_exit(int Status);
void _Ebcscript_Code_log(const char *Fmt, ...);

FILE *Ebcscript_Code_Fplog/* = stdout*/;
size_t Ebcscript_Code_MallocTotal = 0;
void *(*Ebcscript_Code_malloc)(size_t Size) = &_Ebcscript_Code_malloc;
void (*Ebcscript_Code_free)(void *) = &_Ebcscript_Code_free;
void (*Ebcscript_Code_exit)(int) = &_Ebcscript_Code_exit;
void (*Ebcscript_Code_log)(const char *Fmt, ...) = &_Ebcscript_Code_log;

/*                                                                   関数定義 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Code_malloc(size_t Size)
{
	void *P;

	if ((P = malloc(Size + sizeof(size_t))) == NULL){
	  Ebcscript_Code_log(
	   "Ebcscript_Code_malloc(): "
	   "error: memory allocation error(total=%d bytes)\n",
	   Ebcscript_Code_MallocTotal);
	  Ebcscript_Code_exit(1);
	}
	*(size_t *)P = Size;
	Ebcscript_Code_MallocTotal += Size;
	return (size_t *)P + 1;
}

void _Ebcscript_Code_free(void *P)
{
	P = (size_t *)P - 1;
	Ebcscript_Code_MallocTotal -= *(size_t *)P;
	free(P);
	return;
}

void Ebcscript_Code_onexit(void)
{
	fclose(Ebcscript_Code_Fplog);
}

void _Ebcscript_Code_exit(int Status)
{
	Ebcscript_Code_onexit();
	exit(Status);
}

void _Ebcscript_Code_log(const char *Fmt, ...)
{
	va_list Args;

	fprintf(Ebcscript_Code_Fplog, "ebcscript_code: ");

	va_start(Args, Fmt);
	vfprintf(Ebcscript_Code_Fplog, Fmt, Args);
	va_end(Args);
}
/* -------------------------------------------------------------------------- */
ebcscript_stack *Ebcscript_newStack(size_t Size)
{
	ebcscript_stack *Stk;

	Stk = Ebcscript_Code_malloc(sizeof(ebcscript_stack));
	Stk->Size = Size;
	Stk->Head = Ebcscript_Code_malloc(Size);
	Stk->Pointer = Stk->Head + Stk->Size;

	return Stk;
}

void Ebcscript_deleteStack(ebcscript_stack *Stk)
{
	Ebcscript_Code_free(Stk->Head);
	Ebcscript_Code_free(Stk);
}

void Ebcscript_Stack_pointLast(ebcscript_stack *Stk)
{
	Stk->Pointer = Stk->Head + Stk->Size;
}

void Ebcscript_Stack_push_char(ebcscript_stack *Stk, char C)
{
	if (Stk->Pointer - sizeof(char) < Stk->Head) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_push_char(): error: stack overflow\n");
	  Ebcscript_Code_exit(1);
	}
	Stk->Pointer -= sizeof(char);
	*(char *)Stk->Pointer = C;
}

void Ebcscript_Stack_push_short(ebcscript_stack *Stk, short S)
{
	if (Stk->Pointer - sizeof(short) < Stk->Head) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_push_short(): error: stack overflow\n");
	  Ebcscript_Code_exit(1);
	}
	Stk->Pointer -= sizeof(short);
	*(short *)Stk->Pointer = S;
}

void Ebcscript_Stack_push_int(ebcscript_stack *Stk, int I)
{
	if (Stk->Pointer - sizeof(int) < Stk->Head) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_push_int(): error: stack overflow\n");
	  Ebcscript_Code_exit(1);
	}
	Stk->Pointer -= sizeof(int);
	*(int *)Stk->Pointer = I;
}

void Ebcscript_Stack_push_long(ebcscript_stack *Stk, long L)
{
	if (Stk->Pointer - sizeof(long) < Stk->Head) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_push_long(): error: stack overflow\n");
	  Ebcscript_Code_exit(1);
	}
	Stk->Pointer -= sizeof(long);
	*(long *)Stk->Pointer = L;
}

void Ebcscript_Stack_push_uchar(ebcscript_stack *Stk, unsigned char C)
{
	if (Stk->Pointer - sizeof(unsigned char) < Stk->Head) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_push_uchar(): error: stack overflow\n");
	  Ebcscript_Code_exit(1);
	}
	Stk->Pointer -= sizeof(unsigned char);
	*(unsigned char *)Stk->Pointer = C;
}

void Ebcscript_Stack_push_ushort(ebcscript_stack *Stk, unsigned short S)
{
	if (Stk->Pointer - sizeof(unsigned short) < Stk->Head) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_push_ushort(): error: stack overflow\n");
	  Ebcscript_Code_exit(1);
	}
	Stk->Pointer -= sizeof(unsigned short);
	*(unsigned short *)Stk->Pointer = S;
}

void Ebcscript_Stack_push_uint(ebcscript_stack *Stk, unsigned int I)
{
	if (Stk->Pointer - sizeof(unsigned int) < Stk->Head) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_push_uint(): error: stack overflow\n");
	  Ebcscript_Code_exit(1);
	}
	Stk->Pointer -= sizeof(unsigned int);
	*(unsigned int *)Stk->Pointer = I;
}

void Ebcscript_Stack_push_ulong(ebcscript_stack *Stk, unsigned long L)
{
	if (Stk->Pointer - sizeof(unsigned long) < Stk->Head) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_push_ulong(): error: stack overflow\n");
	  Ebcscript_Code_exit(1);
	}
	Stk->Pointer -= sizeof(unsigned long);
	*(unsigned long *)Stk->Pointer = L;
}

void Ebcscript_Stack_push_float(ebcscript_stack *Stk, float F)
{
	if (Stk->Pointer - sizeof(float) < Stk->Head) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_push_float(): error: stack overflow\n");
	  Ebcscript_Code_exit(1);
	}
	Stk->Pointer -= sizeof(float);
	*(float *)Stk->Pointer = F;
}

void Ebcscript_Stack_push_double(ebcscript_stack *Stk, double D)
{
	if (Stk->Pointer - sizeof(double) < Stk->Head) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_push_double(): error: stack overflow\n");
	  Ebcscript_Code_exit(1);
	}
	Stk->Pointer -= sizeof(double);
	*(double *)Stk->Pointer = D;
}

void Ebcscript_Stack_push_address(ebcscript_stack *Stk, void *P)
{
	if (Stk->Pointer - sizeof(void *) < Stk->Head) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_push_address(): error: stack overflow\n");
	  Ebcscript_Code_exit(1);
	}
	Stk->Pointer -= sizeof(void *);
	*(void **)Stk->Pointer = P;
}

void Ebcscript_Stack_pop_char(ebcscript_stack *Stk, char *C)
{
	if (Stk->Pointer + sizeof(char) > Stk->Head + Stk->Size) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_pop_char(): error: stack overrun\n");
	  Ebcscript_Code_exit(1);
	}
	*C = *(char *)Stk->Pointer;
	Stk->Pointer += sizeof(char);
}

void Ebcscript_Stack_pop_short(ebcscript_stack *Stk, short *S)
{
	if (Stk->Pointer + sizeof(short) > Stk->Head + Stk->Size) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_pop_short(): error: stack overrun\n");
	  Ebcscript_Code_exit(1);
	}
	*S = *(short *)Stk->Pointer;
	Stk->Pointer += sizeof(short);
}

void Ebcscript_Stack_pop_int(ebcscript_stack *Stk, int *I)
{
	if (Stk->Pointer + sizeof(int) > Stk->Head + Stk->Size) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_pop_int(): error: stack overrun\n");
	  Ebcscript_Code_exit(1);
	}
	*I = *(int *)Stk->Pointer;
	Stk->Pointer += sizeof(int);
}

void Ebcscript_Stack_pop_long(ebcscript_stack *Stk, long *L)
{
	if (Stk->Pointer + sizeof(long) > Stk->Head + Stk->Size) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_pop_long(): error: stack overrun\n");
	  Ebcscript_Code_exit(1);
	}
	*L = *(long *)Stk->Pointer;
	Stk->Pointer += sizeof(long);
}

void Ebcscript_Stack_pop_uchar(ebcscript_stack *Stk, unsigned char *C)
{
	if (Stk->Pointer + sizeof(unsigned char) > Stk->Head + Stk->Size) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_pop_uchar(): error: stack overrun\n");
	  Ebcscript_Code_exit(1);
	}
	*C = *(unsigned char *)Stk->Pointer;
	Stk->Pointer += sizeof(unsigned char);
}

void Ebcscript_Stack_pop_ushort(ebcscript_stack *Stk, unsigned short *S)
{
	if (Stk->Pointer + sizeof(unsigned short) > Stk->Head + Stk->Size) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_pop_ushort(): error: stack overrun\n");
	  Ebcscript_Code_exit(1);
	}
	*S = *(unsigned short *)Stk->Pointer;
	Stk->Pointer += sizeof(unsigned short);
}

void Ebcscript_Stack_pop_uint(ebcscript_stack *Stk, unsigned int *I)
{
	if (Stk->Pointer + sizeof(unsigned int) > Stk->Head + Stk->Size) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_pop_uint(): error: stack overrun\n");
	  Ebcscript_Code_exit(1);
	}
	*I = *(unsigned int *)Stk->Pointer;
	Stk->Pointer += sizeof(unsigned int);
}

void Ebcscript_Stack_pop_ulong(ebcscript_stack *Stk, unsigned long *L)
{
	if (Stk->Pointer + sizeof(unsigned long) > Stk->Head + Stk->Size) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_pop_ulong(): error: stack overrun\n");
	  Ebcscript_Code_exit(1);
	}
	*L = *(unsigned long *)Stk->Pointer;
	Stk->Pointer += sizeof(unsigned long);
}

void Ebcscript_Stack_pop_float(ebcscript_stack *Stk, float *F)
{
	if (Stk->Pointer + sizeof(float) > Stk->Head + Stk->Size) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_pop_float(): error: stack overrun\n");
	  Ebcscript_Code_exit(1);
	}
	*F = *(float *)Stk->Pointer;
	Stk->Pointer += sizeof(float);
}

void Ebcscript_Stack_pop_double(ebcscript_stack *Stk, double *D)
{
	if (Stk->Pointer + sizeof(double) > Stk->Head + Stk->Size) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_pop_double(): error: stack overrun\n");
	  Ebcscript_Code_exit(1);
	}
	*D = *(double *)Stk->Pointer;
	Stk->Pointer += sizeof(double);
}

void Ebcscript_Stack_pop_address(ebcscript_stack *Stk, void **P)
{
	if (Stk->Pointer + sizeof(void *) > Stk->Head + Stk->Size) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_pop_address(): error: stack overrun\n");
	  Ebcscript_Code_exit(1);
	}
	*P = *(void **)Stk->Pointer;
	Stk->Pointer += sizeof(void *);
}

void Ebcscript_Stack_add_sp(ebcscript_stack *Stk, int I)
{
	Stk->Pointer += I;
	if (Stk->Pointer > Stk->Head + Stk->Size) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_add_sp(): error: stack overrun\n");
	  Ebcscript_Code_exit(1);
	}
}

void Ebcscript_Stack_sub_sp(ebcscript_stack *Stk, int I)
{
	Stk->Pointer -= I;
	if (Stk->Pointer < Stk->Head) {
	  Ebcscript_Code_log(
	   "Ebcscript_Stack_sub_sp(): error: stack overflow\n");
	  Ebcscript_Code_exit(1);
	}
}
