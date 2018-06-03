#include "param_parser.h"

// Returns 1 on success, 0 on failure
int paramparser_get_string(int argc, char* argv[], const char* param_name, char** destination, int required) {
	size_t param_name_length = strlen(param_name);
	for(int i = 0; i < argc; i++) {
		// Skip empty entries
		if(!argv[i]) continue;
		
		if(!strncmp(argv[i], param_name, param_name_length)) {
			// OK, we found the parameter, we need to make sure there's a value.
			// We _are_ allowing equals signs, so we must account for them.
			
			if(strlen(argv[i]) == param_name_length) {
				// No equals sign, so must be next arg
				if(i + 1 >= argc) {
					// Past end of array
					return 0;
				} else if(!argv[i + 1]) {
					// Next param is already used
					return 0;
				} else {
					(*destination) = argv[i + 1];
					argv[i] = NULL;
					argv[i + 1] = NULL;
					return 1;
				}
			} else {
				// Equals sign?
				// We know that there's at least one more character in the string.
				// Also, there's a possibility that maybe this is not the parameter
				// that we are looking for.
				if(argv[i][param_name_length] != '=') {
					// This isn't the argument we are looking for.
					continue;
				}
				if(argv[i][param_name_length + 1] == '\0') {
					// There's no value here, error
					return 0;
				}
				(*destination) = argv[i] + param_name_length + 1;
				argv[i] = NULL;
				return 1;
			}
		}
	}
	return !required;
}
// Can't fail, because either the flag exists, or it doesn't. And flags aren't
// required, because if it's a required flag, then why is it an argument?
// Returns 1 if found, 0 if not, so it can be chained.
int paramparser_get_flag(int argc, char* argv[], const char* param_name, int* destination) {
	for(int i = 0; i < argc; i++) {
		// Skip empty entries
		if(!argv[i]) continue;
		
		if(!strcmp(argv[i], param_name)) {
			(*destination) = 1;
			argv[i] = NULL;
			return 1;
		}
	}
	(*destination) = 0;
	return 0;
}

// Returns 1 if any arguments left, 0 otherwise
int paramparser_check_any_remaining(int argc, char* argv[]) {
	for(int i = 1; i < argc; i++) {
		if(argv[i]) return 1;
	}
	return 0;
}
