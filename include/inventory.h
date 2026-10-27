
#ifndef INVENTORY_H
#define INVENTORY_H

#include <stddef.h>
#include "product.h"
#include "supplier.h"
#include "transaction.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Inventory Inventory;

/* Inventory lifecycle */
Inventory *inventory_create(void);
void inventory_destroy(Inventory *inventory);

/* Products */
int inventory_add_product(Inventory *inventory, const Product *product);
Product *inventory_find_product(Inventory *inventory, int id);
int inventory_remove_product(Inventory *inventory, int id);
size_t inventory_product_count(const Inventory *inventory);
const Product *inventory_product_at(
    const Inventory *inventory, size_t index);

int inventory_edit_product(
    Inventory *inventory,
    int id,
    const Product *updated_product);

size_t inventory_search_products(
    const Inventory *inventory,
    const char *search_term,
    Product **results);

void inventory_sort_products_by_name(Inventory *inventory);
void inventory_sort_products_by_quantity(Inventory *inventory);
void inventory_sort_products_by_category(Inventory *inventory);

/* Suppliers */
int inventory_add_supplier(
    Inventory *inventory, const Supplier *supplier);

Supplier *inventory_find_supplier(Inventory *inventory, int id);
int inventory_edit_supplier(
    Inventory *inventory,
    int id,
    const Supplier *updated_supplier);

int inventory_remove_supplier(Inventory *inventory, int id);
size_t inventory_supplier_count(const Inventory *inventory);
const Supplier *inventory_supplier_at(
    const Inventory *inventory, size_t index);

/* Transactions */
int inventory_record_transaction(
    Inventory *inventory,
    const Transaction *transaction);

/* Restore transaction history without changing stock quantities. */
int inventory_load_transaction(
    Inventory *inventory,
    const Transaction *transaction);

size_t inventory_transaction_count(const Inventory *inventory);
const Transaction *inventory_transaction_at(
    const Inventory *inventory, size_t index);

#ifdef __cplusplus
}
#endif

#endif
