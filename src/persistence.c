
#include "persistence.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#endif

static FILE *open_file(const char *path, const char *mode)
{
#ifdef _WIN32
    FILE *file = NULL;

    if (fopen_s(&file, path, mode) != 0) {
        return NULL;
    }

    return file;
#else
    return fopen(path, mode);
#endif
}
static void trim_line(char *line)
{
    size_t length = strlen(line);

    while (length > 0 &&
           (line[length - 1] == '\n' || line[length - 1] == '\r')) {
        line[--length] = '\0';
    }
}

static int split_fields(char *line, char **fields, int max_fields)
{
    int count = 0;
    char *start = line;
    char *p = line;

    for (;;) {
        if (*p == '|' || *p == '\0') {
            if (count >= max_fields) {
                return -1;
            }

            fields[count++] = start;

            if (*p == '\0') {
                break;
            }

            *p = '\0';
            start = p + 1;
        }

        ++p;
    }

    return count;
}

static int parse_int(const char *text, int *value)
{
    char *end;
    long result;

    errno = 0;
    result = strtol(text, &end, 10);

    if (errno != 0 || end == text || *end != '\0' ||
        result < INT_MIN || result > INT_MAX) {
        return 0;
    }

    *value = (int)result;
    return 1;
}

static int parse_double_value(const char *text, double *value)
{
    char *end;
    double result;

    errno = 0;
    result = strtod(text, &end);

    if (errno != 0 || end == text || *end != '\0' ||
        !isfinite(result)) {
        return 0;
    }

    *value = result;
    return 1;
}

static int parse_category(const char *text, Category *category)
{
    static const char *names[] = {
        "Oil", "Rice", "Sugar", "Flour",
        "Canned", "Drinks", "Other"
    };
    int i;

    for (i = 0; i < 7; ++i) {
        if (strcmp(text, names[i]) == 0) {
            *category = (Category)i;
            return 1;
        }
    }

    return 0;
}

static const char *category_name(Category category)
{
    static const char *names[] = {
        "Oil", "Rice", "Sugar", "Flour",
        "Canned", "Drinks", "Other"
    };

    if (category < CAT_OIL || category > CAT_OTHER) {
        return "Other";
    }

    return names[category];
}

static const char *transaction_type_name(TransactionType type)
{
    switch (type) {
        case TX_IN:
            return "IN";
        case TX_OUT:
            return "OUT";
        case TX_ADJUSTMENT:
            return "ADJUSTMENT";
        default:
            return NULL;
    }
}

/*
 * Accept both the older numeric transaction format and the newer
 * readable format. This keeps existing inventory files compatible.
 */
static int parse_transaction_type(const char *text, TransactionType *type)
{
    int numeric_type;

    if (strcmp(text, "IN") == 0) {
        *type = TX_IN;
        return 1;
    }

    if (strcmp(text, "OUT") == 0) {
        *type = TX_OUT;
        return 1;
    }

    if (strcmp(text, "ADJUSTMENT") == 0) {
        *type = TX_ADJUSTMENT;
        return 1;
    }

    if (!parse_int(text, &numeric_type) ||
        numeric_type < TX_IN || numeric_type > TX_ADJUSTMENT) {
        return 0;
    }

    *type = (TransactionType)numeric_type;
    return 1;
}

static int write_product(FILE *file, const Product *product)
{
    size_t i;

    if (fprintf(file, "%d|%s|%s|%s|%d|%d|%.2f|",
                product->id, product->name,
                category_name(product->category), product->unit,
                product->quantity_on_hand, product->reorder_threshold,
                product->unit_price) < 0) {
        return 0;
    }

    for (i = 0; i < (size_t)product->supplier_count; ++i) {
        if (fprintf(file, "%s%d", i == 0 ? "" : ",",
                    product->supplier_ids[i]) < 0) {
            return 0;
        }
    }

    return fputc('\n', file) != EOF;
}

