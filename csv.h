#ifndef _CSV_PARSER
#define _CSV_PARSER

#define CSV_BUFFER_SIZE 4096
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
    fpos_t pos;
    Buffer *buffer;
} CSV;


int __getdelim(char **str, int *str_size, int delimiter, Buffer *b, FILE *stream);


int create_buffer(Buffer *b);
int create_csv(CSV *c, FILE *fp);

int load_row_comma(Row *r, char* row_string, int size);
int load_row(Row *r, char *str, int size, int separator);
int load_buffer(FILE *fp, Buffer *b, int read_characters);

int get_next_line(CSV *csv, Row *r);
int parse_csv(char* filename, CSV *csv);

int destroy_row(Row *r);
int destroy_csv(CSV *c);
int destroy_buffer(Buffer *b);

#endif