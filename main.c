#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
//#define DEBUG 1 /*uncomment for debug print statements*/
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
void analyze_volume_by_sentiment(CBOE_DATA **p_transactions, uint32_t count);
void identify_extreme_momentum_days(CBOE_DATA **p_transactions, uint32_t count);
void analyze_quarterly_seasonality(CBOE_DATA **p_transactions, uint32_t count);
int main(void){
    
    CBOE_DATA **p_transactions = NULL;
    uint32_t transaction_count = 0;
    
    /* OPEN FILE */
    file_desc_status file_status = open_file("SPY241Project.txt");
#ifdef DEBUG
    if(file_status.open_status == 0){
        printf("File opened successully\n\r");
    }
    else{
        printf("File not opened\n\r");
    }
#endif
    
    /*READ FILE*/
    p_transactions = read_data(p_transactions, &transaction_count, &file_status);
    
    printf("\nTotal transactions read from file and successfully stored in memory: %u\n\r", transaction_count);

    /*ANALYZE DATA & PRINT SOME STATS*/
    analyze_volume_by_sentiment(p_transactions, transaction_count);
    identify_extreme_momentum_days(p_transactions, transaction_count);
    analyze_quarterly_seasonality(p_transactions, transaction_count);
    /*CLOSE FILE*/
    file_status = close_file(&file_status);
#ifdef DEBUG
    if(file_status.open_status == 1){
        printf("Closed file descriptor successfully\n\r");
    }
    else{
        printf("Failed to close file descriptor\n\r");
    }
#endif
    if (p_transactions != NULL) {
        for (uint32_t i = 0; i < transaction_count; i++) {
            free(p_transactions[i]);
        }
        free(p_transactions);
    }

    return 0;
}

file_desc_status open_file(char* file_name){
    FILE *fptr;
#ifdef DEBUG
    printf("File to Open: %s\n\r", file_name);
#endif
    fptr = fopen(file_name, "r");
    if(fptr == NULL){
#ifdef DEBUG
        printf("Failed to Open %s\n\r", file_name);
#endif
        file_desc_status file_status = {1, NULL};
        return file_status;
    }
    file_desc_status file_status = {0, fptr};
    return file_status;
}

file_desc_status close_file(file_desc_status* file){
    if (file == NULL || file->FD == NULL) {
#ifdef DEBUG
        printf("Error: Invalid file descriptor provided.\n\r");
#endif
        file_desc_status status = {0, NULL};
        return status;
    }
#ifdef DEBUG
    printf("File descriptor address to Close: %p\n\r", (void*)file->FD);
#endif
    int close_result = fclose(file->FD);
    if(close_result == EOF){
#ifdef DEBUG
        printf("Failed to Close FD\n\r");
#endif
        file_desc_status status = {0, file->FD};
        return status;
    }
#ifdef DEBUG
    printf("Successfully closed file stream\n\r");
#endif
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
#ifdef DEBUG
                printf("Error: System out of memory expanding row table.\n\r");
#endif
                free(p_line_buffer);
                return p_transactions;
            }
            p_transactions = temp_array;
            
            p_transactions[current_idx] = malloc(sizeof(CBOE_DATA));
            if (p_transactions[current_idx] == NULL) {
#ifdef DEBUG
                printf("Error: System out of memory allocating single struct entry.\n\r");
#endif
                free(p_line_buffer);
                return p_transactions;
            }
            
            *p_transactions[current_idx] = temporary_record;
            
            *transaction_count = new_count;
#ifdef DEBUG
            printf("Stored memory matrix block for date: %s\n\r", p_transactions[current_idx]->date);
#endif
        } 
#ifdef DEBUG
        else {
            printf("Skipping non-data or malformed line...\n\r");
        }
#endif
    }

    free(p_line_buffer);
    return p_transactions;
}
void analyze_volume_by_sentiment(CBOE_DATA **p_transactions, uint32_t count) {
    if (p_transactions == NULL || count == 0) return;

    uint64_t panic_vol_sum = 0, calm_vol_sum = 0;
    uint32_t panic_days = 0, calm_days = 0;

    for (uint32_t i = 0; i < count; i++) {
        if (p_transactions[i]->PCR > 1.0f) {
            panic_vol_sum += p_transactions[i]->SPYOV;
            panic_days++;
        } else {
            calm_vol_sum += p_transactions[i]->SPYOV;
            calm_days++;
        }
    }

    printf("\n=== SENTIMENT VS VOLUME ANALYSIS ===\n\r");
    if (panic_days > 0) {
        printf("High Fear Days (PCR > 1.0): %u days | Avg Volume: %lu contracts\n\r", 
               panic_days, panic_vol_sum / panic_days);
    }
    if (calm_days > 0) {
        printf("Calm/Bullish Days (PCR <= 1.0): %u days | Avg Volume: %lu contracts\n\r", 
               calm_days, calm_vol_sum / calm_days);
    }
}

