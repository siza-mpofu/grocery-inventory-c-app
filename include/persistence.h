#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "inventory.h"

#ifdef __cplusplus
extern "C" {
#endif

int persistence_load(Inventory *inventory, const char *path);
int persistence_save(const Inventory *inventory, const char *path);

#ifdef __cplusplus
}
#endif

#endif