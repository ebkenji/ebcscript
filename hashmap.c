/*******************************************************************************
   Project : 
   File    : hashmap.c
   Date    : 2018.3.31
   Note    : 
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "boolean.h"
#include "hashmap.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */
/* ハッシュテーブルサイズの候補（双子素数の大きいほう） */
static const int Hashmap_AltTableSizeSize = 13;
static const int Hashmap_AltTableSize[] = {
	13, 31, 73, 151, 313, 643, 1291, 2593,
	5233, 10501, 21013, 42073, 84181,
};

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
void *_Hashmap_malloc(size_t Size);
void _Hashmap_free(void *P);
void _Hashmap_exit(int Status);
void _Hashmap_log(const char *Fmt, ...);

FILE *Hashmap_Fplog/* = stdout*/;
size_t Hashmap_MallocTotal = 0;
void *(*Hashmap_malloc)(size_t Size) = &_Hashmap_malloc;
void (*Hashmap_free)(void *) = &_Hashmap_free;
void (*Hashmap_exit)(int) = &_Hashmap_exit;
void (*Hashmap_log)(const char *Fmt, ...) = &_Hashmap_log;

/*                                                                   関数定義 */
/* -------------------------------------------------------------------------- */
void *_Hashmap_malloc(size_t Size)
{
	void *P;

	if ((P = malloc(Size + sizeof(size_t))) == NULL){
	  Hashmap_log(
	   "Hashmap_malloc(): "
	   "error: memory allocation error(total=%d bytes)\n",
	   Hashmap_MallocTotal);
	  Hashmap_exit(1);
	}
	*(size_t *)P = Size;
	Hashmap_MallocTotal += Size;
	return (size_t *)P + 1;
}

void _Hashmap_free(void *P)
{
	P = (size_t *)P - 1;
	Hashmap_MallocTotal -= *(size_t *)P;
	free(P);
}

void Hashmap_onexit(void)
{
	fclose(Hashmap_Fplog);
}

void _Hashmap_exit(int Status)
{
	Hashmap_onexit();
	exit(Status);
}

void _Hashmap_log(const char *Fmt, ...)
{
	va_list Args;

	fprintf(Hashmap_Fplog, "hashmap: ");

	va_start(Args, Fmt);
	vfprintf(Hashmap_Fplog, Fmt, Args);
	va_end(Args);
}
/* -------------------------------------------------------------------------- */
static
void hash(hashmap *M, char *Key)
{
	int H, Msb;
	int I, N;

	for (I = 0, H = 0, N = strlen(Key); I < N; I++) {
	  /* 左回転 */
	  Msb = (H < 0) ? 1 : 0;
	  H = (H << 1) | Msb;

	  /* 排他的論理和 */
	  H ^= (int)Key[I];
	}
	H &= 0x7fffffff;

	/* ハッシュ値 */
	M->Hash = H % M->TableSize;

	/* 再ハッシュ値（M->TableSize - 2も素数） */
	M->Rehash = 1 + H % (M->TableSize - 2);
}

static
void rehash(hashmap *M)
{
	/* 次の要素 */
	M->Hash -= M->Rehash;
	if (M->Hash < 0) M->Hash += M->TableSize;
}

static
hashmap_entry **peek(hashmap *M)
{
	return &M->Table[M->Hash];
}

static
boolean equalsKey(char *K1, char *K2)
{
	return !strcmp(K1, K2);
}

static
hashmap_entry **find(hashmap *M, char *Key)
{
	int Ct;
	hashmap_entry **E;

	Ct = 0, hash(M, Key);
	while (*(E = peek(M)) != HASHMAP_ENTRY_EMPTY) {
	                                               /* チェーンの末尾まで */
	  if (equalsKey(Key, (*E)->Key)) {
	    return E;	/* 見つかった */
	  }
	  if (++Ct >= M->TableSize) {
	    return NULL;	/* 全部調べたが見つからなかった */
	  }
	  rehash(M);
	}
	return NULL;	/* チェーンの末尾まで到達した */
}

static void resize(hashmap *M);

static
hashmap_entry **findEmptyOrRemoved(hashmap *M, char *Key)
{
	int Ct;
	hashmap_entry **E;

Begin:
	Ct = 0, hash(M, Key);
	while (*(E = peek(M)) != HASHMAP_ENTRY_EMPTY) {
	                                               /* チェーンの末尾まで */
	  if (*E == HASHMAP_ENTRY_REMOVED) {
	    return E;	/* REMOVEDが見つかった */
	  }
	  if (++Ct >= M->TableSize) {
	    resize(M); goto Begin;	/* ハッシュ表を拡張する */
	  }
	  rehash(M);
	}
	return E;	/* チェーンの末尾 */
}

