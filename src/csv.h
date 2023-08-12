#ifndef _CSV_PARSER
#define _CSV_PARSER

#define CSV_BUFFER_SIZE 4096

//If the line size in bytes is known beforehand, this can be increased to accomodate it, avoiding any realloc's
#define LINE_BUFFER_SIZE 1024 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Row{
    int size;
    char **data;
} Row;

typedef struct Buffer{
    int size;
    char* buffer;
    char *bp; //Points to current location in buffer where there is data to be read
    char *end; //Points to end of data in buffer
} Buffer;

typedef struct CSV{
    FILE *fp;
    Buffer *buffer;
    Row *row;
} CSV;


int __getdelimiter(char **str, int *str_size, int delimiter, Buffer *b, FILE *stream);

int create_buffer(Buffer *b);
int create_csv(CSV *c, FILE *fp);
int create_row(Row *r);

int initialize_csv(CSV **c, FILE *fp);

int load_row_comma(Row *r, char* row_string, int size);
int load_row(Row *r, char *str, int size, int separator);
int load_buffer(FILE *fp, Buffer *b, int read_characters);

int get_next_line(CSV *csv);
int get_next_line2(CSV *csv, int delimiter);
Row get_next_row(CSV *csv);

int destroy_row(Row *r);
int destroy_csv(CSV *c);
int destroy_buffer(Buffer *b);

#endif