#ifndef PARAM_PARSER_INCLUDE
#define PARAM_PARSER_INCLUDE
#include <stdio.h>
#include <string.h>
#define PARAMPARSER_REQUIRED 1
#define PARAMPARSER_OPTIONAL 0

// paramparser defines a few functions for reading in parameters from a
// {int argc, char* argv[]} type parameter set.
// It presently supports parsing parameters of the forms `--boolean`,
// `--key value`, and `--key=value`, though it must be noted that the
// leading `--` is NOT required, and must be included in the parameter
// key. These functions DO modify their argv parameter.
// Also, it is unfortunate that the name of the file has an underscore
// in between `param` and `parser`, while the function names do not.
// This might be addressed in the future.

// ========================
// = param_parser functions
// ========================

// Gets a parameter whose value is a string, and stores it.
// Returns 1 on success, 0 on failure, though it also returns 1
// if the parameter isn't required and wasn't found. This
// might actually be bad behavior, as there is no indication if
// the parameter was found, if it's not required. Though, if the
// destination is set to NULL prior to calling this function, then if
// the destination is not NULL afterwards, then it has been found.
// TODO: Make this more explicit, perhaps by setting the destination
// to NULL immediately as the first line of the function, or by returning
// 0 if the parameter isn't found, even if not required.
int paramparser_get_string(int argc, char** argv, const char* param_name, char** destination, int required);

// If the parameter is in argv, then destination is set to 1, otherwise
// it is set to 0.
// Returns whether or not the parameter was found.
int paramparser_get_flag(int argc, char** argv, const char* param_name, int* destination);

// Returns 1 if there are any non-NULL entries in argv (ie any unused
// parameters).
int paramparser_check_any_remaining(int argc, char** argv);

#endif
