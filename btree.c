/*******************************************************************************
   Project : 
   File    : btree.c
   Date    : 2013.4.28-
   Note    : binary tree
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "btree.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
void *_BTree_malloc(size_t Size);
void _BTree_free(void *P);
void _BTree_exit(int Status);
void _BTree_log(const char *Fmt, ...);

FILE *BTree_Fplog/* = stdout*/;
size_t BTree_MallocTotal = 0;
void *(*BTree_malloc)(size_t Size) = &_BTree_malloc;
void (*BTree_free)(void *) = &_BTree_free;
void (*BTree_exit)(int) = &_BTree_exit;
void (*BTree_log)(const char *Fmt, ...) = &_BTree_log;

/*                                                                   関数定義 */
/* -------------------------------------------------------------------------- */
void *_BTree_malloc(size_t Size)
{
	void *P;

	if ((P = malloc(Size + sizeof(size_t))) == NULL){
	  BTree_log(
	   "BTree_malloc(): "
	   "error: memory allocation error(total=%d bytes)\n",
	   BTree_MallocTotal);
	  BTree_exit(1);
	}
	*(size_t *)P = Size;
	BTree_MallocTotal += Size;
	return (size_t *)P + 1;
}

void _BTree_free(void *P)
{
	P = (size_t *)P - 1;
	BTree_MallocTotal -= *(size_t *)P;
	free(P);
}

void BTree_onexit(void)
{
	fclose(BTree_Fplog);
}

void _BTree_exit(int Status)
{
	fclose(BTree_Fplog);
	exit(Status);
}

void _BTree_log(const char *Fmt, ...)
{
	va_list Args;

	fprintf(BTree_Fplog, "btree: ");

	va_start(Args, Fmt);
	vfprintf(BTree_Fplog, Fmt, Args);
	va_end(Args);
}
/* -------------------------------------------------------------------------- */
btree *newBTree(void *Datum, btree *Left, btree *Right)
{
	btree *P;

	P = BTree_malloc(sizeof(btree));
	P->Datum = Datum;
	P->Left = Left;
	P->Right = Right;
	return P;
}

void *deleteBTree(btree *T)
{
	void *P;

	if (T == NULL) return NULL;

	P = T->Datum;
	BTree_free(T);
	return P;
}

void BTree_clearDummy(void *Datum)
{
	return;
}

void BTree_clear(btree *T, void (*DeleteDatum)(void *))
{
	void *P;

	if (T == NULL) return;

	BTree_clear(T->Left, DeleteDatum);
	BTree_clear(T->Right, DeleteDatum);
	if ((P = deleteBTree(T)) != NULL)
	  DeleteDatum(P);
}
