#include "dobjects.h"

// EMPTY_STRING is used in dstring_lazy_init; the idea is that
// we don't want to actually allocate any memory for the dstring yet
// (hence 'lazy'), but we also don't want that the dstring be invalid.
// By this, I mean, should you choose to print out the dstring immediately
// after initializing it, or if you pass it to something that does a strlen()
// operation or something on it, I don't want you to get a segmentation fault.
// So, by having it instead point to this empty string, you can use it
// as a dstring of length 0.
static const char EMPTY_STRING[] = "";


// Will return NULL if unable to init
darray_struct* darray_init_with_size(darray_struct* darray, size_t elem_size, size_t initial_size) {
	darray->array = malloc(initial_size * elem_size);

	if(!darray->array) {
		// malloc failed
		fprintf(stderr, "Unable to allocate space for darray\n");
		return NULL;
	} else {
		darray->total_length = initial_size;
		darray->length = 0;
		darray->elem_size = elem_size;
	}
	return darray;
}
darray_struct* darray_init(darray_struct* darray, size_t elem_size) {
	return darray_init_with_size(darray, elem_size, DARRAY_INITIAL_SIZE);
}
void darray_lazy_init(darray_struct* darray, size_t elem_size) {
	darray->total_length = 0;
	darray->length = 0;
	darray->elem_size = elem_size;
	darray->array = NULL;
}
void darray_free(darray_struct* darray) {
	darray->total_length = 0;
	darray->length = 0;
	darray->elem_size = 0;

	// Return early if there's nothing to free, as is the case if the darray
	// was lazily initialized.
	if(darray->array == NULL) {
		return;
	}
	free(darray->array);
	darray->array = NULL;
}

// Will return NULL if unable to resize
darray_struct* darray_increase_size_specific_amount(darray_struct* darray, size_t additional_elems) {
	size_t new_elem_count = darray->total_length + additional_elems;

	// Not immediately overwriting darray->array as in darray_init_with_size, as if
	// realloc returns NULL, we then lose the original array.
	void* new_pointer = realloc(darray->array, (new_elem_count * darray->elem_size));

	if(new_pointer == NULL) {
		fprintf(stderr, "Unable to increase size of darray\n");
		return NULL;
	} else {
		darray->array = new_pointer;
		darray->total_length = new_elem_count;
	}
	return darray;
}
// Gives some breathing room when resizing.
darray_struct* darray_increase_size(darray_struct* darray) {
	size_t add_elems = 0;

	// Smaller arrays are given less additional space
	if(darray->total_length < DARRAY_INCREMENT_SIZE) {
		add_elems = DARRAY_SMALL_INCREMENT_SIZE;
	} else {
		add_elems = DARRAY_INCREMENT_SIZE;
	}
	return darray_increase_size_specific_amount(darray, add_elems);
}
// Will return the darray, or NULL if error. Uses memcpy to add the item
darray_struct* darray_append(darray_struct* darray, const void* elem) {
	// We won't resize if we are going to fill the array completely, only
	// if there's no room for the item. This is so that if a darray was
	// created with an exact size on purpose, we don't waste space.
	if(darray->total_length - darray->length == 0) {
		if(!darray_increase_size(darray)) {
			fprintf(stderr, "Unable to increase darray size\n");
			return NULL;
		}
	}
	memcpy(darray->array + (darray->length * darray->elem_size), elem, darray->elem_size);
	darray->length++;
	return darray;
}

darray_struct* darray_clone(darray_struct* darray) {
	darray_struct* new_darray = malloc(sizeof(darray_struct));
	if(new_darray == NULL) {
		fprintf(stderr, "Error cloning darray, malloc error\n");
		return NULL;
	}
	if(!darray_init_with_size(new_darray, darray->elem_size, darray->total_length)) {
		fprintf(stderr, "Error cloning darray, darray init error\n");
		free(new_darray);
		return NULL;
	}
	memcpy(new_darray->array, darray->array, (darray->elem_size * darray->length));
	new_darray->length = darray->length;
	return new_darray;
}

