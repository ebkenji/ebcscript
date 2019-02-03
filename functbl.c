/*******************************************************************************
   Project : C script
   File    : functbl.c
   Date    : 2019.1.26-
   Note    : 関数表とダミー関数の管理
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "functbl.h"
#include "boolean.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */
#define EBCSCRIPT_DUMMYFUNCTION_POOLSIZE 32
#define EBCSCRIPT_FUNCTIONMAP_SIZE_INITIAL 16

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Functionmap_malloc(size_t Size);
void _Ebcscript_Functionmap_free(void *P);
void _Ebcscript_Functionmap_exit(int Status);
void _Ebcscript_Functionmap_log(const char *Fmt, ...);

FILE *Ebcscript_Functionmap_Fplog/* = stdout*/;
size_t Ebcscript_Functionmap_MallocTotal = 0;
void *(*Ebcscript_Functionmap_malloc)(size_t Size) =
                                                 &_Ebcscript_Functionmap_malloc;
void (*Ebcscript_Functionmap_free)(void *) = &_Ebcscript_Functionmap_free;
void (*Ebcscript_Functionmap_exit)(int) = &_Ebcscript_Functionmap_exit;
void (*Ebcscript_Functionmap_log)(const char *Fmt, ...) =
                                                    &_Ebcscript_Functionmap_log;

/* -------------------------------------------------------------------------- */
static void Ebcscript_Dummyfunction_000(void) {}
static void Ebcscript_Dummyfunction_001(void) {}
static void Ebcscript_Dummyfunction_002(void) {}
static void Ebcscript_Dummyfunction_003(void) {}
static void Ebcscript_Dummyfunction_004(void) {}
static void Ebcscript_Dummyfunction_005(void) {}
static void Ebcscript_Dummyfunction_006(void) {}
static void Ebcscript_Dummyfunction_007(void) {}
static void Ebcscript_Dummyfunction_008(void) {}
static void Ebcscript_Dummyfunction_009(void) {}
static void Ebcscript_Dummyfunction_00a(void) {}
static void Ebcscript_Dummyfunction_00b(void) {}
static void Ebcscript_Dummyfunction_00c(void) {}
static void Ebcscript_Dummyfunction_00d(void) {}
static void Ebcscript_Dummyfunction_00e(void) {}
static void Ebcscript_Dummyfunction_00f(void) {}
static void Ebcscript_Dummyfunction_010(void) {}
static void Ebcscript_Dummyfunction_011(void) {}
static void Ebcscript_Dummyfunction_012(void) {}
static void Ebcscript_Dummyfunction_013(void) {}
static void Ebcscript_Dummyfunction_014(void) {}
static void Ebcscript_Dummyfunction_015(void) {}
static void Ebcscript_Dummyfunction_016(void) {}
static void Ebcscript_Dummyfunction_017(void) {}
static void Ebcscript_Dummyfunction_018(void) {}
static void Ebcscript_Dummyfunction_019(void) {}
static void Ebcscript_Dummyfunction_01a(void) {}
static void Ebcscript_Dummyfunction_01b(void) {}
static void Ebcscript_Dummyfunction_01c(void) {}
static void Ebcscript_Dummyfunction_01d(void) {}
static void Ebcscript_Dummyfunction_01e(void) {}
static void Ebcscript_Dummyfunction_01f(void) {}

ebcscript_dummyfunction Ebcscript_Dummyfunction_Pool[
                                           EBCSCRIPT_DUMMYFUNCTION_POOLSIZE] = {
	{false, (void *)&Ebcscript_Dummyfunction_000},
	{false, (void *)&Ebcscript_Dummyfunction_001},
	{false, (void *)&Ebcscript_Dummyfunction_002},
	{false, (void *)&Ebcscript_Dummyfunction_003},
	{false, (void *)&Ebcscript_Dummyfunction_004},
	{false, (void *)&Ebcscript_Dummyfunction_005},
	{false, (void *)&Ebcscript_Dummyfunction_006},
	{false, (void *)&Ebcscript_Dummyfunction_007},
	{false, (void *)&Ebcscript_Dummyfunction_008},
	{false, (void *)&Ebcscript_Dummyfunction_009},
	{false, (void *)&Ebcscript_Dummyfunction_00a},
	{false, (void *)&Ebcscript_Dummyfunction_00b},
	{false, (void *)&Ebcscript_Dummyfunction_00c},
	{false, (void *)&Ebcscript_Dummyfunction_00d},
	{false, (void *)&Ebcscript_Dummyfunction_00e},
	{false, (void *)&Ebcscript_Dummyfunction_00f},
	{false, (void *)&Ebcscript_Dummyfunction_010},
	{false, (void *)&Ebcscript_Dummyfunction_011},
	{false, (void *)&Ebcscript_Dummyfunction_012},
	{false, (void *)&Ebcscript_Dummyfunction_013},
	{false, (void *)&Ebcscript_Dummyfunction_014},
	{false, (void *)&Ebcscript_Dummyfunction_015},
	{false, (void *)&Ebcscript_Dummyfunction_016},
	{false, (void *)&Ebcscript_Dummyfunction_017},
	{false, (void *)&Ebcscript_Dummyfunction_018},
	{false, (void *)&Ebcscript_Dummyfunction_019},
	{false, (void *)&Ebcscript_Dummyfunction_01a},
	{false, (void *)&Ebcscript_Dummyfunction_01b},
	{false, (void *)&Ebcscript_Dummyfunction_01c},
	{false, (void *)&Ebcscript_Dummyfunction_01d},
	{false, (void *)&Ebcscript_Dummyfunction_01e},
	{false, (void *)&Ebcscript_Dummyfunction_01f},
};

