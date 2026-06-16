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
CBOE_DATA **read_data(CBOE_DATA **p_transactions, uint32_t* transaction_count, file_desc_status* p_file_desc);
CBOE_DATA **analyze_data(CBOE_DATA **p_transactions, uint32_t* transaction_count);

int main(void){
    
    CBOE_DATA **p_transactions = NULL;
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
    p_transactions = read_data(p_transactions, &transaction_count, &file_status);
    
    printf("\nTotal transactions successfully stored in memory: %u\n\r", transaction_count);

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

    // CLEANUP WORK: Clean up your dynamic arrays before exiting main
    if (p_transactions != NULL) {
        for (uint32_t i = 0; i < transaction_count; i++) {
            free(p_transactions[i]); // Free individual struct rows
        }
        free(p_transactions);
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

    while((getline(&p_line_buffer, &size, p_file_desc->FD)) != -1){
        
        CBOE_DATA temporary_record;

        int fields_parsed = sscanf(p_line_buffer, " %10[^,],%f,%u,%u,%u",
                           temporary_record.date,
                           &temporary_record.PCR,
                           &temporary_record.SPYPV,
                           &temporary_record.SPYCV,
                           &temporary_record.SPYOV);

        if (fields_parsed == 5) {
            
            uint32_t current_idx = *transaction_count;
            uint32_t new_count = current_idx + 1;
            
            CBOE_DATA **temp_array = realloc(p_transactions, new_count * sizeof(CBOE_DATA *));
            if (temp_array == NULL) {
                printf("Error: System out of memory expanding row table.\n\r");
                free(p_line_buffer);
                return p_transactions;
            }
            p_transactions = temp_array;
            
            p_transactions[current_idx] = malloc(sizeof(CBOE_DATA));
            if (p_transactions[current_idx] == NULL) {
                printf("Error: System out of memory allocating single struct entry.\n\r");
                free(p_line_buffer);
                return p_transactions;
            }
            
            *p_transactions[current_idx] = temporary_record;
            
            *transaction_count = new_count;
            
            printf("Stored memory matrix block for date: %s\n\r", p_transactions[current_idx]->date);
        } else {
            printf("Skipping non-data or malformed line...\n\r");
        }
    }

    free(p_line_buffer);
    return p_transactions;
}
CBOE_DATA **analyze_data(CBOE_DATA **p_transactions, uint32_t* transaction_count){

}