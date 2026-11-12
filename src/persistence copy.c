#include "persistence.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
        result < -2147483647L - 1 || result > 2147483647L) {
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

    if (errno != 0 || end == text || *end != '\0') {
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

int persistence_load(Inventory *inventory, const char *path)
{
    FILE *file;
    char line[1024];
    int line_number = 0;
    enum { NONE, PRODUCTS, SUPPLIERS, TRANSACTIONS } section = NONE;

    if (inventory == NULL || path == NULL) {
        return 0;
    }

    file = fopen(path, "r");

    if (file == NULL) {
        if (errno == ENOENT) {
            return 1;
        }
        return 0;
    }

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
            fprintf(stderr, "Skipping malformed line %d\n", line_number);
            continue;
        }

        if (section == PRODUCTS && count == 8) {
            Product product = {0};
            int id;

            if (!parse_int(fields[0], &id) ||
                !parse_category(fields[2], &product.category) ||
                !parse_int(fields[4], &product.quantity_on_hand) ||
                !parse_int(fields[5], &product.reorder_threshold) ||
                !parse_double_value(fields[6], &product.unit_price)) {
                fprintf(stderr, "Skipping malformed line %d\n", line_number);
                continue;
            }

            product.id = id;
            snprintf(product.name, sizeof(product.name), "%s", fields[1]);
            snprintf(product.unit, sizeof(product.unit), "%s", fields[3]);

            /* Supplier IDs are optional comma-separated values. */
            if (fields[7][0] != '\0') {
                char *token = strtok(fields[7], ",");
                while (token != NULL &&
                       product.supplier_count < MAX_SUPPLIERS) {
                    int supplier_id;
                    if (parse_int(token, &supplier_id)) {
                        product.supplier_ids[product.supplier_count++] =
                            supplier_id;
                    }
                    token = strtok(NULL, ",");
                }
            }

            if (inventory_add_product(inventory, &product) == 0) {
                fprintf(stderr, "Could not load product on line %d\n",
                        line_number);
            }
        } else if (section == SUPPLIERS && count == 5) {
            Supplier supplier = {0};

            snprintf(supplier.name, sizeof(supplier.name), "%s", fields[1]);
            snprintf(supplier.contact_person,
                     sizeof(supplier.contact_person), "%s", fields[2]);
            snprintf(supplier.phone, sizeof(supplier.phone), "%s", fields[3]);
            snprintf(supplier.email, sizeof(supplier.email), "%s", fields[4]);

            if (inventory_add_supplier(inventory, &supplier) == 0) {
                fprintf(stderr, "Could not load supplier on line %d\n",
                        line_number);
            }
        } else if (section == TRANSACTIONS && count == 7) {
            Transaction transaction = {0};
            int type;
            int valid;

            valid = parse_int(fields[0], &transaction.id) &&
                    parse_int(fields[2], &type) &&
                    parse_int(fields[3], &transaction.product_id) &&
                    parse_int(fields[4], &transaction.quantity_delta) &&
                    parse_int(fields[5], &transaction.supplier_id);

            if (!valid || type < TX_IN || type > TX_ADJUSTMENT) {
                fprintf(stderr, "Skipping malformed line %d\n", line_number);
                continue;
            }

            transaction.type = (TransactionType)type;
            snprintf(transaction.timestamp, sizeof(transaction.timestamp),
                     "%s", fields[1]);
            snprintf(transaction.note, sizeof(transaction.note), "%s",
                     fields[6]);

            if (inventory_record_transaction(inventory, &transaction) == 0) {
                fprintf(stderr, "Could not load transaction on line %d\n",
                        line_number);
            }
        } else {
            fprintf(stderr, "Skipping malformed line %d\n", line_number);
        }
    }

    if (ferror(file)) {
        fclose(file);
        return 0;
    }

    if (fclose(file) != 0) {
        return 0;
    }

    return 1;
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
    temp_path = malloc(path_length + 5);

    if (temp_path == NULL) {
        return 0;
    }

    snprintf(temp_path, path_length + 5, "%s.tmp", path);
    file = fopen(temp_path, "w");

    if (file == NULL) {
        free(temp_path);
        return 0;
    }

    fprintf(file, "# Grocery inventory data\n[PRODUCTS]\n");

    for (i = 0; i < inventory_product_count(inventory); ++i) {
        const Product *p = inventory_product_at(inventory, i);
        size_t j;

        if (p == NULL) {
            ok = 0;
            break;
        }

        fprintf(file, "%d|%s|%s|%s|%d|%d|%.2f|",
                p->id, p->name, category_name(p->category), p->unit,
                p->quantity_on_hand, p->reorder_threshold, p->unit_price);

        for (j = 0; j < (size_t)p->supplier_count; ++j) {
            fprintf(file, "%s%d", j == 0 ? "" : ",", p->supplier_ids[j]);
        }

        fputc('\n', file);
    }

    fprintf(file, "[SUPPLIERS]\n");

    for (i = 0; ok && i < inventory_supplier_count(inventory); ++i) {
        const Supplier *s = inventory_supplier_at(inventory, i);

        if (s == NULL) {
            ok = 0;
            break;
        }

        fprintf(file, "%d|%s|%s|%s|%s\n",
                s->id, s->name, s->contact_person, s->phone, s->email);
    }

    fprintf(file, "[TRANSACTIONS]\n");

    for (i = 0; ok && i < inventory_transaction_count(inventory); ++i) {
        const Transaction *t = inventory_transaction_at(inventory, i);

        if (t == NULL) {
            ok = 0;
            break;
        }

        fprintf(file, "%d|%s|%d|%d|%d|%d|%s\n",
                t->id, t->timestamp, (int)t->type, t->product_id,
                t->quantity_delta, t->supplier_id, t->note);
    }

    if (ferror(file)) {
    ok = 0;
}

if (fclose(file) != 0) {
    ok = 0;
}

if (ok) {
    if (remove(path) != 0 && errno != ENOENT) {
        ok = 0;
    }

    if (ok && rename(temp_path, path) != 0) {
        ok = 0;
    }
}

if (!ok) {
    remove(temp_path);
}

    free(temp_path);
    return ok;
}