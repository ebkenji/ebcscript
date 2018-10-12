/*******************************************************************************
   Project : 
   File    : hashmap.h
   Date    : 2018.3.31
   Note    : 
*******************************************************************************/
#ifndef HASHMAP
#define HASHMAP

#include "boolean.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */
#define HASHMAP_ENTRY_EMPTY	(hashmap_entry *)0
#define HASHMAP_ENTRY_REMOVED	(hashmap_entry *)1

/*                                                                 構造体宣言 */
/* -------------------------------------------------------------------------- */
typedef struct hashmap hashmap;
typedef struct hashmap_entry hashmap_entry;
struct hashmap {
	int IndexTableSize;/* 現在使っているテーブルサイズの候補の添字 */
	int TableSize;
	struct hashmap_entry {
	  char *Key;
	  void *Value;
	} **Table;

	int Hash;
	int Rehash;
};

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
extern FILE *Hashmap_Fplog;
extern size_t Hashmap_MallocTotal;
extern void *(*Hashmap_malloc)(size_t Size);
extern void (*Hashmap_free)(void *);
extern void (*Hashmap_exit)(int);
extern void (*Hashmap_log)(const char *Fmt, ...);

/*                                                                 マクロ定義 */
/* -------------------------------------------------------------------------- */
#define Hashmap_foreach(M, V, Action)					\
{									\
	int I;								\
	hashmap_entry *E;						\
	for (I = 0; I < M->TableSize; I++) {				\
	  if ((E = M->Table[I]) != HASHMAP_ENTRY_EMPTY			\
	                   && E != HASHMAP_ENTRY_REMOVED) {		\
	    V = E->Value;						\
	    Action;							\
	  }								\
	}								\
}

/*                                                                   関数宣言 */
/* -------------------------------------------------------------------------- */
void Hashmap_onexit(void);

hashmap *newHashmap(void);
void deleteHashmap(hashmap *);

boolean Hashmap_containsKey(hashmap *M, char *Key);

/* 登録に成功したらtrueを返す。
   登録に失敗（すでに同じキーを持つデータがある）したらfalseを返す。 */
boolean Hashmap_add(hashmap *M, char *Key, void *Value);

/* Valueへのポインタを返す。
   見つからなければ、NULLを返す。 */
void **Hashmap_find(hashmap *M, char *Key);

/* 削除が成功したらtrueを返す。
   データが見つからなければfalseを返す。 */
boolean Hashmap_remove(hashmap *M, char *Key);

/* 全ての要素を削除する */
void Hashmap_clear(hashmap *M, void (*DeleteValue)(void *));

/* 1つも登録されていない */
boolean Hashmap_isEmpty(hashmap *M);

void Hashmap_dump(hashmap *M);

#endif
