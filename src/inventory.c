#include "inventory.h"

#include <stdlib.h>
#include <string.h>

struct Inventory {
    Product *products;
    size_t product_count;
    size_t product_capacity;

    Supplier *suppliers;
    size_t supplier_count;
    size_t supplier_capacity;

    Transaction *transactions;
    size_t transaction_count;
    size_t transaction_capacity;

    int next_product_id;
    int next_supplier_id;
    int next_transaction_id;
};

static int grow_array(void **array, size_t *capacity, size_t element_size)
{
    size_t new_capacity;
    void *new_array;

    if (*capacity > (size_t)-1 / 2) {
        return 0;
    }

    new_capacity = (*capacity == 0) ? 8 : *capacity * 2;

    if (element_size != 0 &&
        new_capacity > (size_t)-1 / element_size) {
        return 0;
    }

    new_array = realloc(*array, new_capacity * element_size);

    if (new_array == NULL) {
        return 0;
    }

    *array = new_array;
    *capacity = new_capacity;
    return 1;
}

Inventory *inventory_create(void)
{
    Inventory *inventory = calloc(1, sizeof(*inventory));

    if (inventory != NULL) {
        inventory->next_product_id = 1001;
        inventory->next_supplier_id = 1;
        inventory->next_transaction_id = 1;
    }

    return inventory;
}

void inventory_destroy(Inventory *inventory)
{
    if (inventory != NULL) {
        free(inventory->products);
        free(inventory->suppliers);
        free(inventory->transactions);
        free(inventory);
    }
}

int inventory_add_product(Inventory *inventory, const Product *product)
{
    Product copy;
    size_t i;

    if (inventory == NULL || product == NULL ||
        product->name[0] == '\0' ||
        product->unit[0] == '\0' ||
        product->quantity_on_hand < 0 ||
        product->reorder_threshold < 0 ||
        product->unit_price < 0.0 ||
        product->supplier_count < 0 ||
        product->supplier_count > MAX_SUPPLIERS) {
        return 0;
    }

    /* Reject duplicate names always; reject duplicate IDs only for
       explicit (loaded) IDs, since new products arrive with id == 0. */
    for (i = 0; i < inventory->product_count; ++i) {
        if (strcmp(inventory->products[i].name, product->name) == 0 ||
            (product->id > 0 &&
             inventory->products[i].id == product->id)) {
            return 0;
        }
    }

    if (inventory->product_count == inventory->product_capacity &&
        !grow_array((void **)&inventory->products,
                    &inventory->product_capacity, sizeof(Product))) {
        return 0;
    }

    copy = *product;

    if (copy.id > 0) {
        /* Preserve an existing ID when loading saved data. */
        if (copy.id >= inventory->next_product_id) {
            if (copy.id == 2147483647) {
                return 0;
            }

            inventory->next_product_id = copy.id + 1;
        }
    } else {
        /* Assign a new ID for a newly created product. */
        if (inventory->next_product_id <= 0 ||
            inventory->next_product_id == 2147483647) {
            return 0;
        }

        copy.id = inventory->next_product_id++;
    }

    inventory->products[inventory->product_count++] = copy;

    return copy.id;
}

Product *inventory_find_product(Inventory *inventory, int id)
{
    size_t i;

    if (inventory == NULL) {
        return NULL;
    }

    for (i = 0; i < inventory->product_count; ++i) {
        if (inventory->products[i].id == id) {
            return &inventory->products[i];
        }
    }

    return NULL;
}

int inventory_remove_product(Inventory *inventory, int id)
{
    size_t i;
    size_t j;

    if (inventory == NULL) {
        return 0;
    }

    for (i = 0; i < inventory->product_count; ++i) {
        if (inventory->products[i].id == id) {
            for (j = 0; j < inventory->transaction_count; ++j) {
                if (inventory->transactions[j].product_id == id) {
                    return 0;
                }
            }

            for (j = i; j + 1 < inventory->product_count; ++j) {
                inventory->products[j] = inventory->products[j + 1];
            }

            --inventory->product_count;
            return 1;
        }
    }

    return 0;
}