/* ハッシュ表を拡張 */
static
void resize(hashmap *M)
{
	int I;
	int OldTableSize;
	hashmap_entry **OldTable, *E, **F;

	/* 次のテーブルサイズ */
	M->IndexTableSize++;
	if (Hashmap_AltTableSizeSize <= M->IndexTableSize) {
	  Hashmap_log(
	   "Hashmap_resize(): error: unexpected size\n");
	  Hashmap_exit(1);
	}

	/* 古いテーブルを保存しておく */
	OldTableSize = M->TableSize;
	OldTable = M->Table;

	/* 新しいテーブルを確保する */
	M->TableSize = Hashmap_AltTableSize[M->IndexTableSize];
	M->Table = Hashmap_malloc(sizeof(hashmap_entry *) * M->TableSize);
	for (I = 0; I < M->TableSize; I++) {
	  M->Table[I] = HASHMAP_ENTRY_EMPTY;
	}

	/* 詰め直し */
	for (I = 0; I < OldTableSize; I++) {
	  if ((E = OldTable[I]) != HASHMAP_ENTRY_EMPTY
	                   && E != HASHMAP_ENTRY_REMOVED) {
	    F = findEmptyOrRemoved(M, E->Key);
	    *F = E;
	  }
	}

	/* 古いテーブルを破棄する */
	Hashmap_free(OldTable);
}
/* -------------------------------------------------------------------------- */
hashmap_entry *Hashmap_newEntry(char *Key, void *Value)
{
	hashmap_entry *E;

	E = Hashmap_malloc(sizeof(hashmap_entry));
	E->Key = (char *)Hashmap_malloc(strlen(Key) + 1);
	strcpy(E->Key, Key);
	E->Value = Value;
	return E;
}

void Hashmap_deleteEntry(hashmap_entry *E)
{
	Hashmap_free(E->Key);
	Hashmap_free(E);
}

hashmap *newHashmap(void)
{
	hashmap *M;
	int I;

	M = Hashmap_malloc(sizeof(hashmap));
	M->IndexTableSize = 0;
	M->TableSize = Hashmap_AltTableSize[0];
	M->Table = Hashmap_malloc(sizeof(hashmap_entry *) * M->TableSize);
	for (I = 0; I < M->TableSize; I++) {
	  M->Table[I] = HASHMAP_ENTRY_EMPTY;
	}
	return M;
}

void deleteHashmap(hashmap *M)
{
	int I;
	hashmap_entry *E;

	for (I = 0; I < M->TableSize; I++) {
	  if ((E = M->Table[I]) != HASHMAP_ENTRY_EMPTY
	                   && E != HASHMAP_ENTRY_REMOVED) {
	    Hashmap_deleteEntry(E);
	  }
	}
	Hashmap_free(M->Table);
	Hashmap_free(M);
}

boolean Hashmap_containsKey(hashmap *M, char *Key)
{
	hashmap_entry **E;

	if (E = find(M, Key)) {
	  return true;
	} else {
	  return false;
	}
}

/* Valueへのポインタを返す。
   見つからなければ、NULLを返す。 */
void **Hashmap_find(hashmap *M, char *Key)
{
	hashmap_entry **E;

	if (Key == NULL) {
	  Hashmap_log("Hashmap_find(): error: null argument\n");
	  Hashmap_exit(1);
	}

	if (E = find(M, Key)) {
	  return &(*E)->Value;
	} else {
	  return NULL;
	}
}

/* 登録に成功したらtrueを返す。
   登録に失敗（すでに同じキーを持つデータがある）したらfalseを返す。 */
boolean Hashmap_add(hashmap *M, char *Key, void *Value)
{
	hashmap_entry **E;

	if (Key == NULL) {
	  Hashmap_log("Hashmap_add(): error: null argument\n");
	  Hashmap_exit(1);
	}

	if (find(M, Key)) {
	  /* 重複。上書きする方法も。 */
	  Hashmap_log(
	   "Hashmap_add(): error: key \'%s\' already exists\n", Key);
	  return false;
	}

	if (E = findEmptyOrRemoved(M, Key)) {
	  *E = Hashmap_newEntry(Key, Value);
	  return true;
	} else {
	  /* ここでresize()しない。データ構造に依存することはここに書かない。*/
	  return false;
	}
}

/* 削除が成功したらtrueを返す。
   データが見つからなければfalseを返す。 */
boolean Hashmap_remove(hashmap *M, char *Key)
{
	hashmap_entry **E;

	if (Key == NULL) {
	  Hashmap_log("Hashmap_remove(): error: null argument\n");
	  Hashmap_exit(1);
	}

	if (E = find(M, Key)) {
	  Hashmap_deleteEntry(*E); *E = HASHMAP_ENTRY_REMOVED;
	  return true;
	} else {
	  return false;
	}
}

boolean Hashmap_isEmpty(hashmap *M)
{
	int I;

	for (I = 0; I < M->TableSize; I++) {
	  if (M->Table[I] == HASHMAP_ENTRY_EMPTY)
	    return false;
	}
	return true;
}

void Hashmap_clear(hashmap *M, void (*DeleteValue)(void *))
{
	int I;
	hashmap_entry *E;

	for (I = 0; I < M->TableSize; I++) {
	  if ((E = M->Table[I]) != HASHMAP_ENTRY_EMPTY
	                   && E != HASHMAP_ENTRY_REMOVED) {
	    DeleteValue(E->Value);
	    Hashmap_deleteEntry(E);
	    M->Table[I] = HASHMAP_ENTRY_REMOVED;
	  }
	}
}

void Hashmap_dump(hashmap *M)
{
	int I;
	hashmap_entry *E;

	for (I = 0; I < M->TableSize; I++) {
	  fprintf(Hashmap_Fplog, "[%d] ", I);

	  if ((E = M->Table[I]) != HASHMAP_ENTRY_EMPTY
	                   && E != HASHMAP_ENTRY_REMOVED) {
	    hash(M, E->Key);
	    fprintf(Hashmap_Fplog, "(%d,%d) ", M->Hash, M->Rehash);
	    fprintf(Hashmap_Fplog, "%s", E->Key);
	  }
	  fprintf(Hashmap_Fplog, "\n");
	}
}
