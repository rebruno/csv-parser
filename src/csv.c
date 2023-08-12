#include "csv.h"

#define FAILED_MALLOC -1
#define INCORRECT_STRING -2
#define FAILED_READ -3
#define INCORRECT_FORMAT -4

char separator = ',';

/**
 * Similar to glibc's __getdelim, used in getline(), but without using the internal structure of FILE and only using standard C
 * delimiter is the character that it stops reading at. Included in the final string
 * Requires an initalized Buffer, and an open stream
 * If the Buffer already has something in it, then b->bp points to it
 * Otherwise it has nothing
 * Returns the number of characters read or an error number
 * String is null-terminated
 * Works with UTF-8 strings, delimiter must still be ASCII 
 */
int __getdelimiter(char **str, int *str_size, int delimiter, Buffer *b, FILE *stream){
    int error_num;
    int len; //Number of characters to process in buffer
    int cur_len = 0;

    if (str == NULL || str_size == NULL || b == NULL){
        return INCORRECT_FORMAT;
    }

    //If not allocated, allocate str and set str_size
    if (*str == NULL || *str_size <= 0){
        if (*str_size <= 0){
            *str_size = LINE_BUFFER_SIZE;
        }
        *str = malloc(*str_size);
        if (*str == NULL){
            return FAILED_MALLOC;
        }
    }

    if (b->end == b->bp){
        error_num = load_buffer(stream, b, b->size);
        if (error_num < 0){
            return error_num;
        }
        len = error_num;
    }
    else{
        len = b->end - b->bp;   
    }

    for (;;){

        char* pt = memchr(b->bp, delimiter, len); 

        if (pt != NULL){
            len = pt - b->bp + 1;
        }

        int needed = cur_len + len + 1;

        if (needed > *str_size){
            if (needed < 2 * *str_size){
                needed = 2 * *str_size;
            }
            //Need to reallocate
            char *new_str = realloc(*str, needed);
            if (new_str == NULL){
                return FAILED_MALLOC;
            }
            *str = new_str;
            *str_size = needed;
        }


        memcpy(*str + cur_len, b->bp, len);

        cur_len += len;
        b->bp = b->bp + len;

        if (pt != NULL){ 
            break;
        }

                
        len = b->end - b->bp;   
        if (len == 0){
            //Buffer has been exhausted 
            if (feof(stream)){
                break;
            }
            error_num = load_buffer(stream, b, b->size);
            if (error_num < 0){
                return error_num;
            }
            len = error_num;
        }
    }
    

    (*str)[cur_len] = '\0';

    return cur_len;

}


int get_number_of_fields(void *src, int size, int delimiter){
    int num = 0;
    unsigned char *s = (unsigned char *) src;
    while (size-- > 0){
        if (*s == delimiter)
        {
            num++;
        }
        s++;
    }
    return num + 1;
}

/**
 * row_string is a null-terminated string
 * size is the number of bytes for row_string excluding null (strlen for ASCII)
 * Copies row_string to row_strings, replaces separators with nulls
 * And then passes an array of pointers to r->data that point to beginning of each field  
 *
 * Separator must be ASCII
 * The row must have it's fields set to NULL initially(create_row() does this)  
*/
int load_row_comma(Row *r, char* row_string, int size){
    return load_row(r, row_string, size, ',');
}
int load_row(Row *r, char* row_string, int size, int separator){

    int error_num = 0;

    if (r->data != NULL){
        free(r->data[0]);
        free(r->data);
    }

    int fields = get_number_of_fields(row_string, size, separator);

    char **data = malloc(fields * sizeof data);
    if (data == NULL){
        error_num = FAILED_MALLOC;
        goto EXIT;
    }


    char* row_strings = malloc(size + 1);
    if (row_strings == NULL){
        error_num = FAILED_MALLOC;
        goto EXIT;
    }

    memcpy(row_strings, row_string, size + 1);

    char* p = row_strings;
    int offset = 0;
    data[offset++] = row_strings;

    for (int i = 0; i < size; i++, p++){
        if (*p == separator){
            *p = '\0';
            data[offset++] = p+1;
        }
    }

    if (*p != '\0'){
        error_num = INCORRECT_STRING;
        goto EXIT;
    }

    data[offset-1][strcspn(data[offset-1], "\r\n")] = '\0'; //strips newline, works with CRLF or LF

    r->size = offset; //data[offset-1] is the final element
    r->data = data;

EXIT:
    if (error_num < 0) {
        free(data);
        free(row_strings);
    }

    return error_num;
}


