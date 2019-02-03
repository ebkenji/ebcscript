#include <stdio.h>
#include "ebcscrip.h"
#include "name.h"

ebcscript *env;

void print(char *s)
{
	printf("%s", s);
}

void print_callee(void *stack)
{
	print(*(char **)stack);
}

void run(void (*pf[])())
{
	(*pf[0])();
	(*pf[1])();
}

void run_callee(void *stack)
{
	run(*(void (***)())stack);
}

void function2(void)
{
	print("function2()\n");
}

void function2_callee(void)
{
	function2();
}

void function1_caller(void)
{
	Ebcscript_call(env, "function1");
}

int main(void)
{
	Ebcscript_setFplog(stdout);

	env = newEbcscript(1024);
	Ebcscript_addTrnsunit(env, "script.c");

	Ebcscript_bindFunction(env, "script.c", "function1", &function1_caller);

	Ebcscript_addFunction(env, "print",
	 Ebcscript_makeType_function(Ebcscript_newType_void(), 1,
	                  Ebcscript_makeType_pointer(Ebcscript_newType_char())),
	 &print,
	 &print_callee
	);

	Ebcscript_addFunction(env, "run",
	 Ebcscript_makeType_function(Ebcscript_newType_void(), 1,
	          Ebcscript_makeType_array(
	           Ebcscript_makeType_pointer(
	            Ebcscript_makeType_function(Ebcscript_newType_void(), 0)))),
	 &run,
	 &run_callee
	);

	Ebcscript_addFunction(env, "function2",
	 Ebcscript_makeType_function(Ebcscript_newType_void(), 0),
	 &function2,
	 &function2_callee
	);

	Ebcscript_resolve(env);

	Ebcscript_call(env, "script");

	deleteEbcscript(env);
	return 0;
}
