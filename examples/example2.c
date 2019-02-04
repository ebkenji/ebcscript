#include <stdio.h>
#include <math.h>
#include "ebcscrip.h"
#include "name.h"

void sqrt_callee(void *stack)
{
	double x, r;

	x = *(double *)stack; stack += sizeof(double);
	r = sqrt(x);
	*(double *)stack= r;
}

int main(void)
{
	ebcscript *env;
	double x, y, r;

	Ebcscript_setFplog(stdout);

	env = newEbcscript(1024);
	Ebcscript_addTrnsunit(env, "magnitud.c");
	Ebcscript_addFunction(env,
	 "sqrt",
	 Ebcscript_makeType_function(
	             Ebcscript_newType_double(), 1, Ebcscript_newType_double()),
	 &sqrt,
	 &sqrt_callee);
	Ebcscript_resolve(env);

	x = 3.0, y = 4.0;
	Ebcscript_sub_sp(env, sizeof(double));
	Ebcscript_push_double(env, y);
	Ebcscript_push_double(env, x);
	Ebcscript_call(env, "magnitude");
	Ebcscript_add_sp(env, sizeof(double) + sizeof(double));
	Ebcscript_pop_double(env, &r);

	printf("magnitude(%f, %f) = %f\n", x, y, r);

	Ebcscript_removeFunction(env, "sqrt");
	Ebcscript_removeTrnsunit(env, "magnitud.c");
	deleteEbcscript(env);
	return 0;
}
