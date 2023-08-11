
# CSV parser
Small cross-platform C library for reading and parsing CSV files.  
Separator must be 1-byte long, and each line must end with an ASCII newline('\n')  

## Compiling and using
Uses only standard C and can be compiled with any compliant compiler.  
If the size of lines is known beforehand then the definition LINE_BUFFER_SIZE in csv.h can be modified  
Simplest way of using is calling `initialize_csv(CSV *c, FILE *f)`, `get_next_line(CSV *c)` and then iterating over the fields in `csv->row->data`.

## Example
A minimal example that counts the number of rows and number of values of a csv file called "example.csv". A different way of reading rows can be found in example/example.c. 
```
#include "csv.h"
int main(void){
    FILE *f = fopen("example.csv", "r");
    if (f == NULL){
        return -1;
    }

    CSV *c;
    int result = initialize_csv(&c, f);
    if (result < 0){
        return -1;
    }

    int rows = 0;
    int values = 0;

    Row* r = c->row;
    while ((result = get_next_line(c)) > 0){
        rows++;
        for (int i = 0; i < r->size; i++){
            values++;
        }
    }
    printf("There are %d rows and %d total values\n", rows, values);
    destroy_csv(c);
    fclose(f);

    return 0;
}
```
## Documentation
 

`int initialize_csv(CSV *c, FILE *f);` FILE must be an open stream  

`int get_next_line(CSV *csv);` Loads next line into c->row  
`int get_next_line2(CSV *csv, int delimiter);` Loads next line into c->row    
`Row get_next_row(CSV *csv);` Returns next row  

`int destroy_csv(CSV *c)`   

### Accessing fields
Row struct has a size field and a data field. There are `size` fields that can be accessed with `Row.data[i]`, where `i < size`. 








