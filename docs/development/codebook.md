# Code Book — Grocery Shop Inventory Management System

## 1. Purpose and Technology

This code book documents the data structures, relationships, source modules, validation rules, sorting operations and persistence format of the Grocery Shop Inventory Management System. The application uses C11 for core inventory operations, persistence and reporting, and C++17 for the command-line interface.

## Language Split and Build Process

The application uses both C and C++ to separate the core inventory functionality from the application entry point.

- Files ending in `.c` (`inventory.c`, `persistence.c`, and `report.c`) are compiled as C11, while `.cpp` files are compiled as C++17. They are compiled separately because the languages have different compilation rules and name-linking conventions.
- The final linking step uses the C++ compiler driver because the application entry point is written in C++. This ensures the C++ runtime and required C++ support are linked correctly alongside the compiled C object files.
- C++ normally changes function names during compilation (name mangling), while C uses different naming conventions. The `extern "C"` guards tell the C++ compiler to use C linkage for declarations of functions implemented in C. This allows C++ source files to call functions from the C modules successfully.

The guards belong in the shared header files that declare C functions and are included by C++ source files. They are typically written as:

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

**Build sequence:** Compile each `.c` file as C, compile the `.cpp` files as C++, then link the resulting object files using the C++ compiler driver. The final executable combines both languages into one application.

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
| `src/inventory.c` | Manages product, supplier and transaction records, validation, sorting and core inventory operations |
| `src/persistence.c` | Saves inventory records to a file and loads stored records |
| `src/report.c` | Provides inventory reporting functions |
| `src/main.cpp` | Implements the command-line menu and application entry point |
| `src/ui_input.cpp` | Implements reusable input-reading and input-validation functions |
| `src/ui_input.hpp` | Declares the reusable input functions used by the command-line interface |
| `src/inventory_handle.hpp` | Provides the C++ inventory wrapper |
| `include/product.h` | Declares the Product structure |
| `include/supplier.h` | Declares the Supplier structure |
| `include/transaction.h` | Declares the Transaction structure |
| `include/inventory.h` | Declares inventory operation interfaces |
| `include/persistence.h` | Declares persistence interfaces |
| `include/report.h` | Declares reporting interfaces |

Input-handling functions such as `readInt`, `readDouble`, `requireInt`, `requireDouble` and `readText` were extracted from `main.cpp` into a separate input module. This reduces the responsibility of the main application file and allows input handling to be reused.

## 5. Core Operations and Validation

The command-line interface provides menus for products, suppliers, transactions and reports, as well as saving and loading inventory.

The implemented operations include:

- Adding, editing, removing, finding, searching, listing and sorting products.
- Adding, editing, removing, finding and listing suppliers.
- Recording stock-in, stock-out and stock-adjustment transactions.
- Generating current-stock, low-stock, transaction-history and inventory-value reports.
- Saving inventory data and loading it again.

### 5.1 Validation Rules

The implemented validation rules include:

- Product names must be unique.
- Product IDs must be unique when an explicit ID is supplied, such as when loading saved records. Newly added products receive their IDs through the inventory logic.
- Product quantities cannot be negative.
- Stock-out operations cannot reduce available stock below zero.
- Products with transaction history must not be deleted.
- A product retains a fixed measurement unit.
- Pipe characters (`|`) must not be used within text fields in the persistence format.

### 5.2 Product Editing and Stock Changes

Product editing allows product details to be changed without directly modifying the current stock quantity.

Quantity is intentionally not editable through the product-editing operation. All stock changes must be recorded through transactions so that the stock movement history is preserved. This maintains an audit trail of stock received, stock issued and manual adjustments.

### 5.3 Sorting

Product sorting is implemented using the standard C library function `qsort()`.

The sorting functions use comparison functions to determine the ordering of products:

- `cmp_name` compares product names using `strcmp()`.
- `cmp_quantity` compares product quantities.
- `cmp_category` compares product categories.

The comparison functions are passed to `qsort()` together with the product array and its number of elements. This replaces manually implemented sorting loops and allows the standard library sorting function to perform the ordering.

### 5.4 Pointer Ownership and Lifetime

