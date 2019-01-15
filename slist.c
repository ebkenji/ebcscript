/*******************************************************************************
   Project : 
   File    : slist.c
   Date    : 2018.4.10-
   Note    : singly linked list
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "boolean.h"
#include "slist.h"

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
void *_SList_malloc(size_t Size);
void _SList_free(void *P);
void _SList_exit(int Status);
void _SList_log(const char *Fmt, ...);

FILE *SList_Fplog/* = stdout*/;
size_t SList_MallocTotal = 0;
void *(*SList_malloc)(size_t Size) = &_SList_malloc;
void (*SList_free)(void *) = &_SList_free;
void (*SList_exit)(int) = &_SList_exit;
void (*SList_log)(const char *Fmt, ...) = &_SList_log;

/*                                                                   関数定義 */
/* -------------------------------------------------------------------------- */
void *_SList_malloc(size_t Size)
{
	void *P;

	if ((P = malloc(Size + sizeof(size_t))) == NULL){
	  SList_log(
	   "SList_malloc(): "
	   "error: memory allocation error(total=%d bytes)\n",
	   SList_MallocTotal);
	  SList_exit(1);
	}
	*(size_t *)P = Size;
	SList_MallocTotal += Size;
	return (size_t *)P + 1;
}

void _SList_free(void *P)
{
	P = (size_t *)P - 1;
	SList_MallocTotal -= *(size_t *)P;
	free(P);
}

void SList_onexit(void)
{
	fclose(SList_Fplog);
}

void _SList_exit(int Status)
{
	SList_onexit();
	exit(Status);
}

void _SList_log(const char *Fmt, ...)
{
	va_list Args;

	fprintf(SList_Fplog, "slist: ");

	va_start(Args, Fmt);
	vfprintf(SList_Fplog, Fmt, Args);
	va_end(Args);
}
/* -------------------------------------------------------------------------- */
slist *newSList(void)
{
	slist *P;

	P = SList_malloc(sizeof(slist));
	P->Head.Datum = NULL;
	P->Head.Next = NULL;
	return P;
}

void deleteSList(slist *L)
{
	if (L != NULL)
	  SList_free(L);
}

slist_cell *newSListCell(void *Datum, slist_cell *Next)
{
	slist_cell *P;

	P = SList_malloc(sizeof(slist_cell));
	P->Datum = Datum;
	P->Next = Next;
	return P;
}

void *deleteSListCell(slist_cell *P)
{
	void *Q = P->Datum;

	SList_free(P);
	return Q;
}

void SList_addFront(slist *L, void *Datum)
{
	L->Head.Next = newSListCell(Datum, L->Head.Next);
}

void SList_addLast(slist *L, void *Datum)
{
	slist_cell *Q;

	for (Q = &(L->Head); Q->Next != NULL; Q = Q->Next)
	  ;
	Q->Next = newSListCell(Datum, NULL);
}

void SList_clearDummy(void *Datum)
{
	return;
}

void SList_clear(slist *L, void (*deleteDatum)(void *))
{
	slist_cell *P, *Q;

	for (P = L->Head.Next;
	     P != NULL;
	     P = (Q = P)->Next, deleteDatum(deleteSListCell(Q)))
	  ;
	L->Head.Next = NULL;
}

void *SList_request(slist *L, int N)
{
	slist_cell *P;

	for (P = L->Head.Next; N > 0 && P != NULL; N--, P = P->Next)
	  ;
	if (P == NULL) {
	  SList_log(
	   "SList_request(): "
	   "error: not exist a %d th element in a list\n", N);
	  SList_exit(1);
	}
	return P->Datum;
}

boolean SList_isEmpty(slist *L)
{
	return L->Head.Next == NULL;
}
