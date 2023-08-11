#include "../src/csv.h"


int test___getdelimiter(void){
    char* filename = "file_test.txt";
    FILE *f = fopen(filename, "r");

    char *str = malloc(CSV_BUFFER_SIZE);
    int count = CSV_BUFFER_SIZE;
    Buffer b;
    create_buffer(&b);

    int result, err_num, sum;
    sum = 0;
    while ((result = __getdelimiter(&str, &count, '\n', &b, f)) > 0){
        sum++;
    }

    destroy_buffer(&b);

    if (result >= 0 && sum == 4){
        return 0;
    }
    
    return -1;

}

int test_load_row(void){
    
    char *str = "'a9ab','432a','6ad0'";
    char* elements[] = {"'a9ab'", "'432a'", "'6ad0'"};

    Row r;
    create_row(&r);
    int result = load_row(&r, str, strlen(str), ',');

    if (result != 0){
        return -1;
    }

    for (int i = 0; i < 3; i++){
        if (strncmp(r.data[i], elements[i], strlen(elements[i])) != 0){
            return -1;
        }
    }

    destroy_row(&r);

    return 0;
}

int test_get_next_line(void){
    CSV *c;

    char* filename = "file_test.txt";
    FILE *f = fopen(filename, "r");

    int result = initialize_csv(&c, f);
    if (result < 0){
        return -1;
    }

    Row r = get_next_line2(c);
    result = r.size;
    if (result < 0){
        return -1;
    }

    result = 0;
    for (int i = 0; i < r.size; i++){
        result = result + atoi(r.data[i]) - (i + 1);
    }

    destroy_csv(c);
    return result;
}

int main(void){

    int result = 0;
    result = result + test___getdelimiter();
    if (result != 0){
        printf("Failed __getdelim\n");
    }

    result = result + test_load_row();
    if (result != 0){
        printf("Failed load_row\n");
    }

    result = result + test_get_next_line();
    if (result != 0){
        printf("Failed get_next_line\n");
    }

    if (result == 0){
        printf("Passed\n");
    }

    return 0;
}