int persistence_load(Inventory *inventory, const char *path)
{
    FILE *file;
    char line[1024];
    int line_number = 0;
    enum { NONE, PRODUCTS, SUPPLIERS, TRANSACTIONS } section = NONE;

    if (inventory == NULL || path == NULL) {
        return 0;
    }

    file = open_file(path, "r");

    if (file == NULL) {
        return errno == ENOENT;
    }
    /* Replace existing inventory with the saved data. */
inventory_clear(inventory);

    while (fgets(line, sizeof(line), file) != NULL) {
        char *fields[16];
        int count;

        ++line_number;
        trim_line(line);

        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }

        if (strcmp(line, "[PRODUCTS]") == 0) {
            section = PRODUCTS;
            continue;
        }

        if (strcmp(line, "[SUPPLIERS]") == 0) {
            section = SUPPLIERS;
            continue;
        }

        if (strcmp(line, "[TRANSACTIONS]") == 0) {
            section = TRANSACTIONS;
            continue;
        }

        count = split_fields(line, fields, 16);

        if (count < 0) {
            fprintf(stderr, "Malformed record at line %d\n", line_number);
            continue;
        }

        if (section == PRODUCTS && count == 8) {
            Product product = {0};
            int id;
            char *token;

            if (!parse_int(fields[0], &id) || id <= 0 ||
                !parse_category(fields[2], &product.category) ||
                !parse_int(fields[4], &product.quantity_on_hand) ||
                !parse_int(fields[5], &product.reorder_threshold) ||
                !parse_double_value(fields[6], &product.unit_price) ||
                product.quantity_on_hand < 0 ||
                product.reorder_threshold < 0 ||
                product.unit_price < 0.0 ||
                strlen(fields[1]) >= sizeof(product.name) ||
                strlen(fields[3]) >= sizeof(product.unit)) {
                fprintf(stderr, "Invalid product at line %d\n", line_number);
                continue;
            }

            product.id = id;
            memcpy(product.name, fields[1], strlen(fields[1]) + 1);
            memcpy(product.unit, fields[3], strlen(fields[3]) + 1);

            token = fields[7];

            while (*token != '\0') {
                char *comma = strchr(token, ',');
                int supplier_id;

                if (comma != NULL) {
                    *comma = '\0';
                }

                if (product.supplier_count >= MAX_SUPPLIERS ||
                    !parse_int(token, &supplier_id) || supplier_id <= 0) {
                    fprintf(stderr, "Invalid supplier IDs at line %d\n",
                            line_number);
                    break;
                }

                product.supplier_ids[product.supplier_count++] = supplier_id;

                if (comma == NULL) {
                    break;
                }

                token = comma + 1;
            }

            if (inventory_add_product(inventory, &product) == 0) {
                fprintf(stderr, "Could not load product at line %d\n",
                        line_number);
            }

        } else if (section == SUPPLIERS && count == 5) {
            Supplier supplier = {0};
            int id;

            if (!parse_int(fields[0], &id) || id <= 0 ||
                strlen(fields[1]) >= sizeof(supplier.name) ||
                strlen(fields[2]) >= sizeof(supplier.contact_person) ||
                strlen(fields[3]) >= sizeof(supplier.phone) ||
                strlen(fields[4]) >= sizeof(supplier.email)) {
                fprintf(stderr, "Invalid supplier at line %d\n", line_number);
                continue;
            }

            supplier.id = id;
            memcpy(supplier.name, fields[1], strlen(fields[1]) + 1);
            memcpy(supplier.contact_person, fields[2],
                   strlen(fields[2]) + 1);
            memcpy(supplier.phone, fields[3], strlen(fields[3]) + 1);
            memcpy(supplier.email, fields[4], strlen(fields[4]) + 1);

            if (inventory_add_supplier(inventory, &supplier) == 0) {
                fprintf(stderr, "Could not load supplier at line %d\n",
                        line_number);
            }

        } else if (section == TRANSACTIONS && count == 7) {
            Transaction transaction = {0};
            int id;

            if (!parse_int(fields[0], &id) || id <= 0 ||
                !parse_transaction_type(fields[2], &transaction.type) ||
                !parse_int(fields[3], &transaction.product_id) ||
                !parse_int(fields[4], &transaction.quantity_delta) ||
                !parse_int(fields[5], &transaction.supplier_id) ||
                strlen(fields[1]) >= sizeof(transaction.timestamp) ||
                strlen(fields[6]) >= sizeof(transaction.note)) {
                fprintf(stderr, "Invalid transaction at line %d\n",
                        line_number);
                continue;
            }

            transaction.id = id;
            memcpy(transaction.timestamp, fields[1],
                   strlen(fields[1]) + 1);
            memcpy(transaction.note, fields[6], strlen(fields[6]) + 1);

            if (inventory_load_transaction(inventory, &transaction) == 0) {
                fprintf(stderr, "Could not load transaction at line %d\n",
                        line_number);
            }

        } else {
            fprintf(stderr, "Unexpected record at line %d\n", line_number);
        }
    }

    if (ferror(file)) {
        fclose(file);
        return 0;
    }

    return fclose(file) == 0;
}