// Will return NULL if unable to init
dstring_struct* dstring_init_with_size(dstring_struct* dstring, size_t initial_size) {
	// +1 for null terminator
	dstring->str = malloc(initial_size + 1);

	if(!dstring->str) {
		// malloc failed
		fprintf(stderr, "Unable to allocate space for dstring\n");
		return NULL;
	} else {
		dstring->str[0] = '\0';
		dstring->total_length = initial_size;
		dstring->length = 0;
	}
	return dstring;
}
// Immediately allocates DSTRING_INITIAL_SIZE bytes
dstring_struct* dstring_init(dstring_struct* dstring) {
	return dstring_init_with_size(dstring, DSTRING_INITIAL_SIZE);
}
// Will delay allocation of memory until an append is attempted.
// It points str at an empty string "", so that it's safe to print the string.
// This function exists so that dstring initialization code doesn't need to
// do error checking; since the only way to use the string is to append,
// and any code calling a dstring append function should do error checking,
// there's no harm in doing this, and it should make code cleaner.
void dstring_lazy_init(dstring_struct* dstring) {
	dstring->str = (char*) EMPTY_STRING;
	dstring->total_length = 0;
	dstring->length = 0;
}
void dstring_free(dstring_struct* dstring) {
	if(dstring->str == NULL || dstring->str == EMPTY_STRING || dstring->total_length == 0) {
		return;
	}
	free(dstring->str);
	// Point it back to EMPTY_STRING just to be nice
	dstring->str = (char*) EMPTY_STRING;
	dstring->total_length = 0;
	dstring->length = 0;
}
// Will free the dstrings in a darray, AND the darray.
// But it won't actually free the pointer itself.
void darray_of_dstrings_free(darray_struct* darray) {
	for(size_t i = 0; i < darray->length; i++) {
		dstring_free((dstring_struct*) darray_get_elem(darray, i));
	}
	darray_free(darray);
}
// Will return NULL if unable to resize
dstring_struct* dstring_resize_no_extra(dstring_struct* dstring, size_t additional_bytes) {
	size_t new_size = dstring->total_length + additional_bytes;

	// Set it to NULL so that realloc works
	if(dstring->str == EMPTY_STRING) {
		dstring->str = NULL;
	}

	// Don't immediately overwrite dstring->str so that if realloc returns
	// NULL, we don't lose the string.
	// Also, add 1 so that calling code doesn't have to account for the
	// null terminator.
	void* new_pointer = realloc(dstring->str, new_size + 1);
	if(new_pointer == NULL) {
		fprintf(stderr, "Unable to allocate more space for dstring\n");
		return NULL;
	} else {
		dstring->str = (char*) new_pointer;
		dstring->total_length = new_size;
	}
	return dstring;
}
// Adds some extra breathing room when resizing. Returns NULL on error
dstring_struct* dstring_resize(dstring_struct* dstring, size_t min_additional_bytes) {
	size_t add_bytes = min_additional_bytes;

	// Don't give small strings excessive breathing room
	if(dstring->total_length >= DSTRING_INCREMENT_SIZE) {
		add_bytes += DSTRING_INCREMENT_SIZE;
	} else if(dstring->total_length > 0) {
		add_bytes += DSTRING_SMALL_INCREMENT_SIZE;
	}
	return dstring_resize_no_extra(dstring, add_bytes);
}

// Will return the dstring, or NULL if an error
dstring_struct* dstring_append(dstring_struct* dstring, const char* text) {
	size_t text_len = strlen(text);
	// Resize if too small
	if((dstring->total_length - dstring->length) < (text_len + 1)) {
		if(!dstring_resize(dstring, text_len)) {
			return NULL;
		}
	}
	memcpy(dstring->str + dstring->length, text, text_len);
	dstring->length += text_len;
	dstring->str[dstring->length] = '\0';
	return dstring;
}
dstring_struct* dstring_append_printf(dstring_struct* dstring, const char* format, ...) {
	va_list argptr;
	va_start(argptr, format);
	dstring_struct* res = dstring_append_vaprintf(dstring, format, argptr);
	va_end(argptr);
	return res;
}
dstring_struct* dstring_append_vaprintf(dstring_struct* dstring, const char* format, va_list args) {
	va_list argptr;
	va_copy(argptr, args);
	// Going to use this to determine how much space is actually needed
	char buff[3];
	size_t num_needed = vsnprintf(buff, 2, format, argptr);
	va_end(argptr);
	// I might be adding an extra byte here, but I don't really care... Better
	// safe than sorry.
	if((dstring->total_length - dstring->length) < (num_needed + 1)) {
		if(!dstring_resize(dstring, num_needed)) {
			return NULL;
		}
	}
	int num_written = vsnprintf(&dstring->str[dstring->length], num_needed+1, format, args);
	if(num_written < 0) {
		return NULL;
	}
	dstring->length += num_written;
	return dstring;
}
// Will shift the null byte and length back. Will stop at 0.
void dstring_remove_num_chars_in_text(dstring_struct* dstring, const char* text) {
	size_t text_len = strlen(text);
	if(text_len > dstring->length) {
		text_len = dstring->length;
	}
	dstring->length -= text_len;
	dstring->str[dstring->length] = '\0';
}

