#include "dobjects.h"




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
void darray_free(darray_struct* darray) {
	if(darray->array == NULL) {
		return;
	}
	free(darray->array);
	darray->array = NULL;
	darray->total_length = 0;
	darray->length = 0;
	darray->elem_size = 0;
}

// Will return NULL if unable to resize
darray_struct* darray_increase_size_specific_amount(darray_struct* darray, size_t additional_elems) {
	size_t new_elem_count = darray->total_length + additional_elems;
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
darray_struct* darray_increase_size(darray_struct* darray) {
	return darray_increase_size_specific_amount(darray, DARRAY_INCREMENT_SIZE);
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
// NO BOUNDS CHECKING IS DONE
void* darray_get_elem(darray_struct* darray, size_t index) {
	return darray->array + (index * darray->elem_size);
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
dstring_struct* dstring_init(dstring_struct* dstring) {
	return dstring_init_with_size(dstring, DSTRING_INITIAL_SIZE);
}
void dstring_free(dstring_struct* dstring) {
	if(dstring->str == NULL || dstring->total_length == 0) {
		return;
	}
	free(dstring->str);
	dstring->str = NULL;
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
dstring_struct* dstring_resize(dstring_struct* dstring, size_t min_additional_bytes) {
	size_t new_size = dstring->total_length + min_additional_bytes + DSTRING_INCREMENT_SIZE;
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

// Will return the dstring, or NULL if an error
dstring_struct* dstring_append(dstring_struct* dstring, const char* text) {
	size_t text_len = strlen(text);
	// Check if too small
	if((dstring->total_length - dstring->length) < (text_len + 1)) {
		if(!dstring_resize(dstring, text_len)) {
			return NULL;
		}
	}
	//strncat(dstring->str, text, text_len);
	memcpy(dstring->str + dstring->length, text, text_len);
	dstring->length += text_len;
	dstring->str[dstring->length] = '\0';
	return dstring;
}
dstring_struct* dstring_append_printf(dstring_struct* dstring, const char* format, ...) {
	// TODO: Check if printf failed
	va_list argptr;
	va_start(argptr, format);
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
	va_list argptr2;
	va_start(argptr2, format);
	int num_written = vsnprintf(&dstring->str[dstring->length], num_needed+1, format, argptr2);
	dstring->length += num_written;
	va_end(argptr2);
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
		if(!dstring_resize(dstring, file_size)) {
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
	do {
		// TODO: Shouldn't resize unless out of space, to respect smaller strings.
		if((dstring->total_length - dstring->length) < (DSTRING_FILE_READ_BLOCK_SIZE + 1)) {
			if(!dstring_resize(dstring, DSTRING_FILE_READ_BLOCK_SIZE)) {
				pclose(process_output);
				fprintf(stderr, "Bailing out of reading process output into dstring because dstring couldn't resize\n");
				return NULL;
			}
		}
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
	size_t num_chars_written = fwrite(dstring->str, sizeof(char), dstring->length, fd);
	fclose(fd);
	if(num_chars_written != dstring->length) {
		fprintf(stderr, "Error writing file %s\n", file);
	}
	return (num_chars_written == dstring->length);
}
// Returns 0 if error, 1 otherwise; check did_write to see if the file was
// actually written.
int dstring_write_file_if_different(dstring_struct* dstring, const char* filename, int* did_write) {
	(*did_write) = 0;
	dstring_struct file_contents;
	if(!dstring_init(&file_contents)) {
		fprintf(stderr, "Error writing file, dstring init error\n");
		return 0;
	}
	int need_to_write = 1;
	if(!access(filename, F_OK)) {
		if(!dstring_read_file(&file_contents, filename)) {
			fprintf(stderr, "Error writing file, couldn't read existing file\n");
			dstring_free(&file_contents);
			return 0;
		}
		if(!strcmp(file_contents.str, dstring->str)) {
			need_to_write = 0;
		}
		
	} else {
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
	dstring_init(&dstring);
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