/*                                                                   関数定義 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Functionmap_malloc(size_t Size)
{
	void *P;

	if ((P = malloc(Size + sizeof(size_t))) == NULL){
	  Ebcscript_Functionmap_log(
	   "Ebcscript_Functionmap_malloc(): "
	   "error: memory allocation error(total=%d bytes)\n",
	   Ebcscript_Functionmap_MallocTotal);
	  Ebcscript_Functionmap_exit(1);
	}
	*(size_t *)P = Size;
	Ebcscript_Functionmap_MallocTotal += Size;
	return (size_t *)P + 1;
}

void _Ebcscript_Functionmap_free(void *P)
{
	P = (size_t *)P - 1;
	Ebcscript_Functionmap_MallocTotal -= *(size_t *)P;
	free(P);
}

void Ebcscript_Functionmap_onexit(void)
{
	fclose(Ebcscript_Functionmap_Fplog);
}

void _Ebcscript_Functionmap_exit(int Status)
{
	Ebcscript_Functionmap_onexit();
	exit(Status);
}

void _Ebcscript_Functionmap_log(const char *Fmt, ...)
{
	va_list Args;

	fprintf(Ebcscript_Functionmap_Fplog, "ebcscript_functionmap: ");

	va_start(Args, Fmt);
	vfprintf(Ebcscript_Functionmap_Fplog, Fmt, Args);
	va_end(Args);
}
/* -------------------------------------------------------------------------- */
void *Ebcscript_Dummyfunction_getDummy(void)
{
	int I;

	for (I = 0; I < EBCSCRIPT_DUMMYFUNCTION_POOLSIZE; I++) {
	  if (!Ebcscript_Dummyfunction_Pool[I].InUse) {
	    Ebcscript_Dummyfunction_Pool[I].InUse = true;
	    return Ebcscript_Dummyfunction_Pool[I].FunctionID;
	  }
	}

	Ebcscript_Functionmap_log(
	 "Ebcscript_Dummyfunction_getDummy(): error: "
	 "dummy functions are out of stock\n");
	return NULL;
}

void Ebcscript_Dummyfunction_releaseDummy(void *FunctionID)
{
	int I;

	for (I = 0; I < EBCSCRIPT_DUMMYFUNCTION_POOLSIZE; I++) {
	  if (Ebcscript_Dummyfunction_Pool[I].FunctionID == FunctionID) {
	    Ebcscript_Dummyfunction_Pool[I].InUse = false;
	    return;
	  }
	}
}

boolean Ebcscript_Dummyfunction_isDummy(void *FunctionID)
{
	int I;

	for (I = 0; I < EBCSCRIPT_DUMMYFUNCTION_POOLSIZE; I++) {
	  if (Ebcscript_Dummyfunction_Pool[I].FunctionID == FunctionID) {
	    return true;
	  }
	}
	return false;
}
/* -------------------------------------------------------------------------- */
static
int find(ebcscript_functionmap *FMap, void *FunctionID)
{
	int I, Low, High;

	Low = 0, High = FMap->N;
	for (I = FMap->N / 2; Low != High; I = Low + (High - Low) / 2) {
	  if (FMap->Table[I].FunctionID == FunctionID) {
	    return I;
	  } else if (FMap->Table[I].FunctionID < FunctionID) {
	    Low = I + 1;
	  } else {
	    High = I;
	  }
	}
	return -1;
}

