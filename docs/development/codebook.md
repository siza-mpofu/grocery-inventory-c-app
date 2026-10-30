# Code Book — Grocery Shop Inventory Management System

## 1. Purpose and Technology

This code book documents the data structures, relationships, source modules, validation rules and persistence format of the Grocery Shop Inventory Management System. The application uses C11 for core inventory operations, persistence and reporting, and C++17 for the command-line interface.

## Language Split and Build Process

The application uses both C and C++ to separate the core inventory functionality from the application entry point.

-  Files ending in `.c` (`inventory.c`, `persistence.c`, and `report.c`) are compiled as C11, while `main.cpp` is compiled as C++17. They are compiled separately because the languages have different compilation rules and name-linking conventions.
- The final linking step uses the C++ compiler driver because the application entry point is written in C++. This ensures the C++ runtime and required C++ support are linked correctly alongside the compiled C object files.
- C++ normally changes function names during compilation (name mangling), while C uses different naming conventions. The `extern "C"` guards tell the C++ compiler to use C linkage for declarations of functions implemented in C. This allows `main.cpp` to call functions from the C modules successfully.

The guards belong in the shared header files that declare C functions and are included by the C++ source file. They are typically written as:

```c
#ifdef __cplusplus
extern "C" {
#endif

/* C function declarations */

#ifdef __cplusplus
}
#endif
```

The `__cplusplus` checks ensure that the guards affect C++ compilation only, so the same headers can also be included by C source files.

**Build sequence:** Compile each `.c` file as C, compile `main.cpp` as C++, then link the resulting object files using the C++ compiler driver. The final executable combines both languages into one application.

## 2. Data Structures

### 2.1 Product

A product represents an item held in the grocery shop's inventory. Its structure is declared in `include/product.h`.

| Field | Description |
|---|---|
| `id` | Unique product identifier |
| `name` | Product name |
| `category` | Category: Oil, Rice, Sugar, Flour, Canned, Drinks or Other |
| `unit` | Measurement unit, such as bottle, packet or kilogram |
| `quantity_on_hand` | Current quantity in stock |
| `reorder_threshold` | Stock level used to identify low-stock products |
| `unit_price` | Price per unit |
| `supplier_ids` | IDs of associated suppliers |
| `supplier_count` | Number of associated suppliers |

### 2.2 Supplier

A supplier represents a business or person supplying products. Its structure is declared in `include/supplier.h`.

| Field | Description |
|---|---|
| `id` | Unique supplier identifier |
| `name` | Supplier or business name |
| `contact_person` | Main contact person |
| `phone` | Contact telephone number |
| `email` | Contact email address |

### 2.3 Transaction

A transaction records a stock movement or adjustment. Its structure is declared in `include/transaction.h`.

| Field | Description |
|---|---|
| `id` | Unique transaction identifier |
| `timestamp` | Date and time in ISO 8601 format |
| `type` | Stock-in, stock-out or stock adjustment |
| `product_id` | ID of the affected product |
| `quantity_delta` | Signed change in stock quantity |
| `supplier_id` | Associated supplier ID, or `-1` when not applicable |
| `note` | Description of the transaction |

The `TransactionType` enumeration defines `TX_IN`, `TX_OUT` and `TX_ADJUSTMENT`.

## 3. In-Memory Organisation and Relationships

The application uses C structures to represent records and dynamic arrays to maintain collections of products, suppliers and transactions.

Relationships are represented through identifiers:

- A product can store the IDs of its associated suppliers.
- A transaction references the product affected by the stock movement through `product_id`.
- An incoming transaction can optionally reference a supplier through `supplier_id`.
- Transaction records preserve the history of stock movements for reporting.

When a transaction is recorded, the inventory logic updates the affected product's quantity and records the transaction. Stock-out operations must not reduce stock below zero. The exact handling of adjustment quantities follows the implemented transaction logic.

## 4. Source Modules

| File | Responsibility |
|---|---|
| `src/inventory.c` | Manages product, supplier and transaction records and core inventory operations |
| `src/persistence.c` | Saves inventory records to a file and loads stored records |
| `src/report.c` | Provides inventory reporting functions |
| `src/main.cpp` | Implements the command-line menu and user interaction |
| `src/inventory_handle.hpp` | Provides the C++ inventory wrapper |
| `include/product.h` | Declares the Product structure |
| `include/supplier.h` | Declares the Supplier structure |
| `include/transaction.h` | Declares the Transaction structure |
| `include/inventory.h` | Declares inventory operation interfaces |
| `include/persistence.h` | Declares persistence interfaces |
| `include/report.h` | Declares reporting interfaces |