size_t inventory_product_count(const Inventory *inventory)
{
    return inventory == NULL ? 0 : inventory->product_count;
}

const Product *inventory_product_at(const Inventory *inventory, size_t index)
{
    if (inventory == NULL || index >= inventory->product_count) {
        return NULL;
    }

    return &inventory->products[index];
}

int inventory_edit_product(
    Inventory *inventory,
    int id,
    const Product *updated_product)
{
    size_t i;
    Product *existing;
    Product copy;

    if (inventory == NULL || updated_product == NULL ||
        updated_product->name[0] == '\0' ||
        updated_product->unit[0] == '\0' ||
        updated_product->quantity_on_hand < 0 ||
        updated_product->reorder_threshold < 0 ||
        updated_product->unit_price < 0.0 ||
        updated_product->supplier_count < 0 ||
        updated_product->supplier_count > MAX_SUPPLIERS) {
        return 0;
    }

    existing = inventory_find_product(inventory, id);

    if (existing == NULL) {
        return 0;
    }

    /* Prevent another product from having the same name. */
    for (i = 0; i < inventory->product_count; ++i) {
        if (inventory->products[i].id != id &&
            strcmp(inventory->products[i].name,
                   updated_product->name) == 0) {
            return 0;
        }
    }

    copy = *updated_product;

    /* Editing must never change the product's ID. */
    copy.id = id;

    *existing = copy;

    return 1;
}

size_t inventory_search_products(
    const Inventory *inventory,
    const char *search_term,
    Product **results)
{
    size_t i;
    size_t count = 0;
    size_t term_length;

    if (inventory == NULL || search_term == NULL ||
        results == NULL || search_term[0] == '\0') {
        return 0;
    }

    term_length = strlen(search_term);

    for (i = 0; i < inventory->product_count; ++i) {
        const char *name = inventory->products[i].name;
        size_t j;

        /* Case-insensitive search without modifying stored names. */
        for (j = 0; name[j] != '\0'; ++j) {
            size_t k;

            for (k = 0; k < term_length && name[j + k] != '\0'; ++k) {
                char a = name[j + k];
                char b = search_term[k];

                if (a >= 'A' && a <= 'Z') {
                    a = (char)(a - 'A' + 'a');
                }

                if (b >= 'A' && b <= 'Z') {
                    b = (char)(b - 'A' + 'a');
                }

                if (a != b) {
                    break;
                }
            }

            if (k == term_length) {
                results[count++] = &inventory->products[i];
                break;
            }
        }
    }

    return count;
}

/* Compare products alphabetically by name. */
static int cmp_name(const void *a, const void *b)
{
    const Product *pa = (const Product *)a;
    const Product *pb = (const Product *)b;

    return strcmp(pa->name, pb->name);
}

/* Compare products by quantity, from lowest to highest. */
static int cmp_quantity(const void *a, const void *b)
{
    const Product *pa = (const Product *)a;
    const Product *pb = (const Product *)b;

    if (pa->quantity_on_hand < pb->quantity_on_hand) {
        return -1;
    }

    if (pa->quantity_on_hand > pb->quantity_on_hand) {
        return 1;
    }

    return 0;
}

/* Compare products by category. */
static int cmp_category(const void *a, const void *b)
{
    const Product *pa = (const Product *)a;
    const Product *pb = (const Product *)b;

    if (pa->category < pb->category) {
        return -1;
    }

    if (pa->category > pb->category) {
        return 1;
    }

    return 0;
}

/* Sort products alphabetically by name using qsort. */
void inventory_sort_products_by_name(Inventory *inventory)
{
    if (inventory != NULL && inventory->product_count > 1) {
        qsort(inventory->products,
              inventory->product_count,
              sizeof(Product),
              cmp_name);
    }
}

/* Sort products by quantity, lowest first. */
void inventory_sort_products_by_quantity(Inventory *inventory)
{
    if (inventory != NULL && inventory->product_count > 1) {
        qsort(inventory->products,
              inventory->product_count,
              sizeof(Product),
              cmp_quantity);
    }
}