// NULL on error. Should be called with a pre-init-ed dstring.
dstring_struct* dstring_read_file(dstring_struct* dstring, const char* file) {
	FILE* fd = fopen(file, "r");
	if(!fd) {
		fprintf(stderr, "Unable to open file %s\n", file);
		return NULL;
	}
	fseek(fd, 0L, SEEK_END);
	size_t file_size = (size_t) ftell(fd);
	rewind(fd);
	if((dstring->total_length - dstring->length) < (file_size + 1)) {
		if(!dstring_resize_no_extra(dstring, file_size)) {
			fclose(fd);
			fprintf(stderr, "Bailing out of reading file %s into dstring because dstring couldn't resize\n", file);
			return NULL;
		}
	}
	size_t bytes_read = fread(dstring->str + dstring->length, 1, file_size, fd);
	dstring->length += bytes_read;
	if(ferror(fd)) {
		fprintf(stderr, "Bailing out of reading file %s into dstring because error while reading file\n", file);
		fclose(fd);
		return NULL;
	}
	fclose(fd);
	dstring->str[dstring->length] = '\0';
	return dstring;
}
// This function WILL pclose() the process output when it is done.
// The value in process_exit_code must only be used if the returned dstring
// is not NULL.
dstring_struct* dstring_read_process_output(dstring_struct* dstring, FILE* process_output, int* process_exit_code) {
	if(!process_output) {
		fprintf(stderr, "Error reading process output, NULL FILE*\n");
		return NULL;
	}

	// Read in the process output a chunk at a time
	do {
		// Make sure there's enough room for the next chunk
		if((dstring->total_length - dstring->length) < (DSTRING_FILE_READ_BLOCK_SIZE + 1)) {
			if(!dstring_resize(dstring, DSTRING_FILE_READ_BLOCK_SIZE)) {
				pclose(process_output);
				fprintf(stderr, "Bailing out of reading process output into dstring because dstring couldn't resize\n");
				return NULL;
			}
		}

		// Read in the next chunk
		size_t bytes_read = fread(dstring->str + dstring->length, 1, DSTRING_FILE_READ_BLOCK_SIZE, process_output);
		dstring->length += bytes_read;

		if(bytes_read < DSTRING_FILE_READ_BLOCK_SIZE) {
			break;
		}
	} while(1);


	if(ferror(process_output)) {
		fprintf(stderr, "Bailing out of reading process output into dstring because error while reading\n");
		pclose(process_output);
		return NULL;
	}
	(*process_exit_code) = pclose(process_output);
	return dstring;
}
// Will overwrite, not append.
int dstring_write_file(dstring_struct* dstring, const char* file) {
	FILE* fd = fopen(file, "w");

	if(!fd) {
		fprintf(stderr, "Unable to open file %s\n", file);
		return 0;
	}

	// Write the entire file out at once
	size_t num_chars_written = fwrite(dstring->str, sizeof(char), dstring->length, fd);
	fclose(fd);

	if(num_chars_written != dstring->length) {
		fprintf(stderr, "Error writing file %s\n", file);
	}
	return (num_chars_written == dstring->length);
}
// Returns 1 if different, -1 if error, 0 if the same.
int dstring_compare_to_file(dstring_struct* dstring, const char* filename) {
	FILE* fd = fopen(filename, "r");

	if(!fd) {
		fprintf(stderr, "Unable to open file %s\n", filename);
		return -1;
	}

	// Get the file size
	fseek(fd, 0L, SEEK_END);
	size_t file_size = (size_t) ftell(fd);
	rewind(fd);

	// If the file is not the same length as the string, then we don't have
	// to compare their contents to know they're different
	if(file_size != dstring->length) {
		fclose(fd);
		return 1;
	}

	// Compare the file a block at a time; there's no sense in consuming
	// the extra memory needed to read the entire file into a dstring
	char read_buffer[DSTRING_FILE_READ_BLOCK_SIZE];
	char* current_location = dstring->str;
	int different = 0;

	do {
		size_t bytes_read = fread(read_buffer, 1, DSTRING_FILE_READ_BLOCK_SIZE, fd);
		if(bytes_read == 0) break;
		if(strncmp(read_buffer, current_location, bytes_read)) {
			different = 1;
			break;
		}
		current_location += bytes_read;
		if(bytes_read < DSTRING_FILE_READ_BLOCK_SIZE) {
			break;
		}
	} while(1);

	if(ferror(fd)) {
		fprintf(stderr, "Bailing out of comparing file %s because error while reading\n", filename);
		fclose(fd);
		return -1;
	}
	fclose(fd);
	return different;
}
// Returns 0 if error, 1 otherwise; check did_write to see if the file was
// actually written.
int dstring_write_file_if_different(dstring_struct* dstring, const char* filename, int* did_write) {
	(*did_write) = 0;
	dstring_struct file_contents;
	dstring_lazy_init(&file_contents);

	int need_to_write = 1;
	if(!access(filename, F_OK)) {
		int res = dstring_compare_to_file(dstring, filename);
		if(res == -1) {
			return 0;
		} else {
			need_to_write = res;
		}
		
	} else {
		// TODO: This belongs in calling code
		printf("Creating file %s as it doesn't exist\n", filename);
	}
	if(need_to_write) {
		if(!dstring_write_file(dstring, filename)) {
			fprintf(stderr, "Error writing file %s\n", filename);
			dstring_free(&file_contents);
			return 0;
		}
		(*did_write) = 1;
	}
	dstring_free(&file_contents);
	return 1;
}

