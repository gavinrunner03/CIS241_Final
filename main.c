#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint8_t date[11];      /* Date of Transaction */
    float PCR;            /* Put/Call Ratio  */
    uint32_t SPYPV;       /* SPY Put Volume  */
    uint32_t SPYCV;       /* SPY Call Volume */
    uint32_t SPYOV;       /* SPY Options Volume */
} CBOE_DATA;

typedef struct {
    uint8_t open_status;
    FILE *FD;
} file_desc_status;

file_desc_status open_file(char* file_name);
file_desc_status close_file(file_desc_status* file);
CBOE_DATA **read_data(CBOE_DATA **p_transactions, uint32_t* transaction_count, file_desc_stauts* p_file_desc);

int main(void){
    
    CBOE_DATA **p_transactions;
    uint32_t transaction_count = 0;
    /* OPEN FILE */
    file_desc_status file_status = open_file("SPY241Project.txt");
    if(file_status.open_status == 0){
    printf("File opened successully\n\r");
    }
    else{
        printf("File not opened\n\r");
    }
    /*READ FILE*/

    /*ANALYZE DATA*/

    /*PRINT SOME STATS*/
    /*CLOSE FILE*/
    file_status = close_file(&file_status);
    if(file_status.open_status == 1){
        printf("Closed file descriptor successfully\n\r");
    }
    else{
        printf("Failed to close file descriptor\n\r");
    }
    return 0;
}

file_desc_status open_file(char* file_name){
    FILE *fptr;
    printf("File to Open: %s\n\r", file_name);
    fptr = fopen(file_name, "r");
    if(fptr == NULL){
        printf("Failed to Open %s\n\r", file_name);
        file_desc_status file_status = {1, NULL};
        return file_status;
    }
    file_desc_status file_status = {0, fptr};
    return file_status;
}

file_desc_status close_file(file_desc_status* file){
    if (file == NULL || file->FD == NULL) {
        printf("Error: Invalid file descriptor provided.\n\r");
        file_desc_status status = {0, NULL};
        return status;
    }
    printf("File descriptor address to Close: %p\n\r", (void*)file->FD);
    int close_result = fclose(file->FD);
    if(close_result == EOF){
        printf("Failed to Close FD\n\r");
        file_desc_status status = {0, file->FD};
        return status;
    }
    printf("Successfully closed file stream\n\r");
    file->FD = NULL;
    file->open_status = 1;
    
    file_desc_status status = {1, NULL};
    return status;
}

CBOE_DATA **read_data(CBOE_DATA **p_transactions, uint32_t* transaction_count, file_desc_status* p_file_desc){

    size_t size = 0;
    char *p_line_buffer = NULL;

    

    /*while result is not EOF, then add a struct of the data to the **p_transactions*/
    while((getline(&p_line_buffer, &size, p_file_desc->FD))!=-1){
        /*Parse the data*/

    }
    /*free memory and return once loop is broken*/
    free(p_line_buffer);
    return p_transactions;
}