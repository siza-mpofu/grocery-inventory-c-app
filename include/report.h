#ifndef REPORT_H
#define REPORT_H

#include <stdio.h>
#include "inventory.h"

#ifdef __cplusplus
extern "C" {
#endif

void report_current_stock(const Inventory *inventory, FILE *output);
void report_low_stock(const Inventory *inventory, FILE *output);
void report_transaction_history(const Inventory *inventory, FILE *output);
double report_total_value(const Inventory *inventory);

#ifdef __cplusplus
}
#endif

#endif