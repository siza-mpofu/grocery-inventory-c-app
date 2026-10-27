#ifndef TRANSACTION_H
#define TRANSACTION_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TS_LEN 32
#define MAX_NOTE_LEN 128

typedef enum {
    TX_IN,
    TX_OUT,
    TX_ADJUSTMENT
} TransactionType;

typedef struct {
    int id;
    char timestamp[MAX_TS_LEN];
    TransactionType type;
    int product_id;
    int quantity_delta;
    int supplier_id;
    char note[MAX_NOTE_LEN];
} Transaction;

#ifdef __cplusplus
}
#endif

#endif