static
void resize(ebcscript_functionmap *FMap)
{
	int NewTableSize, OldTableSize;
	ebcscript_functionmap_entry *NewTable, *OldTable;

	OldTableSize = FMap->TableSize;
	OldTable     = FMap->Table;

	NewTableSize = OldTableSize * 2;
	NewTable     = Ebcscript_Functionmap_malloc(
	                             OldTableSize * 2 * sizeof(FMap->Table[0]));

	memcpy(NewTable, OldTable, OldTableSize * sizeof(FMap->Table[0]));

	Ebcscript_Functionmap_free(OldTable);

	FMap->TableSize = NewTableSize;
	FMap->Table     = NewTable;
}
/* -------------------------------------------------------------------------- */
ebcscript_functionmap *Ebcscript_newFunctionmap(void)
{
	ebcscript_functionmap *FMap;

	FMap = Ebcscript_Functionmap_malloc(sizeof(ebcscript_functionmap));
	FMap->TableSize = EBCSCRIPT_FUNCTIONMAP_SIZE_INITIAL;
	FMap->Table = Ebcscript_Functionmap_malloc(
	                 sizeof(ebcscript_functionmap_entry) * FMap->TableSize);
	FMap->N = 0;
	return FMap;
}

void Ebcscript_deleteFunctionmap(ebcscript_functionmap *FMap)
{
	Ebcscript_Functionmap_free(FMap->Table);
	Ebcscript_Functionmap_free(FMap);
}

ebcscript_functionmap_entry *Ebcscript_Functionmap_find(
                                 ebcscript_functionmap *FMap, void *FunctionID)
{
	int I;

	if (0 <= (I = find(FMap, FunctionID))) {
	  return &FMap->Table[I];
	}
	return NULL;
}

boolean Ebcscript_Functionmap_add(
 ebcscript_functionmap *FMap,
 void *FunctionID, void *CodeAddress, boolean IsNative
)
{
	int I;

	if (0 <= find(FMap, FunctionID))
	  return false;

	if (FMap->N >= FMap->TableSize) {
	  resize(FMap);
	}

	for (I = FMap->N; I >= 1; I--) {
	  if (FMap->Table[I - 1].FunctionID <= FunctionID) {
	    break;
	  }
	  FMap->Table[I] = FMap->Table[I - 1];
	}

	FMap->Table[I].FunctionID  = FunctionID;
	FMap->Table[I].CodeAddress = CodeAddress;
	FMap->Table[I].IsNative    = IsNative;
	FMap->N++;
	return true;
}

void Ebcscript_Functionmap_remove(
                                 ebcscript_functionmap *FMap, void *FunctionID)
{
	int I;

	if (0 > (I = find(FMap, FunctionID)))
	  return;

	for (; I < FMap->N - 1; I++) {
	  FMap->Table[I] = FMap->Table[I + 1];
	}

	FMap->N--;
}

void Ebcscript_Functionmap_Entry_dump(ebcscript_functionmap_entry *FInfo)
{
	fprintf(Ebcscript_Functionmap_Fplog, "{%p, %p, %d}",
	 FInfo->FunctionID,
	 FInfo->CodeAddress,
	 FInfo->IsNative
	);
}

void Ebcscript_Functionmap_dump(ebcscript_functionmap *FMap)
{
	int I;

	fprintf(Ebcscript_Functionmap_Fplog,
	 "N = %d, TableSize = %d\n", FMap->N, FMap->TableSize);
	for (I = 0; I < FMap->N; I++) {
	  fprintf(Ebcscript_Functionmap_Fplog, "[%d]: ", I);
	  Ebcscript_Functionmap_Entry_dump(&FMap->Table[I]);
	  fprintf(Ebcscript_Functionmap_Fplog, "\n");
	}
}
/* -------------------------------------------------------------------------- */
#ifdef DEBUG
int main(int argc, char *argv[])
{
	ebcscript_functionmap *FMap;
	ebcscript_functionmap_entry *FInfo;
	void *ID;
	int I;

	FMap = Ebcscript_newFunctionmap();

	Ebcscript_Functionmap_add(FMap, (void *)2, NULL, false);
	Ebcscript_Functionmap_add(FMap, (void *)5, NULL, false);
	Ebcscript_Functionmap_add(FMap, (void *)30, NULL, false);
	Ebcscript_Functionmap_add(FMap, (void *)7, NULL, false);
	Ebcscript_Functionmap_add(FMap, (void *)9, NULL, false);
	Ebcscript_Functionmap_add(FMap, (void *)1, NULL, false);

	Ebcscript_Functionmap_remove(FMap, (void *)30);
	Ebcscript_Functionmap_remove(FMap, (void *)1);

	Ebcscript_Functionmap_dump(FMap);

	FInfo = Ebcscript_Functionmap_find(FMap, (void *)2);
	printf("%p\n", FInfo->FunctionID);

	Ebcscript_deleteFunctionmap(FMap);
	return 0;
}
#endif