int dstring_test() {
	dstring_struct dstring;
	if(dstring_init(&dstring) == NULL) {
		fprintf(stderr, "Error initializing dstring\n");
		return 0;
	}
	dstring_append(&dstring, "Hello, world!");
	printf("%s\n%zu\n%zu\n", dstring.str, dstring.total_length, dstring.length);
	dstring_append(&dstring, "\nYo, this is a test!");
	printf("%s\n%zu\n%zu\n", dstring.str, dstring.total_length, dstring.length);
	for(int i=0; i < 30000; i++) {
		size_t prev_size = dstring.total_length;
		void* ptr = dstring_append(&dstring, "a");
		if(ptr == NULL) {
			fprintf(stderr, "Error, ptr is null!\n");
			dstring_free(&dstring);
			return 1;
		}
		if(prev_size != dstring.total_length) {
			printf("Total: %zu\nCurrent: %zu\n", dstring.total_length, dstring.length);
		}
	}
	dstring_free(&dstring);
	return 0;
}
int darray_test() {
	darray_struct darray;
	darray_init(&darray, sizeof(int));
	for(int i= 0; i<10; i++) {
		int tmp = i * i;
		if(!darray_append(&darray, &tmp)) {
			darray_free(&darray);
			return 0;
		}
	}
	for(size_t i = 0; i < darray.length; i++) {
		printf("%zu: %d\n", i, *(int*)darray_get_elem(&darray, i));
	}
	for(size_t i=darray.length; i< 300; i++) {
		size_t prev_size = darray.total_length;
		if(!darray_append(&darray, &i)) {
			darray_free(&darray);
			return 0;
		}
		if(prev_size != darray.total_length) {
			printf("Total: %zu\nCurrent: %zu\n", darray.total_length, darray.length);
		}
	}
	darray_free(&darray);
	return 1;
}
int dstring_try_load_file(dstring_struct* destination, dstring_struct* base_dir, const char* file, const char* filetype) {
	if(!dstring_append(base_dir, file)) {
		fprintf(stderr, "Unable to load %s file %s in dir %s, dstring append error\n", filetype, file, base_dir->str);
		return 0;
	}
	if(!dstring_read_file(destination, base_dir->str)) {
		fprintf(stderr, "Unable to load %s file %s, dstring read file error\n", filetype, base_dir->str);
		dstring_remove_num_chars_in_text(base_dir, file);
		return 0;
	}
	dstring_remove_num_chars_in_text(base_dir, file);
	return 1;
	
}
void dstring_remove_trailing_newlines(dstring_struct* dstring) {
	if(dstring->length == 0) return;
	size_t i = dstring->length - 1;
	do {
		if(dstring->str[i] == '\r' || dstring->str[i] == '\n') {
			dstring->str[i] = '\0';
			if(dstring->length > 0) {
				dstring->length--;
			} else {
				break;
			}
		} else {
			break;
		}
	} while(i != 0);
}


