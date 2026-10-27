# Grocery Shop Inventory Management System

## Overview
A command-line application for managing grocery shop products, suppliers, stock transactions, and inventory reports. It uses C11 for core inventory operations and C++17 for the user interface.

## Features
- Add, edit, remove, search, and sort products.
- Manage supplier records.
- Record stock-in, stock-out, and stock adjustments.
- Validate stock quantities and prevent negative inventory.
- Generate stock, low-stock, transaction history, and inventory valuation reports.
- Save and reload inventory data using a text file.

## Technologies
- C11 and C++17
- Microsoft Visual C++ (MSVC) on Windows
- Git and GitHub for version control

## Project Structure
- `src/` — implementation files.
- `include/` — shared headers and data structure declarations.
- `data/inventory.txt` — persistent inventory records.
- `docs/` — code book and all relevant project documentation 
- `build/` — generated executable and object files.
- `Makefile` — build automation for compatible GCC/Clang environments.

## Build and Run on Windows
Open the **x64 Native Tools Command Prompt for Visual Studio** and navigate to the project root.

Create the build directory if needed:

```bat
if not exist build mkdir build
```

Compile the C modules:

```bat
cl /nologo /std:c11 /W4 /I include /c src\inventory.c /Fo:build\inventory.obj
cl /nologo /std:c11 /W4 /I include /c src\persistence.c /Fo:build\persistence.obj
cl /nologo /std:c11 /W4 /I include /c src\report.c /Fo:build\report.obj
```

Compile and link the C++ application:

```bat
cl /nologo /std:c++17 /EHsc /W4 /I include /c src\main.cpp /Fo:build\main.obj
cl /nologo /EHsc build\inventory.obj build\persistence.obj build\report.obj build\main.obj /Fe:build\grocery_inventory.exe
```

Run the application from the project root:

```bat
build\grocery_inventory.exe
```

The project also includes a Makefile for compatible GCC/Clang environments.

## Data Persistence
Inventory records are stored in `data/inventory.txt`. The file contains `[PRODUCTS]`, `[SUPPLIERS]`, and `[TRANSACTIONS]` sections, using pipe-separated fields. Keep the file in the expected location when running the application.

See [`docs/codebook.md`](docs/codebook.md) for the data structures, relationships, file format, and persistence details.

## Testing
The application has been exercised through its command-line interface, including inventory operations, transactions, reports, and saving and reloading data. Specific edge-case test results should be documented in the Phase 2 implementation report.
