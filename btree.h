/*******************************************************************************
   Project : 
   File    : btree.h
   Date    : 2013.4.28-
   Note    : binary tree
*******************************************************************************/
#ifndef BTREE
#define BTREE

/*                                                                 構造体宣言 */
/* -------------------------------------------------------------------------- */
typedef struct btree {
	void *Datum;
	struct btree *Left;
	struct btree *Right;
} btree;

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
extern FILE *BTree_Fplog;
extern size_t BTree_MallocTotal;
extern void *(*BTree_malloc)(size_t);
extern void (*BTree_free)(void *);
extern void (*BTree_exit)(int);
extern void (*BTree_log)(const char *Fmt, ...);

/*                                                                   関数宣言 */
/* -------------------------------------------------------------------------- */
void BTree_onexit(void);

btree *newBTree(void *Datum, btree *Left, btree *Right);
void *deleteBTree(btree *P);

void BTree_clearDummy(void *Datum);
void BTree_clear(btree *T, void (*DeleteDatum)(void *));

#endif
