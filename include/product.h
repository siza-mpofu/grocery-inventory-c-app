#ifndef PRODUCT_H
#define PRODUCT_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NAME_LEN 64
#define MAX_UNIT_LEN 16
#define MAX_SUPPLIERS 8

typedef enum {
    CAT_OIL,
    CAT_RICE,
    CAT_SUGAR,
    CAT_FLOUR,
    CAT_CANNED,
    CAT_DRINKS,
    CAT_OTHER
} Category;

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    Category category;
    char unit[MAX_UNIT_LEN];
    int quantity_on_hand;
    int reorder_threshold;
    double unit_price;
    int supplier_ids[MAX_SUPPLIERS];
    int supplier_count;
} Product;

#ifdef __cplusplus
}
#endif

#endif