// If the delimiter passed in is '\n', then '\r' chars will also be erased.
darray_struct* dstring_split_to_darray(dstring_struct* dstring, darray_struct* darray, char delimiter) {
	int prev_was_delimiter = 1;
	for(size_t i = 0; i < dstring->length; i++) {
		if(prev_was_delimiter) {
			char* tmp = dstring->str + i;
			if(!darray_append(darray, &tmp)) {
				fprintf(stderr, "Error splitting dstring, darray append error");
				return NULL;
			}
		}
		prev_was_delimiter = dstring->str[i] == delimiter;
		if(dstring->str[i] == delimiter || (delimiter == '\n' && dstring->str[i] == '\r')) {
			dstring->str[i] = '\0';
		}
	}
	return darray;
}

#define DSTRINGBUILDER_INTERNAL_DSTRING 0
#define DSTRINGBUILDER_INTERNAL_DSTRINGBUILDER 1

typedef struct dstringbuilder_internal_struct {
	int type;
	// Existing items appended won't be freed, new items created
	// by this dstringbuilder will be freed when the dstringbuilder is freed.
	int can_free;
	// Anonymous union, used because this can only be one or the other,
	// so no need to have a NULL pointer for the one that's not in use
	union {
		dstring_struct* dstring;
		dstringbuilder_struct* dstringbuilder;
	};
} dstringbuilder_internal_struct;

dstringbuilder_internal_struct* dstringbuilder_get_internal(dstringbuilder_struct* dstringbuilder, size_t index) {
	return (dstringbuilder_internal_struct*) darray_get_elem(&dstringbuilder->array, index);
}

