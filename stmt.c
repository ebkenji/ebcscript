/*******************************************************************************
   Project : C script
   File    : stmt.c
   Date    : 2018.12.5-
   Note    : 制御構文のコード生成のための補助処理
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include "parser.h"
#include "trnsunit.h"
#include "stmt.h"
#include "name.h"
#include "slist.h"
#include "hashmap.h"
#include "boolean.h"

/*                                                                   定数定義 */
/* -------------------------------------------------------------------------- */

/*                                                             グローバル変数 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Parser_Statement_malloc(size_t Size);
void _Ebcscript_Parser_Statement_free(void *P);
void _Ebcscript_Parser_Statement_exit(int Status);
void _Ebcscript_Parser_Statement_log(const char *Fmt, ...);

FILE *Ebcscript_Parser_Statement_Fplog/* = stdout*/;
size_t Ebcscript_Parser_Statement_MallocTotal = 0;
void *(*Ebcscript_Parser_Statement_malloc)(size_t Size) =
                                            &_Ebcscript_Parser_Statement_malloc;
void (*Ebcscript_Parser_Statement_free)(void *) =
                                              &_Ebcscript_Parser_Statement_free;
void (*Ebcscript_Parser_Statement_exit)(int) =
                                              &_Ebcscript_Parser_Statement_exit;
void (*Ebcscript_Parser_Statement_log)(const char *Fmt, ...) =
                                               &_Ebcscript_Parser_Statement_log;

/*                                                                   関数定義 */
/* -------------------------------------------------------------------------- */
void *_Ebcscript_Parser_Statement_malloc(size_t Size)
{
	void *P;

	if ((P = malloc(Size + sizeof(size_t))) == NULL){
	  Ebcscript_Parser_Statement_log(
	   "Ebcscript_Parser_Statement_malloc(): "
	   "error: memory allocation error(total=%d bytes)\n",
	   Ebcscript_Parser_Statement_MallocTotal);
	  Ebcscript_Parser_Statement_exit(1);
	}
	*(size_t *)P = Size;
	Ebcscript_Parser_Statement_MallocTotal += Size;
	return (size_t *)P + 1;
}

void _Ebcscript_Parser_Statement_free(void *P)
{
	P = (size_t *)P - 1;
	Ebcscript_Parser_Statement_MallocTotal -= *(size_t *)P;
	free(P);
}

void Ebcscript_Parser_Statement_onexit(void)
{
	fclose(Ebcscript_Parser_Statement_Fplog);
}

void _Ebcscript_Parser_Statement_exit(int Status)
{
	Ebcscript_Parser_Statement_onexit();
	exit(Status);
}

void _Ebcscript_Parser_Statement_log(const char *Fmt, ...)
{
	va_list Args;

	fprintf(Ebcscript_Parser_Statement_Fplog,
	                                        "ebcscript_parser_statement: ");

	va_start(Args, Fmt);
	vfprintf(Ebcscript_Parser_Statement_Fplog, Fmt, Args);
	va_end(Args);
}
/* -------------------------------------------------------------------------- */
ebcscript_parser_blockstack *Ebcscript_Parser_newBlockstack(void)
{
	ebcscript_parser_blockstack *BS;

	BS = Ebcscript_Parser_Statement_malloc(
	                                   sizeof(ebcscript_parser_blockstack));
	BS->Prev = NULL;
/*	BS->Nest = 1;*/
	return BS;
}

void Ebcscript_Parser_deleteBlockstack(ebcscript_parser_blockstack *BS)
{
	switch (BS->Kind) {
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_IF:
	    Ebcscript_Parser_deleteBlockstack_if(BS);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_LOOP:
	    Ebcscript_Parser_deleteBlockstack_loop(BS);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_SWITCH:
	    Ebcscript_Parser_deleteBlockstack_switch(BS);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION:
	    Ebcscript_Parser_deleteBlockstack_function(BS);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK:
	    Ebcscript_Parser_deleteBlockstack_block(BS);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT:
	    Ebcscript_Parser_deleteBlockstack_trnsunit(BS);
	    break;
	  default:
	    break;
	}
	Ebcscript_Parser_Statement_free(BS);
}

