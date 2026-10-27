#include "report.h"

#include <stdio.h>

void report_current_stock(const Inventory *inventory, FILE *output)
{
    size_t i;

    if (inventory == NULL || output == NULL) {
        return;
    }

    fprintf(output, "\nCURRENT INVENTORY\n");
    fprintf(output, "------------------------------------------------------------\n");
    fprintf(output, "%-6s %-24s %-10s %8s %10s\n",
            "ID", "Product", "Unit", "Quantity", "Price");
    fprintf(output, "------------------------------------------------------------\n");

    for (i = 0; i < inventory_product_count(inventory); ++i) {
        const Product *p = inventory_product_at(inventory, i);

        if (p != NULL) {
            fprintf(output, "%-6d %-24s %-10s %8d %10.2f\n",
                    p->id, p->name, p->unit,
                    p->quantity_on_hand, p->unit_price);
        }
    }
}

void report_low_stock(const Inventory *inventory, FILE *output)
{
    size_t i;
    int found = 0;

    if (inventory == NULL || output == NULL) {
        return;
    }

    fprintf(output, "\nLOW-STOCK PRODUCTS\n");

    for (i = 0; i < inventory_product_count(inventory); ++i) {
        const Product *p = inventory_product_at(inventory, i);

        if (p != NULL && p->quantity_on_hand <= p->reorder_threshold) {
            fprintf(output, "ID: %d | %s | Quantity: %d | Threshold: %d\n",
                    p->id, p->name, p->quantity_on_hand,
                    p->reorder_threshold);
            found = 1;
        }
    }

    if (!found) {
        fprintf(output, "No products are currently at or below their threshold.\n");
    }
}

void report_transaction_history(const Inventory *inventory, FILE *output)
{
    size_t i;

    if (inventory == NULL || output == NULL) {
        return;
    }

    fprintf(output, "\nTRANSACTION HISTORY\n");

    for (i = 0; i < inventory_transaction_count(inventory); ++i) {
        const Transaction *t = inventory_transaction_at(inventory, i);

        if (t != NULL) {
            const char *type_name =
                t->type == TX_IN ? "IN" :
                t->type == TX_OUT ? "OUT" : "ADJUSTMENT";

            fprintf(output,
                    "ID: %d | Date: %s | Type: %s | Product ID: %d "
                    "| Change: %+d | Supplier ID: %d | Note: %s\n",
                    t->id, t->timestamp, type_name, t->product_id,
                    t->quantity_delta, t->supplier_id, t->note);
        }
    }
}

double report_total_value(const Inventory *inventory)
{
    size_t i;
    double total = 0.0;

    if (inventory == NULL) {
        return 0.0;
    }

    for (i = 0; i < inventory_product_count(inventory); ++i) {
        const Product *p = inventory_product_at(inventory, i);

        if (p != NULL) {
            total += p->quantity_on_hand * p->unit_price;
        }
    }

    return total;
}