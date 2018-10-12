/*******************************************************************************
   Project : 
   File    : slist.h
   Date    : 2018.4.10-
   Note    : singly linked list
*******************************************************************************/
#ifndef SLIST
#define SLIST

#include "boolean.h"

/*                                                                 構造体宣言 */
/* -------------------------------------------------------------------------- */
typedef struct slist_cell {
	void *Datum;
	struct slist_cell *Next;
} slist_cell;

typedef struct slist {
	struct slist_cell Head;
} slist;

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
extern FILE *SList_Fplog;
extern size_t SList_MallocTotal;
extern void *(*SList_malloc)(size_t);
extern void (*SList_free)(void *);
extern void (*SList_exit)(int);
extern void (*SList_log)(const char *Fmt, ...);

/*                                                                 マクロ定義 */
/* -------------------------------------------------------------------------- */
#define SList_foreach(L, Value, Action)					\
{									\
	slist_cell *P, *Q;						\
	for (Q = &(L)->Head, P = Q->Next;				\
	     P != NULL;							\
	     Q = P, P = P->Next) {					\
	  Value = P->Datum;						\
	  Action;							\
	}								\
}

/*                                                                   関数宣言 */
/* -------------------------------------------------------------------------- */
void SList_onexit(void);

slist *newSList(void);
/* Lに連結する要素は消えない。先にSList_clear()を行わなければならない */
void deleteSList(slist *L);

void SList_addFront(slist *L, void *Datum);
void SList_addLast(slist *L, void *Datum);

/* L自体は消えない。後でdeleteSList()を行わなければならない */
void SList_clear(slist *L, void (*deleteDatum)(void *));
void SList_clearDummy(void *Datum);

void *SList_request(slist *L, int N);

/* 1つも要素がない */
boolean SList_isEmpty(slist *L);

slist_cell *newSListCell(void *Datum, slist_cell *Next);
void *deleteSListCell(slist_cell *P);

#endif