Functions that return pointers to products provide access to records stored inside the inventory's internal array. These pointers are not owned by the caller and must not be freed by the caller.

Pointers returned by product lookup and search operations should not be retained across operations that add, remove or otherwise modify the product array, because the array may be reallocated or its contents may change.

Callers should use these pointers only while the underlying product array remains valid.

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
1|2026-10-27T11:11:20|IN|1001|3|-1|cooking oil in
```

This is an illustrative example. Actual records depend on the saved inventory.

### 6.1 Product Record

```text
id|name|category|unit|quantity_on_hand|reorder_threshold|unit_price|supplier_ids
```

The fields represent the product ID, name, category, unit, quantity, reorder threshold, price per unit, and associated supplier IDs.

The final field may be empty when no supplier IDs are stored. For example:

```text
1001|Sunflower Oil 1L|Oil|bottle|49|12|3.49|
```

The trailing pipe represents the empty final supplier ID field.

### 6.2 Supplier Record

```text
id|name|contact_person|phone|email
```

The fields represent the supplier ID, name, contact person, phone number and email address.

### 6.3 Transaction Type Encoding

Transaction types are stored as readable text values rather than numeric enumeration values. The supported values are:

- `IN` – Stock received.
- `OUT` – Stock issued or sold.
- `ADJUSTMENT` – Manual stock adjustment.

During loading, these text values are converted into their corresponding `TransactionType` enumeration constants using string comparison.

The loader also accepts legacy numeric transaction type values (`0`, `1` and `2`) for compatibility with previously saved inventory files. New records are saved using the readable text values.

This approach avoids making the current saved file format dependent on the order of values in the enum declaration and improves readability and maintainability.

### 6.4 Save and Load Process

The persistence module writes inventory records to a temporary file before attempting to replace the existing file. This helps prevent the original inventory file from being overwritten by an incomplete save operation.

Loading reads the file's sections and reconstructs records in memory. When a file is successfully opened for loading, the existing inventory records are cleared before the saved records are read. This means loading replaces the current inventory contents rather than appending the loaded records to them.

The file must remain at `data/inventory.txt` relative to the working directory. The section headers and field order must be preserved, and text fields must not contain the pipe delimiter.

If the inventory file is missing, the application can start with an empty inventory. Malformed-file handling should be interpreted according to the actual parser behaviour and verified test results. Keep a backup before manually editing saved inventory data.

## 7. Build Environment

The application has been compiled and run successfully on Windows using two build environments:

- **Microsoft Visual C++ (MSVC):** C11 for the C modules and C++17 for the application entry point.
- **GCC/G++ with GNU Make:** MinGW-w64 GCC/G++ on Windows, using the supplied Makefile.

The Makefile compiles the C source files as C11 and the C++ source files as C++17, then links the object files using the C++ compiler driver.

The root `README.md` provides the prerequisites and build and run commands for both environments.

## 8. Testing and Verification

The following tests were performed during implementation.

| Test | Expected behaviour | Observed result |
|---|---|---|
| Invalid menu input | Reject invalid input | Passed — validation message displayed. |
| Negative product quantity (-5) | Reject negative quantity | Passed — “Invalid product details.” displayed. |
| Duplicate product name | Prevent duplicate products | Passed — duplicate-name message displayed. |
| Low-stock report | Identify stock below the reorder threshold | Passed — Sugar had 7 units against a threshold of 8. |
| Total inventory value | Calculate and display total value | Passed — displayed 234.01. |
| Persistence across restart | Retain saved inventory data | Passed — Sunflower Oil 1L (49 units) and Sugar (7 units) remained after restarting. |
| Missing inventory file | Start without crashing when the file is missing | The application opened and displayed an empty stock report. |
| Malformed inventory file | Handle invalid records without crashing | The application displayed an empty stock report. |

The application was also rebuilt and run after the input-module extraction, sorting changes and Makefile updates.

## 9. Scope and Future Improvements

Expiry-date tracking is outside the current scope. Potential improvements include automated unit tests, stronger validation and recovery for malformed files, a graphical user interface, and additional reports.
