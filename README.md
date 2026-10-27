# Grocery Shop Inventory Management System

A command-line inventory management application for a grocery shop, implemented using a hybrid C and C++ design.

## Features

- Product and supplier records
- Inventory transaction tracking
- Stock and low-stock reports
- Inventory valuation
- Text-file persistence

## Technology

- C11 for inventory management, persistence, and reports
- C++17 for the application entry point and RAII wrapper
- Make for building the application

## Project Structure

- `include/` — shared header files
- `src/` — C and C++ source files
- `data/` — saved inventory data
- `docs/` — project documentation
- `build/` — generated build output

## Build

From the project root, run:

```bash
make
```

## Run

On Linux or a compatible shell:

```bash
./build/grocery_inventory
```

On Windows, the executable may be named `grocery_inventory.exe`, depending on the compiler and build environment.

## Clean

```bash
make clean
```

## Data Format

Inventory data is stored in `data/inventory.txt` using pipe-separated fields in the PRODUCTS, SUPPLIERS, and TRANSACTIONS sections.

## Status

This is the initial implementation and requires compilation and functional testing before submission.