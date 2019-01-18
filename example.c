#include <stdio.h>
#include "ebcscrip.h"

int main(void)
{
	ebcscript *env;
	char n;
	int v;

	Ebcscript_setFplog(stdout);

	env = newEbcscript(1024);
	Ebcscript_addTrnsunit(env, "factoria.c");
	Ebcscript_resolve(env);

	n = 5;
	Ebcscript_sub_sp(env, sizeof(int));
	Ebcscript_push_char(env, n);
	Ebcscript_call(env, "factorial");
	Ebcscript_add_sp(env, sizeof(char));
	Ebcscript_pop_int(env, &v);

	printf("5! = %d\n", v);

	Ebcscript_removeTrnsunit(env, "factoria.c");
	deleteEbcscript(env);
	return 0;
}