/* Sort products by category using qsort. */
void inventory_sort_products_by_category(Inventory *inventory)
{
    if (inventory != NULL && inventory->product_count > 1) {
        qsort(inventory->products,
              inventory->product_count,
              sizeof(Product),
              cmp_category);
    }
}

int inventory_add_supplier(Inventory *inventory, const Supplier *supplier)
{
    Supplier copy;
    size_t i;

    if (inventory == NULL || supplier == NULL ||
        supplier->name[0] == '\0') {
        return 0;
    }

    for (i = 0; i < inventory->supplier_count; ++i) {
        if ((supplier->id > 0 &&
             inventory->suppliers[i].id == supplier->id) ||
            strcmp(inventory->suppliers[i].name, supplier->name) == 0) {
            return 0;
        }
    }

    copy = *supplier;

    if (copy.id > 0) {
        /* Preserve an existing ID when loading saved supplier data. */
        if (copy.id >= inventory->next_supplier_id) {
            if (copy.id == 2147483647) {
                return 0;
            }

            inventory->next_supplier_id = copy.id + 1;
        }
    } else {
        /* Assign an ID to a new supplier. */
        if (inventory->next_supplier_id <= 0 ||
            inventory->next_supplier_id == 2147483647) {
            return 0;
        }

        copy.id = inventory->next_supplier_id++;
    }

    if (inventory->supplier_count == inventory->supplier_capacity &&
        !grow_array((void **)&inventory->suppliers,
                    &inventory->supplier_capacity, sizeof(Supplier))) {
        return 0;
    }

    inventory->suppliers[inventory->supplier_count++] = copy;

    return copy.id;
}

Supplier *inventory_find_supplier(Inventory *inventory, int id)
{
    size_t i;

    if (inventory == NULL) {
        return NULL;
    }

    for (i = 0; i < inventory->supplier_count; ++i) {
        if (inventory->suppliers[i].id == id) {
            return &inventory->suppliers[i];
        }
    }

    return NULL;
}

int inventory_edit_supplier(
    Inventory *inventory,
    int id,
    const Supplier *updated_supplier)
{
    size_t i;
    Supplier *existing;
    Supplier copy;

    if (inventory == NULL || updated_supplier == NULL ||
        updated_supplier->name[0] == '\0') {
        return 0;
    }

    existing = inventory_find_supplier(inventory, id);

    if (existing == NULL) {
        return 0;
    }

    /* Supplier names must be unique. */
    for (i = 0; i < inventory->supplier_count; ++i) {
        if (inventory->suppliers[i].id != id &&
            strcmp(inventory->suppliers[i].name,
                   updated_supplier->name) == 0) {
            return 0;
        }
    }

    copy = *updated_supplier;
    copy.id = id;

    *existing = copy;

    return 1;
}

int inventory_remove_supplier(Inventory *inventory, int id)
{
    size_t i;
    size_t j;

    if (inventory == NULL ||
        inventory_find_supplier(inventory, id) == NULL) {
        return 0;
    }

    /* Do not remove a supplier referenced by a product. */
    for (i = 0; i < inventory->product_count; ++i) {
        for (j = 0;
             j < (size_t)inventory->products[i].supplier_count;
             ++j) {
            if (inventory->products[i].supplier_ids[j] == id) {
                return 0;
            }
        }
    }

    /* Do not remove a supplier referenced by transaction history. */
    for (i = 0; i < inventory->transaction_count; ++i) {
        if (inventory->transactions[i].supplier_id == id) {
            return 0;
        }
    }

    for (i = 0; i < inventory->supplier_count; ++i) {
        if (inventory->suppliers[i].id == id) {
            for (j = i; j + 1 < inventory->supplier_count; ++j) {
                inventory->suppliers[j] =
                    inventory->suppliers[j + 1];
            }

            --inventory->supplier_count;
            return 1;
        }
    }

    return 0;
}

size_t inventory_supplier_count(const Inventory *inventory)
{
    return inventory == NULL ? 0 : inventory->supplier_count;
}

const Supplier *inventory_supplier_at(const Inventory *inventory, size_t index)
{
    if (inventory == NULL || index >= inventory->supplier_count) {
        return NULL;
    }

    return &inventory->suppliers[index];
}

