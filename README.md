# Grocery Shop Inventory Management System

A command-line grocery inventory management application developed using C11 and C++17. The system helps a small grocery shop manage products and suppliers, record stock movements, generate inventory reports, and save and load inventory data.

## Features

- Add, edit, remove, search and list products.
- Manage supplier records and associate suppliers with products.
- Record stock-in, stock-out and adjustment transactions.
- Maintain transaction history.
- Sort products by name, quantity and category.
- Generate current stock and low-stock reports.
- Calculate total inventory value.
- Save and load inventory records using a text file.
- Validate user input and stored records.

## Technologies Used

- C11 – inventory operations, persistence and reporting.
- C++17 – application entry point and command-line interface.
- MSVC – Windows compilation and linking.
- GCC/G++ (MinGW-w64) – Windows compilation and linking.
- GNU Make – build automation.
- Git and GitHub – version control.

## Prerequisites

The project has been built and run using both supported Windows toolchains.

### Option 1: Microsoft Visual C++ (MSVC)

Requirements:
- Windows operating system.
- Visual Studio or Visual Studio Build Tools.
- Desktop development with C++ workload.
- MSVC compiler.
- Visual Studio Developer Command Prompt.

### Option 2: GCC/G++ with MinGW-w64

Requirements:
- Windows operating system.
- MinGW-w64 GCC compiler.
- G++ compiler with C++17 support.
- GNU Make.
- Git Bash or another terminal with Make available.

Verify the installations:

```bash
gcc --version
g++ --version
make --version
```

## Quick Start

### 1. Clone the repository

```bash
git clone https://github.com/siza-mpofu/grocery-inventory-c-app.git
cd grocery-inventory-c-app
```

### 2. Build using MSVC

Open the Visual Studio Developer Command Prompt and navigate to the project directory.

Create the build directory if it does not already exist:

```cmd
mkdir build
```

Compile the C source files:

```cmd
cl /nologo /W4 /Iinclude /c src\inventory.c /Fo:build\inventory.obj
cl /nologo /W4 /Iinclude /c src\persistence.c /Fo:build\persistence.obj
cl /nologo /W4 /Iinclude /c src\report.c /Fo:build\report.obj
```

Compile the C++ source files:

```cmd
cl /nologo /std:c++17 /EHsc /W4 /Iinclude /Isrc /c src\ui_input.cpp /Fo:build\ui_input.obj
cl /nologo /std:c++17 /EHsc /W4 /Iinclude /Isrc /c src\main.cpp /Fo:build\main.obj
```

Link the object files:

```cmd
cl /nologo /EHsc build\main.obj build\ui_input.obj build\inventory.obj build\persistence.obj build\report.obj /Fe:build\grocery_inventory.exe
```

Run the application:

```cmd
build\grocery_inventory.exe
```

### 3. Build using GCC/G++ and Make

Open Git Bash and navigate to the project directory:

```bash
cd /c/Users/mpofu/Desktop/grocery-inventory-c-cpp
```

For a fresh build, run:

```bash
make clean
make
```

The Makefile compiles the C source files using GCC, compiles the C++ source files using G++, and links the resulting object files using G++.

Run the application:

```bash
./build/grocery_inventory
```

To remove generated build files:

```bash
make clean
```

## Project Structure

```text
grocery-inventory-c-cpp/
│
├── build/                    # Compiled object files and executable
│
├── data/
│   └── inventory.txt         # Persistent inventory data
│
├── include/
│   ├── inventory.h           # Inventory declarations
│   ├── persistence.h         # Persistence declarations
│   └── report.h              # Reporting declarations
│
├── src/
│   ├── main.cpp              # Application entry point and menu
│   ├── inventory.c           # Core inventory operations
│   ├── persistence.c         # Saving and loading inventory data
│   ├── report.c              # Reports and calculations
│   ├── inventory_handle.hpp  # C++ inventory ownership wrapper
│   ├── ui_input.cpp          # Input handling implementation
│   └── ui_input.hpp          # Input handling declarations
│
├── Makefile                  # GCC/G++ build automation
├── README.md                 # Project documentation
└── .gitignore                # Excluded files
```

## Data Storage and Persistence

The application stores inventory information in:

```text
data/inventory.txt
```

The file uses a section-based, pipe-delimited text format containing three sections:

- `[PRODUCTS]` – product details, quantities, prices and supplier associations.
- `[SUPPLIERS]` – supplier contact information.
- `[TRANSACTIONS]` – stock movements and transaction history.

The application uses this file to retain inventory information between sessions.

### Missing data file

If the inventory file does not exist, the loader treats the missing-file condition as a successful empty-inventory initialisation. The application can therefore start without pre-existing inventory records. Inventory data can subsequently be saved to the configured file location.

## Exit Codes

| Exit code | Meaning |
|---|---|
| `0` | Successful application completion. |
| `1` | Inventory could not be created. |

## Testing

Testing during development covered:

- Invalid menu input.
- Negative opening quantities.
- Duplicate product names.
- Low-stock identification.
- Total inventory value calculation.
- Saving and reloading inventory records.
- Application compilation and execution using MSVC and GCC/G++ on Windows.


## Known Limitations

- The application is a local command-line prototype rather than a multi-user system.
- Inventory records are stored in a local text file rather than a database.
- GCC/G++ support has been verified on Windows using MinGW-w64; Clang and non-Windows environments are not currently documented as supported build targets.

## Future Improvements

- Expand automated regression testing for persistence and malformed records.
- Improve error recovery and diagnostic reporting.
- Consider database integration for larger inventories and multiple users.
- Investigate additional compiler and operating-system support.
