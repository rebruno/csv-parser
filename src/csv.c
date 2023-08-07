#include "csv.h"

#define FAILED_MALLOC -1
#define INCORRECT_STRING -2
#define FAILED_READ -3
#define INCORRECT_FORMAT -4

char separator = ',';

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

/*
    row_string is a null-terminated string
    size is the size in bytes of row_string (strlen for ASCII)
    Copies row_string to row_strings, replaces separators with nulls
    And then passes an array of pointers to r->data that point to beginning of each field
*/
int load_row_comma(Row *r, char* row_string, int size){
    return load_row(r, row_string, size, ',');
}

int load_row(Row *r, char* row_string, int size, int separator){
    int error_num = 0;

    int fields = get_number_of_fields(row_string, size, separator);

    char **data = malloc(fields * sizeof data);
    if (data == NULL){
        error_num = FAILED_MALLOC;
        goto EXIT;
    }

    //memcpy(data, (unsigned char*)&size, sizeof size);

    char* row_strings = malloc(size + 1);
    if (row_strings == NULL){
        error_num = FAILED_MALLOC;
        goto EXIT;
    }

    memcpy(row_strings, row_string, size + 1);

    char* p = row_strings;
    int offset = 0;
    data[offset++] = row_strings;


    //Iterate over row_strings
    //If a character is a separator, replace it with '\0' and set data[offset] to the start of the next field
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

    r->size = offset; //1 indexed, so data[offset-1] is the final element
    r->data = data;

//https://softwareengineering.stackexchange.com/questions/154974/is-this-a-decent-use-case-for-goto-in-c
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
 * Similar to glib'c's __getdelim, used in getline(), but without using the internal structure of FILE
 * delimiter is the character that it stops reading at. Included in the final string
 * Requires an initalized Buffer, and an open stream
 * If the Buffer already has something in it, then b->bp points to it
 * Otherwise it has nothing
 * Returns the number of characters read or an error number
 * 
 * Without a lot of testing, this function works with UTF encodings(not the delimiter since memchr is used)
 */
int __getdelim(char **str, int *str_size, int delimiter, Buffer *b, FILE *stream){
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

        char* pt = memchr(b->bp, delimiter, len); //Replace with unicode in future?

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

        if (pt != NULL || len == 0){ 
            break;
        }
                
        len = b->end - b->bp;   
    }
    

    (*str)[cur_len] = '\0';

    return cur_len;

}



/** 
 * Takes in a CSV struct, and outputs the next line in Row, as an array of strings
 * CSV must be initialized with create_csv() before calling this function
 */
int get_next_line(CSV *csv, Row *r){
    int error_num = 0;
    char newline = '\n';

    fsetpos(csv->fp, &(csv->pos));

    int str_size = r->size;
    char *row_string = malloc(r->size);

    error_num = __getdelim(&(row_string), &(str_size), newline, csv->buffer, csv->fp);
    if (error_num < 0){
        return error_num;
    }

    error_num = load_row(r, row_string, str_size, ',');
    if (error_num < 0){
        return error_num;
    }

    fgetpos(csv->fp, &(csv->pos));

}



// fp must be an open stream
int create_csv(CSV *c, FILE *fp){
    int error_num = 0;
    c = malloc(sizeof c);
    if (c == NULL){
        return FAILED_MALLOC;
    }
    c->fp = fp;

    Buffer *buffer;
    error_num = create_buffer(buffer);
    if (error_num < 0) {
        free(c);
        return error_num;
    }

    c->buffer = buffer;

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



int parse_csv(char* filename, CSV *csv){
    int error_num = 0;

    FILE *fp = fopen(filename, "r");
    if (fp == NULL){
        return FAILED_READ;
    }

    error_num = create_csv(csv, fp);
    if (error_num < 0){
        return error_num;
    }

    return error_num;
}


//Since these could be stored on the stack, the pointer passed is not free'd
int destroy_row(Row *r){
    free(r->data);
    return 0;
}

int destroy_csv(CSV *csv){

    destroy_buffer(csv->buffer);

    if (csv->fp != NULL){
        fclose(csv->fp);
        csv->fp = NULL;
    }

    return 0;

}

int destroy_buffer(Buffer *b){
    free(b->buffer);
    return 0;
}