int inventory_record_transaction(
    Inventory *inventory,
    const Transaction *transaction)
{
    Transaction copy;
    Product *product;
    size_t i;
    long long new_quantity;

    if (inventory == NULL || transaction == NULL ||
        transaction->quantity_delta == 0) {
        return 0;
    }

    product = inventory_find_product(inventory, transaction->product_id);

    if (product == NULL) {
        return 0;
    }

    if (transaction->type != TX_IN &&
        transaction->type != TX_OUT &&
        transaction->type != TX_ADJUSTMENT) {
        return 0;
    }

    if (transaction->type == TX_IN && transaction->quantity_delta < 0) {
        return 0;
    }

    if (transaction->type == TX_OUT && transaction->quantity_delta > 0) {
        return 0;
    }

    new_quantity = (long long)product->quantity_on_hand +
                   (long long)transaction->quantity_delta;

    if (new_quantity < 0 || new_quantity > 2147483647LL) {
        return 0;
    }

    for (i = 0; i < inventory->transaction_count; ++i) {
        if (transaction->id > 0 &&
            inventory->transactions[i].id == transaction->id) {
            return 0;
        }
    }

    copy = *transaction;

    if (copy.id > 0) {
        /* Preserve the ID of a saved transaction. */
        if (copy.id >= inventory->next_transaction_id) {
            if (copy.id == 2147483647) {
                return 0;
            }

            inventory->next_transaction_id = copy.id + 1;
        }
    } else {
        /* Assign an ID to a new transaction. */
        if (inventory->next_transaction_id <= 0 ||
            inventory->next_transaction_id == 2147483647) {
            return 0;
        }

        copy.id = inventory->next_transaction_id++;
    }

    if (inventory->transaction_count == inventory->transaction_capacity &&
        !grow_array((void **)&inventory->transactions,
                    &inventory->transaction_capacity,
                    sizeof(Transaction))) {
        return 0;
    }

    inventory->transactions[inventory->transaction_count++] = copy;
    product->quantity_on_hand = (int)new_quantity;

    return copy.id;
}

int inventory_load_transaction(
    Inventory *inventory,
    const Transaction *transaction)
{
    Transaction copy;
    size_t i;

    if (inventory == NULL || transaction == NULL ||
        transaction->id <= 0 ||
        transaction->product_id <= 0 ||
        transaction->quantity_delta == 0 ||
        transaction->timestamp[0] == '\0') {
        return 0;
    }

    if (transaction->type != TX_IN &&
        transaction->type != TX_OUT &&
        transaction->type != TX_ADJUSTMENT) {
        return 0;
    }

    for (i = 0; i < inventory->transaction_count; ++i) {
        if (inventory->transactions[i].id == transaction->id) {
            return 0;
        }
    }

    if (inventory->transaction_count == inventory->transaction_capacity &&
        !grow_array((void **)&inventory->transactions,
                    &inventory->transaction_capacity,
                    sizeof(Transaction))) {
        return 0;
    }

    copy = *transaction;

    if (copy.id >= inventory->next_transaction_id) {
        if (copy.id == 2147483647) {
            return 0;
        }

        inventory->next_transaction_id = copy.id + 1;
    }

    /* Restore history without applying its quantity change again. */
    inventory->transactions[inventory->transaction_count++] = copy;

    return copy.id;
}

size_t inventory_transaction_count(const Inventory *inventory)
{
    return inventory == NULL ? 0 : inventory->transaction_count;
}

const Transaction *inventory_transaction_at(
    const Inventory *inventory, size_t index)
{
    if (inventory == NULL || index >= inventory->transaction_count) {
        return NULL;
    }

    return &inventory->transactions[index];
}

void inventory_clear(Inventory *inventory)
{
    if (inventory == NULL) {
        return;
    }

    inventory->product_count = 0;
    inventory->supplier_count = 0;
    inventory->transaction_count = 0;

    inventory->next_product_id = 1001;
    inventory->next_supplier_id = 1;
    inventory->next_transaction_id = 1;
}