int persistence_save(const Inventory *inventory, const char *path)
{
    char *temp_path;
    size_t path_length;
    FILE *file;
    size_t i;
    int ok = 1;

    if (inventory == NULL || path == NULL) {
        return 0;
    }

    path_length = strlen(path);

    if (path_length > (size_t)-1 - 5) {
        return 0;
    }

    temp_path = malloc(path_length + 5);

    if (temp_path == NULL) {
        return 0;
    }

    snprintf(temp_path, path_length + 5, "%s.tmp", path);
    file = open_file(temp_path, "w");

    if (file == NULL) {
        free(temp_path);
        return 0;
    }

    if (fprintf(file, "# Grocery inventory data\n[PRODUCTS]\n") < 0) {
        ok = 0;
    }

    for (i = 0; ok && i < inventory_product_count(inventory); ++i) {
        const Product *product = inventory_product_at(inventory, i);

        if (product == NULL || !write_product(file, product)) {
            ok = 0;
        }
    }

    if (ok && fprintf(file, "[SUPPLIERS]\n") < 0) {
        ok = 0;
    }

    for (i = 0; ok && i < inventory_supplier_count(inventory); ++i) {
        const Supplier *supplier = inventory_supplier_at(inventory, i);

        if (supplier == NULL ||
            fprintf(file, "%d|%s|%s|%s|%s\n",
                    supplier->id, supplier->name, supplier->contact_person,
                    supplier->phone, supplier->email) < 0) {
            ok = 0;
        }
    }

    if (ok && fprintf(file, "[TRANSACTIONS]\n") < 0) {
        ok = 0;
    }

    for (i = 0; ok && i < inventory_transaction_count(inventory); ++i) {
        const Transaction *transaction =
            inventory_transaction_at(inventory, i);
        const char *type_name;

        if (transaction == NULL) {
            ok = 0;
            break;
        }

        type_name = transaction_type_name(transaction->type);

        if (type_name == NULL ||
            fprintf(file, "%d|%s|%s|%d|%d|%d|%s\n",
                    transaction->id, transaction->timestamp,
                    type_name, transaction->product_id,
                    transaction->quantity_delta, transaction->supplier_id,
                    transaction->note) < 0) {
            ok = 0;
        }
    }

    if (ferror(file)) {
        ok = 0;
    }

    if (fclose(file) != 0) {
        ok = 0;
    }

    if (ok) {
#ifdef _WIN32
        if (!MoveFileExA(temp_path, path,
                         MOVEFILE_REPLACE_EXISTING |
                         MOVEFILE_WRITE_THROUGH)) {
            fprintf(stderr,
                    "Could not replace inventory file (Windows error %lu).\n",
                    (unsigned long)GetLastError());
            ok = 0;
        }
#else
        if (rename(temp_path, path) != 0) {
            fprintf(stderr, "Could not replace inventory file: %s\n",
                    strerror(errno));
            ok = 0;
        }
#endif
    }

    if (!ok) {
        remove(temp_path);
    }

    free(temp_path);
    return ok;
}