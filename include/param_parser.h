#ifndef PARAM_PARSER_INCLUDE
#define PARAM_PARSER_INCLUDE
#include <stdio.h>
#include <string.h>
#define PARAMPARSER_REQUIRED 1
#define PARAMPARSER_OPTIONAL 0
int paramparser_get_string(int, char**, const char*, char**, int);
int paramparser_get_flag(int, char**, const char*, int*);
int paramparser_check_any_remaining(int, char**);


#endif