void dstringbuilder_init(dstringbuilder_struct* dstringbuilder) {
	darray_lazy_init(&dstringbuilder->array, sizeof(dstringbuilder_internal_struct));
	dstringbuilder->current_dstring = NULL;
}
void dstringbuilder_free(dstringbuilder_struct* dstringbuilder) {
	dstringbuilder->current_dstring = NULL;

	for(size_t i = 0; i < dstringbuilder->array.length; i++) {
		dstringbuilder_internal_struct* dsbi = dstringbuilder_get_internal(dstringbuilder, i);
		// Skip items we didn't create
		if(!dsbi->can_free) continue;

		if(dsbi->type == DSTRINGBUILDER_INTERNAL_DSTRING) {
			dstring_free(dsbi->dstring);
			free(dsbi->dstring);
		} else {
			dstringbuilder_free(dsbi->dstringbuilder);
			free(dsbi->dstringbuilder);
		}
	}
	darray_free(&dstringbuilder->array);
}
dstring_struct* dstringbuilder_append_dstring(dstringbuilder_struct* dstringbuilder, dstring_struct* dstring) {
	dstringbuilder->current_dstring = NULL;

	dstringbuilder_internal_struct dsbi;
	dsbi.type = DSTRINGBUILDER_INTERNAL_DSTRING;
	dsbi.can_free = 0;
	dsbi.dstring = dstring;

	if(!darray_append(&dstringbuilder->array, &dsbi)) {
		return NULL;
	}
	return dstring;
}
dstringbuilder_struct* dstringbuilder_append_dstringbuilder(dstringbuilder_struct* dstringbuilder, dstringbuilder_struct* append_me) {
	dstringbuilder->current_dstring = NULL;

	dstringbuilder_internal_struct dsbi;
	dsbi.type = DSTRINGBUILDER_INTERNAL_DSTRINGBUILDER;
	dsbi.can_free = 0;
	dsbi.dstringbuilder = append_me;

	if(!darray_append(&dstringbuilder->array, &dsbi)) {
		return NULL;
	}
	return append_me;
}
dstring_struct* dstringbuilder_new_dstring(dstringbuilder_struct* dstringbuilder) {
	dstring_struct* dstring = (dstring_struct*) malloc(sizeof(dstring_struct));
	if(dstring == NULL) {
		return NULL;
	}
	dstring_lazy_init(dstring);
	dstringbuilder_internal_struct dsbi;
	dsbi.type = DSTRINGBUILDER_INTERNAL_DSTRING;
	dsbi.can_free = 1;
	dsbi.dstring = dstring;

	if(!darray_append(&dstringbuilder->array, &dsbi)) {
		dstring_free(dstring);
		free(dstring);
		return NULL;
	}
	dstringbuilder->current_dstring = dstring;
	return dstring;
}
dstringbuilder_struct* dstringbuilder_new_dstringbuilder(dstringbuilder_struct* dstringbuilder) {
	dstringbuilder_struct* append_me = (dstringbuilder_struct*) malloc(sizeof(dstringbuilder_struct));
	if(append_me == NULL) {
		return NULL;
	}
	dstringbuilder_init(append_me);
	dstringbuilder_internal_struct dsbi;
	dsbi.type = DSTRINGBUILDER_INTERNAL_DSTRINGBUILDER;
	dsbi.can_free = 1;
	dsbi.dstringbuilder = append_me;

	if(!darray_append(&dstringbuilder->array, &dsbi)) {
		dstringbuilder_free(append_me);
		free(append_me);
		return NULL;
	}
	dstringbuilder->current_dstring = NULL;
	return append_me;
}
dstring_struct* dstringbuilder_append(dstringbuilder_struct* dstringbuilder, const char* str) {
	if(dstringbuilder->current_dstring == NULL) {
		dstring_struct* res = dstringbuilder_new_dstring(dstringbuilder);
		if(res == NULL) {
			return NULL;
		}
	}
	if(!dstring_append(dstringbuilder->current_dstring, str)) {
		return NULL;
	}
	return dstringbuilder->current_dstring;
}
dstring_struct* dstringbuilder_append_printf(dstringbuilder_struct* dstringbuilder, const char* format, ...) {
	if(dstringbuilder->current_dstring == NULL) {
		dstring_struct* res = dstringbuilder_new_dstring(dstringbuilder);
		if(res == NULL) {
			return NULL;
		}
	}
	
	va_list argptr;
	va_start(argptr, format);
	dstring_struct* res = dstring_append_vaprintf(dstringbuilder->current_dstring, format, argptr);
	va_end(argptr);
	if(res == NULL) {
		return NULL;
	}
	return dstringbuilder->current_dstring;
}

size_t dstringbuilder_get_length(dstringbuilder_struct* dstringbuilder) {
	size_t length = 0;
	for(size_t i = 0; i < dstringbuilder->array.length; i++) {
		dstringbuilder_internal_struct* dsbi = dstringbuilder_get_internal(dstringbuilder, i);

		if(dsbi->type == DSTRINGBUILDER_INTERNAL_DSTRING) {
			length += dsbi->dstring->length;
		} else {
			length += dstringbuilder_get_length(dsbi->dstringbuilder);
		}
	}
	return length;
}
dstring_struct* dstringbuilder_internal_form(dstringbuilder_struct* dstringbuilder, dstring_struct* append_to_dstring) {
	for(size_t i = 0; i < dstringbuilder->array.length; i++) {
		dstringbuilder_internal_struct* dsbi = dstringbuilder_get_internal(dstringbuilder, i);

		if(dsbi->type == DSTRINGBUILDER_INTERNAL_DSTRING) {
			if(!dstring_append(append_to_dstring, dsbi->dstring->str)) {
				return NULL;
			}
		} else {
			if(!dstringbuilder_internal_form(dsbi->dstringbuilder, append_to_dstring)) {
				return NULL;
			}
		}
	}
	return append_to_dstring;
}
dstring_struct* dstringbuilder_form(dstringbuilder_struct* dstringbuilder) {
	dstring_struct* dstring = (dstring_struct*) malloc(sizeof(dstring_struct));
	if(dstring == NULL) {
		return NULL;
	}
	if(!dstring_init_with_size(dstring, dstringbuilder_get_length(dstringbuilder) + 1)) {
		free(dstring);
		return NULL;
	}
	if(!dstringbuilder_internal_form(dstringbuilder, dstring)) {
		dstring_free(dstring);
		free(dstring);
		return NULL;
	}
	return dstring;
}