## 5. Core Operations and Validation

The command-line interface provides menus for products, suppliers, transactions and reports, as well as saving and loading inventory.

The implemented operations include:

- Adding, editing, removing, finding, searching, listing and sorting products.
- Adding, editing, removing, finding and listing suppliers.
- Recording stock-in, stock-out and stock-adjustment transactions.
- Generating current-stock, low-stock, transaction-history and inventory-value reports.
- Saving inventory data and loading it again.

The design requires unique IDs, valid relationships between records, and non-negative stock. Products with transaction history must not be deleted, and a product retains a fixed measurement unit. Pipe characters (`|`) must not be used within text fields in the persistence format.

The test record should confirm the actual behaviour of invalid input, duplicate IDs, negative quantities, empty inventory, and missing or malformed files. A rule should only be reported as successfully tested when its behaviour has been verified.

## 6. Persistence and File Format

Inventory data is stored at the relative path `data/inventory.txt`. The file uses plain text with pipe-separated fields and three sections:

- `[PRODUCTS]`
- `[SUPPLIERS]`
- `[TRANSACTIONS]`

Example file structure:

```text
# Grocery inventory data
[PRODUCTS]
1001|Sunflower Oil 1L|Oil|bottle|49|12|3.49|
[SUPPLIERS]
[TRANSACTIONS]
1|2026-10-27T11:11:20|0|1001|3|-1|cooking oil in
```

This is an illustrative example. Actual records depend on the saved inventory.

### 6.1 Product record

```text
id|name|category|unit|quantity_on_hand|reorder_threshold|unit_price|supplier_ids
```

The fields represent the product ID, name, category, unit, quantity, reorder threshold, price per unit, and associated supplier IDs. The final field may be empty when no supplier IDs are stored.

### 6.2 Supplier record

```text
id|name|contact_person|phone|email
```

The fields represent the supplier ID, name, contact person, phone number and email address.

### 6.3 Transaction record

```text
id|timestamp|type|product_id|quantity_delta|supplier_id|note
```

The fields represent the transaction ID, timestamp, transaction type, affected product ID, quantity change, supplier ID and transaction note.

Transaction type values are stored numerically according to the enumeration: `0` for `TX_IN`, `1` for `TX_OUT`, and `2` for `TX_ADJUSTMENT`.

### 6.4 Save and load process

The persistence module writes inventory records to a temporary file before attempting to replace the existing file. Loading reads the file's sections and reconstructs records in memory.

The file must remain at `data/inventory.txt` relative to the working directory. The section headers and field order must be preserved, and text fields must not contain the pipe delimiter.

Missing-file and malformed-file handling should be verified against the implementation and recorded test results. Keep a backup before manually editing saved inventory data.

## 7. Build Environment

The application was compiled on Windows using Microsoft Visual C++ (MSVC), with C11 for the C modules and C++17 for the application entry point.

The root `README.md` provides the build and run commands. A Makefile is also included for compatible GCC/Clang environments.

## 8. Testing and Evidence
## Testing and Verification

The following tests were performed during Phase 2 implementation.

| Test | Expected behaviour | Observed result |
|---|---|---|
| Invalid menu input | Reject invalid input | Passed — validation message displayed. |
| Negative product quantity (-5) | Reject negative quantity | Passed — “Invalid product details.” displayed. |
| Duplicate product name | Prevent duplicate products | Passed — duplicate-name message displayed. |
| Low-stock report | Identify stock below the reorder threshold | Passed — Sugar had 7 units against a threshold of 8. |
| Total inventory value | Calculate and display total value | Passed — displayed 234.01. |
| Persistence across restart | Retain saved inventory data | Passed — Sunflower Oil 1L (49 units) and Sugar (7 units) remained after restarting. |
| Missing inventory file | Start without crashing when the file is missing | The application opened and displayed an empty stock report. |
| Malformed inventory file | Handle invalid records without crashing | The application displayed an empty stock report. Detailed error handling still requires verification. |

### Further Testing

Additional verification should cover an existing but empty inventory file, malformed records with different field counts, invalid supplier and transaction references, and automated regression tests. Search, sorting, editing, removal and transaction workflows should also be tested systematically if not already covered.



## 9. Scope and Future Improvements

Expiry-date tracking is outside the current scope. Potential improvements include automated unit tests, stronger validation and recovery for malformed files, a graphical user interface, and additional reports.