void Ebcscript_Parser_resolve(
                         ebcscript_parser *Prs, ebcscript_parser_blockstack *BS)
{
	switch (BS->Kind) {
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_IF:
	    Ebcscript_Parser_resolve_if(Prs, BS);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_LOOP:
	    Ebcscript_Parser_resolve_loop(Prs, BS);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_SWITCH:
	    Ebcscript_Parser_resolve_switch(Prs, BS);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION:
	    Ebcscript_Parser_resolve_function(Prs, BS);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK:
	    Ebcscript_Parser_resolve_block(Prs, BS);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT:
	    Ebcscript_Parser_resolve_trnsunit(Prs, BS);
	    break;
	  default:
	    break;
	}
}

ebcscript_name **Ebcscript_Parser_Blockstack_findVarFuncTypeEnum(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	ebcscript_name **N0;

	switch (BS->Kind) {
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_IF:
	    N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_if(BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_LOOP:
	    N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_loop(BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_SWITCH:
	    N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_switch(
	                                                               BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION:
	    N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_function(
	                                                               BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK:
	    N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_block(
	                                                               BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT:
	    N0 = Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_trnsunit(
	                                                               BS, Key);
	    break;
	  default:
	    N0 = NULL;
	    break;
	}
	return N0;
}

ebcscript_name **Ebcscript_Parser_Blockstack_findTag(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	ebcscript_name **N0;

	switch (BS->Kind) {
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_IF:
	    N0 = Ebcscript_Parser_Blockstack_findTag_if(BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_LOOP:
	    N0 = Ebcscript_Parser_Blockstack_findTag_loop(BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_SWITCH:
	    N0 = Ebcscript_Parser_Blockstack_findTag_switch(BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION:
	    N0 = Ebcscript_Parser_Blockstack_findTag_function(BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK:
	    N0 = Ebcscript_Parser_Blockstack_findTag_block(BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT:
	    N0 = Ebcscript_Parser_Blockstack_findTag_trnsunit(BS, Key);
	    break;
	  default:
	    N0 = NULL;
	    break;
	}
	return N0;
}

ebcscript_name **Ebcscript_Parser_Blockstack_findLabel(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	ebcscript_name **N0;

	switch (BS->Kind) {
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_IF:
	    N0 = Ebcscript_Parser_Blockstack_findLabel_if(BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_LOOP:
	    N0 = Ebcscript_Parser_Blockstack_findLabel_loop(BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_SWITCH:
	    N0 = Ebcscript_Parser_Blockstack_findLabel_switch(BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION:
	    N0 = Ebcscript_Parser_Blockstack_findLabel_function(BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK:
	    N0 = Ebcscript_Parser_Blockstack_findLabel_block(BS, Key);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT:
	    N0 = Ebcscript_Parser_Blockstack_findLabel_trnsunit(BS, Key);
	    break;
	  default:
	    N0 = NULL;
	    break;
	}
	return N0;
}

boolean Ebcscript_Parser_Blockstack_addVarFuncTypeEnum(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	boolean B;

	switch (BS->Kind) {
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_IF:
	    B = Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_if(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_LOOP:
	    B = Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_loop(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_SWITCH:
	    B = Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_switch(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION:
	    B = Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_function(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK:
	    B = Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_block(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT:
	    B = Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_trnsunit(BS, N);
	    break;
	  default:
	    B = false;
	    break;
	}
	return B;
}

boolean Ebcscript_Parser_Blockstack_addTag(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	boolean B;

	switch (BS->Kind) {
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_IF:
	    B = Ebcscript_Parser_Blockstack_addTag_if(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_LOOP:
	    B = Ebcscript_Parser_Blockstack_addTag_loop(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_SWITCH:
	    B = Ebcscript_Parser_Blockstack_addTag_switch(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION:
	    B = Ebcscript_Parser_Blockstack_addTag_function(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK:
	    B = Ebcscript_Parser_Blockstack_addTag_block(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT:
	    B = Ebcscript_Parser_Blockstack_addTag_trnsunit(BS, N);
	    break;
	  default:
	    B = false;
	    break;
	}
	return B;
}

boolean Ebcscript_Parser_Blockstack_addLabel(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	boolean B;

	switch (BS->Kind) {
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_IF:
	    B = Ebcscript_Parser_Blockstack_addLabel_if(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_LOOP:
	    B = Ebcscript_Parser_Blockstack_addLabel_loop(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_SWITCH:
	    B = Ebcscript_Parser_Blockstack_addLabel_switch(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION:
	    B = Ebcscript_Parser_Blockstack_addLabel_function(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK:
	    B = Ebcscript_Parser_Blockstack_addLabel_block(BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT:
	    B = Ebcscript_Parser_Blockstack_addLabel_trnsunit(BS, N);
	    break;
	  default:
	    B = false;
	    break;
	}
	return B;
}

void Ebcscript_Parser_store_address_n(
      ebcscript_parser *Prs, ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	switch (BS->Kind) {
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_IF:
	    Ebcscript_Parser_store_address_n_if(Prs, BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_LOOP:
	    Ebcscript_Parser_store_address_n_loop(Prs, BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_SWITCH:
	    Ebcscript_Parser_store_address_n_switch(Prs, BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION:
	    Ebcscript_Parser_store_address_n_function(Prs, BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK:
	    Ebcscript_Parser_store_address_n_block(Prs, BS, N);
	    break;
	  case EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT:
	    Ebcscript_Parser_store_address_n_trnsunit(Prs, BS, N);
	    break;
	  default:
	    break;
	}
	return;
}

/* -------------------------------------------------------------------------- */
ebcscript_parser_blockstack *Ebcscript_Parser_newBlockstack_if(void)
{
	ebcscript_parser_blockstack *BS;

	BS = Ebcscript_Parser_newBlockstack();
	BS->Kind = EBCSCRIPT_PARSER_BLOCKSTACK_KIND_IF;

	BS->As.If.LEndif = Ebcscript_newName_label("$endif");
	BS->As.If.LElse  = Ebcscript_newName_label("$else");
	BS->As.If.UnresolvedList = newSList();

	return BS;
}

void Ebcscript_Parser_deleteBlockstack_if(ebcscript_parser_blockstack *BS)
{
	Ebcscript_deleteName(BS->As.If.LEndif);
	Ebcscript_deleteName(BS->As.If.LElse);
	SList_clear(BS->As.If.UnresolvedList,
	                          (void (*)(void *))Ebcscript_deleteUnresolved);
	deleteSList(BS->As.If.UnresolvedList);
}

void Ebcscript_Parser_resolve_if(
                         ebcscript_parser *Prs, ebcscript_parser_blockstack *BS)
{
	slist_cell *P;
	ebcscript_unresolved *UR;

	for (P = BS->As.If.UnresolvedList->Head.Next;
	     P != NULL;
	     P = P->Next) {
	  UR = (ebcscript_unresolved *)P->Datum;

	  if (UR->N->As.Label.Addressing == EBCSCRIPT_NAME_ADDRESSING_ONCODE) {
	    *(void **)((ptrdiff_t)UR->CP + Prs->TU->Code) = (void *)
	                                 (UR->N->As.Label.CodeAddress - UR->CP);
	  }
	}
}

void Ebcscript_Parser_store_address_n_if(
      ebcscript_parser *Prs, ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	SList_addFront(BS->As.If.UnresolvedList,
	 Ebcscript_newUnresolved((void *)(Prs->TU->CP - Prs->TU->Code), N)
	);
	Ebcscript_Trnsunit_store_address(Prs->TU, NULL);
}

ebcscript_name **Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_if(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	return NULL;
}

ebcscript_name **Ebcscript_Parser_Blockstack_findTag_if(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	return NULL;
}

ebcscript_name **Ebcscript_Parser_Blockstack_findLabel_if(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	if (!strcmp(Key, BS->As.If.LElse->Identifier)) {
	  return &BS->As.If.LElse;
	} else
	if (!strcmp(Key, BS->As.If.LEndif->Identifier)) {
	  return &BS->As.If.LEndif;
	}
	return NULL;
}

boolean Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_if(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return false;
}

boolean Ebcscript_Parser_Blockstack_addTag_if(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return false;
}

boolean Ebcscript_Parser_Blockstack_addLabel_if(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return false;
}
/* -------------------------------------------------------------------------- */
ebcscript_parser_blockstack *Ebcscript_Parser_newBlockstack_loop(void)
{
	ebcscript_parser_blockstack *BS;

	BS = Ebcscript_Parser_newBlockstack();
	BS->Kind = EBCSCRIPT_PARSER_BLOCKSTACK_KIND_LOOP;

	BS->As.Loop.LBreak    = Ebcscript_newName_label("$break");
	BS->As.Loop.LContinue = Ebcscript_newName_label("$continue");
	BS->As.Loop.LBegin    = Ebcscript_newName_label("$begin");
	BS->As.Loop.UnresolvedList = newSList();

	return BS;
}

void Ebcscript_Parser_deleteBlockstack_loop(ebcscript_parser_blockstack *BS)
{
	Ebcscript_deleteName(BS->As.Loop.LBreak);
	Ebcscript_deleteName(BS->As.Loop.LContinue);
	Ebcscript_deleteName(BS->As.Loop.LBegin);

	SList_clear(BS->As.Loop.UnresolvedList,
	                          (void (*)(void *))Ebcscript_deleteUnresolved);
	deleteSList(BS->As.Loop.UnresolvedList);
}

void Ebcscript_Parser_resolve_loop(
                         ebcscript_parser *Prs, ebcscript_parser_blockstack *BS)
{
	slist_cell *P;
	ebcscript_unresolved *UR;

	for (P = BS->As.Loop.UnresolvedList->Head.Next;
	     P != NULL;
	     P = P->Next) {
	  UR = (ebcscript_unresolved *)P->Datum;

	  if (UR->N->As.Label.Addressing == EBCSCRIPT_NAME_ADDRESSING_ONCODE) {
	    *(void **)((ptrdiff_t)UR->CP + Prs->TU->Code) = (void *)
	                                 (UR->N->As.Label.CodeAddress - UR->CP);
	  }
	}
}

void Ebcscript_Parser_store_address_n_loop(
      ebcscript_parser *Prs, ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	SList_addFront(BS->As.Loop.UnresolvedList,
	 Ebcscript_newUnresolved((void *)(Prs->TU->CP - Prs->TU->Code), N)
	);
	Ebcscript_Trnsunit_store_address(Prs->TU, NULL);
}

ebcscript_name **Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_loop(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	return NULL;
}

ebcscript_name **Ebcscript_Parser_Blockstack_findTag_loop(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	return NULL;
}

ebcscript_name **Ebcscript_Parser_Blockstack_findLabel_loop(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	if (!strcmp(Key, BS->As.Loop.LBreak->Identifier)) {
	  return &BS->As.Loop.LBreak;
	} else
	if (!strcmp(Key, BS->As.Loop.LBegin->Identifier)) {
	  return &BS->As.Loop.LBegin;
	} else
	if (!strcmp(Key, BS->As.Loop.LContinue->Identifier)) {
	  return &BS->As.Loop.LContinue;
	}
	return NULL;
}

boolean Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_loop(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return false;
}

boolean Ebcscript_Parser_Blockstack_addTag_loop(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return false;
}

boolean Ebcscript_Parser_Blockstack_addLabel_loop(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return false;
}

/* -------------------------------------------------------------------------- */
ebcscript_parser_blockstack *Ebcscript_Parser_newBlockstack_switch(void)
{
	ebcscript_parser_blockstack *BS;

	BS = Ebcscript_Parser_newBlockstack();
	BS->Kind = EBCSCRIPT_PARSER_BLOCKSTACK_KIND_SWITCH;

	BS->As.Switch.LBreak   = Ebcscript_newName_label("$break");
	BS->As.Switch.LBegin   = Ebcscript_newName_label("$begin");
	BS->As.Switch.LDefault = Ebcscript_newName_label("$default");
	BS->As.Switch.NSelector = Ebcscript_newName("$selector");
	BS->As.Switch.LCase = newSList();
	BS->As.Switch.UnresolvedList = newSList();

	return BS;
}

void Ebcscript_Parser_deleteBlockstack_switch(ebcscript_parser_blockstack *BS)
{
	Ebcscript_deleteName(BS->As.Switch.LBreak);
	Ebcscript_deleteName(BS->As.Switch.LBegin);
	Ebcscript_deleteName(BS->As.Switch.LDefault);
	Ebcscript_deleteName(BS->As.Switch.NSelector);

	SList_clear(BS->As.Switch.LCase,
	                                (void (*)(void *))Ebcscript_deleteName);
	deleteSList(BS->As.Switch.LCase);

	SList_clear(BS->As.Switch.UnresolvedList,
	                          (void (*)(void *))Ebcscript_deleteUnresolved);
	deleteSList(BS->As.Switch.UnresolvedList);
}

void Ebcscript_Parser_resolve_switch(
                         ebcscript_parser *Prs, ebcscript_parser_blockstack *BS)
{
	slist_cell *P;
	ebcscript_unresolved *UR;

	for (P = BS->As.Switch.UnresolvedList->Head.Next;
	     P != NULL;
	     P = P->Next) {
	  UR = (ebcscript_unresolved *)P->Datum;

	  if (UR->N->As.Label.Addressing == EBCSCRIPT_NAME_ADDRESSING_ONCODE) {
	    *(void **)((ptrdiff_t)UR->CP + Prs->TU->Code) = (void *)
	                                 (UR->N->As.Label.CodeAddress - UR->CP);
	  }
	}
}

void Ebcscript_Parser_store_address_n_switch(
      ebcscript_parser *Prs, ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	SList_addFront(BS->As.Switch.UnresolvedList,
	 Ebcscript_newUnresolved((void *)(Prs->TU->CP - Prs->TU->Code), N)
	);
	Ebcscript_Trnsunit_store_address(Prs->TU, NULL);
}

ebcscript_name **Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_switch(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	if (!strcmp(Key, BS->As.Switch.NSelector->Identifier)) {
	  return &BS->As.Switch.NSelector;
	}
	return NULL;
}

ebcscript_name **Ebcscript_Parser_Blockstack_findTag_switch(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	return NULL;
}

ebcscript_name **Ebcscript_Parser_Blockstack_findLabel_switch(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	if (!strcmp(Key, BS->As.Switch.LBreak->Identifier)) {
	  return &BS->As.Switch.LBreak;
	} else
	if (!strcmp(Key, BS->As.Switch.LBegin->Identifier)) {
	  return &BS->As.Switch.LBegin;
	} else
	if (!strcmp(Key, BS->As.Switch.LDefault->Identifier)) {
	  return &BS->As.Switch.LDefault;
	}
	return NULL;
}

boolean Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_switch(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return false;
}

boolean Ebcscript_Parser_Blockstack_addTag_switch(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return false;
}

boolean Ebcscript_Parser_Blockstack_addLabel_switch(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return false;
}
/* -------------------------------------------------------------------------- */
ebcscript_parser_blockstack *Ebcscript_Parser_newBlockstack_function(void)
{
	ebcscript_parser_blockstack *BS;

	BS = Ebcscript_Parser_newBlockstack();
	BS->Kind = EBCSCRIPT_PARSER_BLOCKSTACK_KIND_FUNCTION;

	BS->As.Function.LReturn = Ebcscript_newName_label("$return");
	BS->As.Function.UnresolvedReturnList = newSList();

	BS->As.Function.ReturnValue = NULL;
	BS->As.Function.ReturnValueSize = 0;
	BS->As.Function.ParametersSize = 0;

	BS->As.Function.LGoto = newHashmap();
	BS->As.Function.UnresolvedGotoList = newSList();

	return BS;
}

void Ebcscript_Parser_deleteBlockstack_function(
                                               ebcscript_parser_blockstack *BS)
{
	Ebcscript_deleteName(BS->As.Function.LReturn);

	SList_clear(BS->As.Function.UnresolvedReturnList,
	                          (void (*)(void *))Ebcscript_deleteUnresolved);
	deleteSList(BS->As.Function.UnresolvedReturnList);

	SList_clear(BS->As.Function.UnresolvedGotoList,
	                          (void (*)(void *))Ebcscript_deleteUnresolved);
	deleteSList(BS->As.Function.UnresolvedGotoList);

	Hashmap_clear(BS->As.Function.LGoto,
	                                (void (*)(void *))Ebcscript_deleteName);
	deleteHashmap(BS->As.Function.LGoto);
}

void Ebcscript_Parser_resolve_function(
                         ebcscript_parser *Prs, ebcscript_parser_blockstack *BS)
{
	{
	  slist_cell *P;
	  ebcscript_unresolved *UR;

	  for (P = BS->As.Function.UnresolvedReturnList->Head.Next;
	       P != NULL;
	       P = P->Next) {
	    UR = (ebcscript_unresolved *)P->Datum;

	    if (UR->N->As.Label.Addressing ==
	                                     EBCSCRIPT_NAME_ADDRESSING_ONCODE) {
	      *(void **)((ptrdiff_t)UR->CP + Prs->TU->Code) = (void *)
	                                 (UR->N->As.Label.CodeAddress - UR->CP);
	    }
	  }
	}
	{
	  slist_cell *P;
	  ebcscript_unresolved *UR;
	  int Nest;
	  void *CP;

	  for (P = BS->As.Function.UnresolvedGotoList->Head.Next;
	       P != NULL;
	       P = P->Next) {
	    UR = (ebcscript_unresolved *)P->Datum;

	    if (UR->N->As.Label.Addressing ==
	                                  EBCSCRIPT_NAME_ADDRESSING_UNDEFINED) {
	      Ebcscript_Parser_Statement_log(
	       "error: label \'%s\' used but not defined\n", UR->N->Identifier);
	      longjmp(Prs->CheckPoint, 1);
	      continue;
	    }
	    if (UR->N->As.Label.Addressing ==
	                                     EBCSCRIPT_NAME_ADDRESSING_ONCODE) {
	      CP = (ptrdiff_t)UR->CP + Prs->TU->Code;
	      for (Nest = UR->Nest;
	           Nest > UR->N->As.Label.Nest;
	           Nest--) {
	        *(ebcscript_instruction *)CP = EBCSCRIPT_INSTRUCTION_MOV_BP_SP;
	        CP += sizeof(ebcscript_instruction);
	        *(ebcscript_instruction *)CP = EBCSCRIPT_INSTRUCTION_POP_BP;
	        CP += sizeof(ebcscript_instruction);
	      }
	      *(ebcscript_instruction *)CP = EBCSCRIPT_INSTRUCTION_JMP;
	      CP += sizeof(ebcscript_instruction);
	      *(void **)CP = (void *)
	                      (UR->N->As.Label.CodeAddress - CP + Prs->TU->Code);
	    }
	  }
	}
}

void Ebcscript_Parser_store_address_n_function(
      ebcscript_parser *Prs, ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	SList_addFront(BS->As.Function.UnresolvedReturnList,
	 Ebcscript_newUnresolved((void *)(Prs->TU->CP - Prs->TU->Code), N)
	);
	Ebcscript_Trnsunit_store_address(Prs->TU, NULL);
}

ebcscript_name **Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_function(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	return NULL;
}

ebcscript_name **Ebcscript_Parser_Blockstack_findTag_function(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	return NULL;
}

ebcscript_name **Ebcscript_Parser_Blockstack_findLabel_function(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	if (!strcmp(Key, BS->As.Function.LReturn->Identifier)) {
	  return &BS->As.Function.LReturn;
	} else {
	  return (ebcscript_name **)Hashmap_find(BS->As.Function.LGoto, Key);
	}
}

boolean Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_function(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return false;
}

boolean Ebcscript_Parser_Blockstack_addTag_function(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return false;
}

boolean Ebcscript_Parser_Blockstack_addLabel_function(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return Hashmap_add(BS->As.Function.LGoto, N->Identifier, N);
}
/* -------------------------------------------------------------------------- */
ebcscript_parser_blockstack *Ebcscript_Parser_newBlockstack_block(void)
{
	ebcscript_parser_blockstack *BS;

	BS = Ebcscript_Parser_newBlockstack();
	BS->Kind = EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK;

	BS->As.Block.DP = NULL;

	BS->As.Block.FrameSize = 0;
	BS->As.Block.UnresolvedFrameSizeList = newSList();

	BS->As.Block.NSVarFuncTypeEnum = newHashmap();
	BS->As.Block.NSTag = newHashmap();
/*	BS->As.Block.UnresolvedList = newSList();*/

	return BS;
}

void Ebcscript_Parser_deleteBlockstack_block(ebcscript_parser_blockstack *BS)
{
	SList_clear(BS->As.Block.UnresolvedFrameSizeList,
	                          (void (*)(void *))Ebcscript_deleteUnresolved);
	deleteSList(BS->As.Block.UnresolvedFrameSizeList);

/*	SList_clear(BS->As.Block.UnresolvedList,
	                          (void (*)(void *))Ebcscript_deleteUnresolved);
	deleteSList(BS->As.Block.UnresolvedList);*/
	Hashmap_clear(BS->As.Block.NSVarFuncTypeEnum,
	                                (void (*)(void *))Ebcscript_deleteName);
	deleteHashmap(BS->As.Block.NSVarFuncTypeEnum);
	Hashmap_clear(BS->As.Block.NSTag,
	                                (void (*)(void *))Ebcscript_deleteName);
	deleteHashmap(BS->As.Block.NSTag);
}

void *Ebcscript_Parser_Blockstack_mallocL(
                                  ebcscript_parser_blockstack *BS, size_t Size)
{
	void *P;

	if (BS->Kind == EBCSCRIPT_PARSER_BLOCKSTACK_KIND_BLOCK) {

/*	  BS->As.Block.DP += Ebcscript_Name_align(BS->As.Block.DP, Align = 4);*/
	  BS->As.Block.DP += Size;
	  P = (void *)-(ptrdiff_t)BS->As.Block.DP;

	}
	return P;
}

void Ebcscript_Parser_resolve_block(
                         ebcscript_parser *Prs, ebcscript_parser_blockstack *BS)
{
	ebcscript_unresolved *UR;
	slist_cell *P;

	for (P = BS->As.Block.UnresolvedFrameSizeList->Head.Next;
	     P != NULL;
	     P = P->Next) {
	  UR = (ebcscript_unresolved *)P->Datum;

	  *(int *)((ptrdiff_t)UR->CP + Prs->TU->Code) = BS->As.Block.FrameSize;
	}
}

void Ebcscript_Parser_store_address_n_block(
      ebcscript_parser *Prs, ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	Ebcscript_Trnsunit_store_address_n(Prs->TU, N);
	return;
}

ebcscript_name **Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_block(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	return (ebcscript_name **)
	                       Hashmap_find(BS->As.Block.NSVarFuncTypeEnum, Key);
}

ebcscript_name **Ebcscript_Parser_Blockstack_findTag_block(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	return (ebcscript_name **)Hashmap_find(BS->As.Block.NSTag, Key);
}

ebcscript_name **Ebcscript_Parser_Blockstack_findLabel_block(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	return NULL;
}

boolean Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_block(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return Hashmap_add(BS->As.Block.NSVarFuncTypeEnum, N->Identifier, N);
}

boolean Ebcscript_Parser_Blockstack_addTag_block(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return Hashmap_add(BS->As.Block.NSTag, N->Identifier, N);
}

boolean Ebcscript_Parser_Blockstack_addLabel_block(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return false;
}

/* -------------------------------------------------------------------------- */
ebcscript_parser_blockstack *Ebcscript_Parser_newBlockstack_trnsunit(void)
{
	ebcscript_parser_blockstack *BS;

	BS = Ebcscript_Parser_newBlockstack();
	BS->Kind = EBCSCRIPT_PARSER_BLOCKSTACK_KIND_TRNSUNIT;

	return BS;
}

void Ebcscript_Parser_deleteBlockstack_trnsunit(ebcscript_parser_blockstack *BS)
{
	return;
}

void Ebcscript_Parser_resolve_trnsunit(
                         ebcscript_parser *Prs, ebcscript_parser_blockstack *BS)
{
	/* blockstackとしてはUnresolvedListを持っていないので何もしない */
	return;
}

void Ebcscript_Parser_store_address_n_trnsunit(
      ebcscript_parser *Prs, ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	Ebcscript_Trnsunit_store_address_n(Prs->TU, N);
	return;
}

ebcscript_name **Ebcscript_Parser_Blockstack_findVarFuncTypeEnum_trnsunit(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	return (ebcscript_name **)
	                Hashmap_find(BS->As.Trnsunit.TU->NSVarFuncTypeEnum, Key);
}

ebcscript_name **Ebcscript_Parser_Blockstack_findTag_trnsunit(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	return (ebcscript_name **)Hashmap_find(BS->As.Trnsunit.TU->NSTag, Key);
}

ebcscript_name **Ebcscript_Parser_Blockstack_findLabel_trnsunit(
                                    ebcscript_parser_blockstack *BS, char *Key)
{
	return NULL;
}

boolean Ebcscript_Parser_Blockstack_addVarFuncTypeEnum_trnsunit(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return Hashmap_add(BS->As.Trnsunit.TU->NSVarFuncTypeEnum,
	                                                       N->Identifier, N);
}

boolean Ebcscript_Parser_Blockstack_addTag_trnsunit(
                           ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return Hashmap_add(BS->As.Trnsunit.TU->NSTag, N->Identifier, N);
}

boolean Ebcscript_Parser_Blockstack_addLabel_trnsunit(
                            ebcscript_parser_blockstack *BS, ebcscript_name *N)
{
	return false;
}
/* -------------------------------------------------------------------------- */
#ifdef DEBUG
int main(int argc, char *argv[])
{
	return 0;
}
#endif
