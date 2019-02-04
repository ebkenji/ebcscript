extern void print(char *s);
extern void run(void (*pf[])());
extern void function2();

void (*pf[2])();

void function1()
{
	print("function1()\n");
}

void script()
{
	pf[0] = &function1;
	pf[1] = &function2;

	(*pf[0])();
	(*pf[1])();

	run(pf);
}