void identify_extreme_momentum_days(CBOE_DATA **p_transactions, uint32_t count) {
    if (p_transactions == NULL || count == 0) return;

    uint64_t total_volume_sum = 0;
    for (uint32_t i = 0; i < count; i++) {
        total_volume_sum += p_transactions[i]->SPYOV;
    }
    uint32_t average_volume = (uint32_t)(total_volume_sum / count);
    uint32_t surge_threshold = average_volume * 2;

    printf("\n=== INSTITUTIONAL MOMENTUM ANALYSIS ===\n\r");
    printf("Dataset Baseline Avg Volume: %u contracts\n\r", average_volume);
    printf("Isolating days exceeding 2x average (%u contracts):\n\r", surge_threshold);
    printf("----------------------------------------------------------------\n\r");
    printf("%-12s | %-12s | %-8s | %-15s\n\r", "Date", "Total Vol", "PCR", "Market Bias");
    printf("----------------------------------------------------------------\n\r");

    uint32_t surge_day_count = 0;

    for (uint32_t i = 0; i < count; i++) {
        if (p_transactions[i]->SPYOV > surge_threshold) {
            surge_day_count++;
            
            char *bias;
            if (p_transactions[i]->PCR > 1.10f) {
                bias = "Extreme Panic";
            } else if (p_transactions[i]->PCR < 0.70f) {
                bias = "Heavy Bullish";
            } else {
                bias = "Balanced/Mixed";
            }

            printf("%-12s | %-12u | %-8.2f | %-15s\n\r", 
                   p_transactions[i]->date, 
                   p_transactions[i]->SPYOV, 
                   p_transactions[i]->PCR, 
                   bias);
        }
    }

    if (surge_day_count == 0) {
        printf("No extreme momentum anomaly days found.\n\r");
    } else {
        printf("----------------------------------------------------------------\n\r");
        printf("Total extreme momentum anomalies found: %u days\n\r", surge_day_count);
    }
}

void analyze_quarterly_seasonality(CBOE_DATA **p_transactions, uint32_t count) {
    if (p_transactions == NULL || count == 0) return;

    uint64_t quarterly_vol_sum[4] = {0, 0, 0, 0};
    double quarterly_pcr_sum[4] = {0.0, 0.0, 0.0, 0.0};
    uint32_t quarterly_day_count[4] = {0, 0, 0, 0};

    for (uint32_t i = 0; i < count; i++) {
        int month = 0;
        
        if (sscanf((char*)p_transactions[i]->date, "%d", &month) != 1) {
            continue; 
        }

        int q_idx = -1;
        if (month >= 1 && month <= 3)       q_idx = 0;
        else if (month >= 4 && month <= 6)  q_idx = 1;
        else if (month >= 7 && month <= 9)  q_idx = 2;
        else if (month >= 10 && month <= 12) q_idx = 3;

        if (q_idx != -1) {
            quarterly_vol_sum[q_idx] += p_transactions[i]->SPYOV;
            quarterly_pcr_sum[q_idx] += p_transactions[i]->PCR;
            quarterly_day_count[q_idx]++;
        }
    }

    printf("\n=== QUARTERLY SEASONALITY ANALYSIS ===\n\r");
    printf("%-10s | %-10s | %-18s\n\r", "Quarter", "Avg PCR", "Avg Daily Volume");
    printf("----------------------------------------------\n\r");

    for (int i = 0; i < 4; i++) {
        if (quarterly_day_count[i] > 0) {
            double avg_pcr = quarterly_pcr_sum[i] / quarterly_day_count[i];
            uint64_t avg_vol = quarterly_vol_sum[i] / quarterly_day_count[i];
            printf("Q%d         | %-10.4f | %-18lu\n\r", i + 1, avg_pcr, avg_vol);
        } else {
            printf("Q%d         | No data    | No data\n\r", i + 1);
        }
    }
    printf("----------------------------------------------\n\r");
}