int dstringbuilder_internal_compare_to_file(dstringbuilder_struct* dstringbuilder, FILE* fd) {
	for(size_t i = 0; i < dstringbuilder->array.length; i++) {
		dstringbuilder_internal_struct* dsbi = dstringbuilder_get_internal(dstringbuilder, i);

		if(dsbi->type == DSTRINGBUILDER_INTERNAL_DSTRING) {
			char read_buffer[DSTRING_FILE_READ_BLOCK_SIZE];
			char* current_location = dsbi->dstring->str;
			size_t current_location_pos = 0;
			do {
				size_t bytes_to_read = DSTRING_FILE_READ_BLOCK_SIZE;
				if((dsbi->dstring->length - current_location_pos) < DSTRING_FILE_READ_BLOCK_SIZE) {
					bytes_to_read = (dsbi->dstring->length - current_location_pos);
				}
				size_t bytes_read = fread(read_buffer, 1, bytes_to_read, fd);
				if(bytes_read == 0) break;
				if(strncmp(read_buffer, current_location, bytes_read)) {
					return 1;
				}
				current_location += bytes_read;
				current_location_pos += bytes_read;
				if(bytes_read < bytes_to_read) {
					break;
				}
			} while(current_location_pos < dsbi->dstring->length);
			if(ferror(fd)) {
				return -1;
			}
		} else {
			int res = dstringbuilder_internal_compare_to_file(dsbi->dstringbuilder, fd);
			if(res != 0) {
				printf("HEY!\n");
				return res;
			}
		}
	}
	return 0;
}

// Returns 1 if different, -1 if error, 0 if the same.
int dstringbuilder_compare_to_file(dstringbuilder_struct* dstringbuilder, const char* filename) {
	FILE* fd = fopen(filename, "r");
	if(!fd) {
		fprintf(stderr, "Unable to open file %s\n", filename);
		return -1;
	}
	fseek(fd, 0L, SEEK_END);
	size_t file_size = (size_t) ftell(fd);
	rewind(fd);
	if(file_size != dstringbuilder_get_length(dstringbuilder)) {
		fclose(fd);
		return 1;
	}
	int res = dstringbuilder_internal_compare_to_file(dstringbuilder, fd);
	fclose(fd);
	return res;
}
// Returns 0 if error, 1 otherwise; check did_write to see if the file was
// actually written.
int dstringbuilder_write_file_if_different(dstringbuilder_struct* dstringbuilder, const char* filename, int* did_write) {
	(*did_write) = 0;

	int need_to_write = 1;
	if(!access(filename, F_OK)) {
		int res = dstringbuilder_compare_to_file(dstringbuilder, filename);
		if(res == -1) {
			return 0;
		} else {
			need_to_write = res;
		}
		
	} else {
		// TODO: Move this to calling code.
		printf("Creating file %s as it doesn't exist\n", filename);
	}
	if(need_to_write) {
		dstring_struct* formed_dstring = dstringbuilder_form(dstringbuilder);
		if(formed_dstring == NULL) {
			fprintf(stderr, "Error writing file %s, couldn't form the dstringbuilder\n", filename);
			return 0;
		}
		if(!dstring_write_file(formed_dstring, filename)) {
			fprintf(stderr, "Error writing file %s\n", filename);
			dstring_free(formed_dstring);
			free(formed_dstring);
			return 0;
		}
		(*did_write) = 1;
		dstring_free(formed_dstring);
		free(formed_dstring);
	}
	return 1;
}
#undef DSTRINGBUILDER_INTERNAL_DSTRING
#undef DSTRINGBUILDER_INTERNAL_DSTRINGBUILDER
