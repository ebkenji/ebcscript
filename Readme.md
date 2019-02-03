# Ebcscript
A simple C library for interpreting C sources.

## Description
Ebcscript is a library that provides a function of C interpreter for your C program.

## Requirements
Ebcscript requires the following to build:
 - Bison
 - Flex
 
 ## Building
To build the library:
```
$ make
parse.y: conflicts: 1 shift/reduce
```

To run an example:
```
$ gcc example1.c ebcscrip.a -o example1
$ ./example1
5! = 120
```

## Usage
See the file "example1.c".

example1.c:
```c
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
```

factoria.c:
```c
int factorial(char n)
{
  return (n <= 1) ? 1 : n * factorial(n - 1);
}
```

## To Do
- Preprocessing directives: #include, #define and #ifdef
- A function that accepts a variable number of arguments

## License
Ebcscript is licensed under the MIT license.
