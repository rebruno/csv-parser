#include "../src/csv.h"

//Using get_next_line(), returning an int
void example1(char* filename){
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("example1_output.txt", "w");
    if (f == NULL){
        printf("Error in example1(): Failed to open %s\n", filename);
    }
    if (f2 == NULL){
        printf("Error in example1(): Can't write\n");
    }


    CSV *c;
    int result = initialize_csv(&c, f);
    if (result < 0){
        printf("Error in example1(): %d\n", result);
    }

    Row* r = c->row;
    //Loop and write the rows to a file
    while ((result = get_next_line(c)) > 0){
        for (int i = 0; i < r->size - 1; i++){
            fprintf(f2, "%s,", r->data[i]);
        }
        fprintf(f2, "%s\n", r->data[r->size-1]);
    }

    destroy_csv(c);

    fclose(f);
    fclose(f2);
    
}

//Using get_next_row(), returning a Row
void example2(char* filename){
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("example2_output.txt", "w");

    if (f == NULL){
        printf("Error in example2(): Failed to open %s\n", filename);
    }
    if (f2 == NULL){
        printf("Error in example2(): Can't write\n");
    }

    CSV *c;

    int result = initialize_csv(&c, f);
    if (result < 0){
        printf("Error in example2(): %d\n", result);
    }

    Row r = get_next_row(c);
    while (r.size > 0){
        for (int i = 0; i < r.size - 1; i++){
            fprintf(f2, "%s,", r.data[i]);
        }
        fprintf(f2, "%s\n", r.data[r.size-1]);

        r = get_next_row(c);
    }

    if (r.size < 0){
        printf("Error: %d\n", r.size);
    }
    
    destroy_csv(c);

    fclose(f);
    fclose(f2);   
}

int main(int argv, char* argc[]){
    char *filename;

    if (argv > 1){
        filename = argc[1];
    }
    else{
        filename = "example_text.txt";    
    }

    example1(filename);
    example2(filename);

    printf("Success\n");
    return 0;
}