int load_buffer(FILE *fp, Buffer *b, int read_characters){
    read_characters = fread(b->buffer, sizeof *(b->buffer), b->size, fp);
    if (read_characters == 0){
        if (ferror(fp)){
            return FAILED_READ;
        }
    }
    b->bp = b->buffer;
    b->end = b->bp + read_characters;
    return read_characters;
}


/** 
 * Takes in a CSV struct, and outputs the number of fields
 * CSV must be initialized with create_csv() before calling this function
 * Returns the number of fields in the row. If zero, then there are none
 * 
 * The row is not kept between successive calls since the data it uses is free'd.
 * If multiple rows are needed at once the data must be copied elsewhere between calls
 */
int get_next_line(CSV *csv){
    return get_next_line2(csv, ',');
}

int get_next_line2(CSV *csv, int delimiter){
    int error_num = 0;
    char newline = '\n';
    Row *r = csv->row;


    if (r->data != NULL && r->size > 0){
        free(r->data[0]);
        free(csv->row->data);
        r->data = NULL;
        r->size = 0;
    }

    int str_size = LINE_BUFFER_SIZE;
    char *row_string = malloc(str_size);

    error_num = __getdelimiter(&(row_string), &(str_size), newline, csv->buffer, csv->fp);
    if (error_num < 0){
        goto Fail;
    }
    if (error_num == 0){
        r->size = 0;
        r->data = NULL;
        goto Exit;
    }

    int str_len = strlen(row_string);
    error_num = load_row(r, row_string, str_len, delimiter);
    if (error_num < 0){
        goto Fail;
    }

Exit:
    free(row_string);

    return r->size;

Fail:
    free(row_string);
    if (r->data != NULL && r->size > 0){
        free(csv->row->data[0]);
        free(csv->row->data);
    }

    r->data = NULL;
    r->size = 0;

    return error_num;
}



/** 
 * Takes in a CSV struct, and outputs the number of fields
 * CSV must be initialized with create_csv() before calling this function
 * Returns the row
 *
 * The row is not kept between successive calls since the data it uses is free'd.
 * If multiple rows are needed at once the data must be copied elsewhere
 */
Row get_next_row(CSV *csv){
    int result = get_next_line(csv);
    if (result < 0){
        Row r;
        r.size = result;
        r.data = NULL;
        return r;
    }

    return *(csv->row);
}


// fp must be an open stream
int initialize_csv(CSV **c, FILE *fp){
    *c = malloc(sizeof **c);
    if (*c == NULL){
        return FAILED_MALLOC;
    }

    return create_csv(*c, fp);
}

int create_csv(CSV *c, FILE *fp){
    int error_num = 0;

    c->fp = fp;

    Buffer *buffer = malloc(sizeof *buffer);
    if (buffer == NULL){
        free(c);
        return FAILED_MALLOC;
    }

    error_num = create_buffer(buffer);
    if (error_num < 0) {
        free(c);
        return error_num;
    }

    c->buffer = buffer;

    c->row = malloc(sizeof *(c->row));
    if (c->row == NULL){
        free(c->buffer);
        free(c);
        return FAILED_MALLOC;
    }

    c->row->size = 0;
    c->row->data = NULL;

    return 0;
}


int create_buffer(Buffer *b){
    
    b->buffer = malloc(CSV_BUFFER_SIZE);
    if (b->buffer == NULL) return FAILED_MALLOC;
    b->size = CSV_BUFFER_SIZE;
    b->bp = b->buffer;
    b->end = b->buffer;

    return 0;
}

int create_row(Row *r){
    r->size = 0;
    r->data = NULL;
    return 0;
}

int destroy_csv(CSV *csv){

    destroy_buffer(csv->buffer);
    free(csv->buffer);

    /*
    if (csv->fp != NULL){
        fclose(csv->fp);
        csv->fp = NULL;
    }
    */
    destroy_row(csv->row);
    free(csv->row);

    free(csv);

    return 0;

}

//Since these could be stored on the stack, the pointer passed is not free'd
int destroy_row(Row *r){
    if (r->size > 0){
        free(r->data[0]);
    }
    free(r->data);
    return 0;
}

int destroy_buffer(Buffer *b){
    free(b->buffer);
    return 0;
}
