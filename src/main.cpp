
#include "inventory_handle.hpp"
#include "persistence.h"
#include "report.h"

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>

static const char *DATA_FILE = "data/inventory.txt";

static int readInt(const char *prompt)
{
    std::string input;

    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, input)) return -1;

        char *end = nullptr;
        long value = std::strtol(input.c_str(), &end, 10);

        while (end && std::isspace(static_cast<unsigned char>(*end)))
            ++end;

        if (end != input.c_str() && end && *end == '\0' &&
            value >= std::numeric_limits<int>::min() &&
            value <= std::numeric_limits<int>::max()) {
            return static_cast<int>(value);
        }

        std::cout << "Enter a valid whole number.\n";
    }
}

static double readDouble(const char *prompt)
{
    std::string input;

    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, input)) return -1.0;

        char *end = nullptr;
        double value = std::strtod(input.c_str(), &end);

        while (end && std::isspace(static_cast<unsigned char>(*end)))
            ++end;

        if (end != input.c_str() && end && *end == '\0' &&
            value >= 0.0 && value <= 1.0e12) {
            return value;
        }

        std::cout << "Enter a valid non-negative amount.\n";
    }
}

static std::string readText(const char *prompt)
{
    std::string input;

    while (true) {
        std::cout << prompt;

        if (!std::getline(std::cin, input)) return "";

        if (!input.empty() &&
            input.find('|') == std::string::npos) {
            return input;
        }

        std::cout << "Text cannot be empty or contain |.\n";
    }
}

template <size_t N>
static void copyText(char (&destination)[N], const std::string &source)
{
    std::snprintf(destination, N, "%s", source.c_str());
}

static bool chooseCategory(Product &product)
{
    std::cout << "\n1. Oil\n2. Rice\n3. Sugar\n4. Flour\n"
              << "5. Canned\n6. Drinks\n7. Other\n";

    switch (readInt("Choose category: ")) {
        case 1: product.category = CAT_OIL; return true;
        case 2: product.category = CAT_RICE; return true;
        case 3: product.category = CAT_SUGAR; return true;
        case 4: product.category = CAT_FLOUR; return true;
        case 5: product.category = CAT_CANNED; return true;
        case 6: product.category = CAT_DRINKS; return true;
        case 7: product.category = CAT_OTHER; return true;
        default: std::cout << "Invalid category.\n"; return false;
    }
}

static void displayProduct(const Product &p)
{
    std::cout << "\nID: " << p.id
              << "\nName: " << p.name
              << "\nCategory code: " << static_cast<int>(p.category)
              << "\nUnit: " << p.unit
              << "\nQuantity: " << p.quantity_on_hand
              << "\nReorder threshold: " << p.reorder_threshold
              << "\nUnit price: " << p.unit_price
              << "\nStock value: " << p.quantity_on_hand * p.unit_price
              << "\n";
}

static void listProducts(Inventory *inv)
{
    size_t count = inventory_product_count(inv);

    if (!count) {
        std::cout << "No products found.\n";
        return;
    }

    for (size_t i = 0; i < count; ++i) {
        const Product *p = inventory_product_at(inv, i);
        if (p) displayProduct(*p);
    }
}

static void addProduct(Inventory *inv)
{
    Product p{};

    copyText(p.name, readText("Product name: "));
    if (!chooseCategory(p)) return;
    copyText(p.unit, readText("Unit: "));

    p.quantity_on_hand = readInt("Opening quantity: ");
    p.reorder_threshold = readInt("Low-stock threshold: ");
    p.unit_price = readDouble("Unit price: ");

    if (p.quantity_on_hand < 0 || p.reorder_threshold < 0 ||
        p.unit_price < 0) {
        std::cout << "Invalid product details.\n";
        return;
    }

    int id = inventory_add_product(inv, &p);
    if (id > 0)
        std::cout << "Product added. ID: " << id << "\n";
    else
        std::cout << "Could not add product. Check for duplicate names.\n";
}

static void editProduct(Inventory *inv)
{
    int id = readInt("Product ID to edit: ");
    Product *old = inventory_find_product(inv, id);

    if (!old) {
        std::cout << "Product not found.\n";
        return;
    }

    Product p = *old;
    copyText(p.name, readText("New product name: "));
    if (!chooseCategory(p)) return;
    copyText(p.unit, readText("Unit: "));

    p.quantity_on_hand = readInt("Quantity: ");
    p.reorder_threshold = readInt("Low-stock threshold: ");
    p.unit_price = readDouble("Unit price: ");

    if (p.quantity_on_hand < 0 || p.reorder_threshold < 0 ||
        p.unit_price < 0) {
        std::cout << "Invalid details.\n";
        return;
    }

    std::cout << (inventory_edit_product(inv, id, &p)
        ? "Product updated.\n"
        : "Update failed. Check the name and details.\n");
}

static void removeProduct(Inventory *inv)
{
    int id = readInt("Product ID to remove: ");

    if (!inventory_find_product(inv, id)) {
        std::cout << "Product not found.\n";
        return;
    }

    if (readText("Type YES to confirm: ") != "YES") {
        std::cout << "Cancelled.\n";
        return;
    }

    std::cout << (inventory_remove_product(inv, id)
        ? "Product removed.\n"
        : "Cannot remove this product; it may have transaction history.\n");
}

static void searchProducts(Inventory *inv)
{
    std::string term = readText("Search product name: ");
    std::vector<Product *> results(inventory_product_count(inv));

    size_t found = inventory_search_products(
        inv, term.c_str(), results.data());

    if (!found) std::cout << "No matching products.\n";

    for (size_t i = 0; i < found; ++i)
        displayProduct(*results[i]);
}

static void sortProducts(Inventory *inv)
{
    std::cout << "1. Name\n2. Quantity\n3. Category\n";
    int choice = readInt("Sort by: ");

    if (choice == 1) inventory_sort_products_by_name(inv);
    else if (choice == 2) inventory_sort_products_by_quantity(inv);
    else if (choice == 3) inventory_sort_products_by_category(inv);
    else {
        std::cout << "Invalid option.\n";
        return;
    }

    listProducts(inv);
}

static void productMenu(Inventory *inv)
{
    while (true) {
        std::cout << "\n--- PRODUCTS ---\n"
                  << "1. Add\n2. Edit\n3. Remove\n4. Find by ID\n"
                  << "5. Search by name\n6. List all\n7. Sort\n0. Back\n";

        switch (readInt("Choose: ")) {
            case 1: addProduct(inv); break;
            case 2: editProduct(inv); break;
            case 3: removeProduct(inv); break;
            case 4: {
                int id = readInt("Product ID: ");
                Product *p = inventory_find_product(inv, id);
                if (p) displayProduct(*p);
                else std::cout << "Product not found.\n";
                break;
            }
            case 5: searchProducts(inv); break;
            case 6: listProducts(inv); break;
            case 7: sortProducts(inv); break;
            case 0: return;
            default: std::cout << "Invalid option.\n";
        }
    }
}

static void displaySupplier(const Supplier &s)
{
    std::cout << "\nSupplier ID: " << s.id
              << "\nName: " << s.name
              << "\nContact person: " << s.contact_person
              << "\nPhone: " << s.phone
              << "\nEmail: " << s.email << "\n";
}

static void listSuppliers(Inventory *inv)
{
    size_t count = inventory_supplier_count(inv);

    if (!count) {
        std::cout << "No suppliers found.\n";
        return;
    }

    for (size_t i = 0; i < count; ++i) {
        const Supplier *s = inventory_supplier_at(inv, i);
        if (s) displaySupplier(*s);
    }
}

static Supplier readSupplierDetails()
{
    Supplier s{};

    copyText(s.name, readText("Supplier name: "));
    copyText(s.contact_person, readText("Contact person: "));
    copyText(s.phone, readText("Phone: "));
    copyText(s.email, readText("Email: "));

    return s;
}

static void addSupplier(Inventory *inv)
{
    Supplier s = readSupplierDetails();
    int id = inventory_add_supplier(inv, &s);

    if (id > 0)
        std::cout << "Supplier added. ID: " << id << "\n";
    else
        std::cout << "Could not add supplier. Check for duplicate names.\n";
}

static void editSupplier(Inventory *inv)
{
    int id = readInt("Supplier ID to edit: ");

    if (!inventory_find_supplier(inv, id)) {
        std::cout << "Supplier not found.\n";
        return;
    }

    Supplier updated = readSupplierDetails();

    std::cout << (inventory_edit_supplier(inv, id, &updated)
        ? "Supplier updated.\n"
        : "Update failed. Check supplier details and name uniqueness.\n");
}

static void removeSupplier(Inventory *inv)
{
    int id = readInt("Supplier ID to remove: ");

    if (!inventory_find_supplier(inv, id)) {
        std::cout << "Supplier not found.\n";
        return;
    }

    if (readText("Type YES to confirm: ") != "YES") {
        std::cout << "Cancelled.\n";
        return;
    }

    std::cout << (inventory_remove_supplier(inv, id)
        ? "Supplier removed.\n"
        : "Cannot remove supplier while products or transactions refer to it.\n");
}

static void supplierMenu(Inventory *inv)
{
    while (true) {
        std::cout << "\n--- SUPPLIERS ---\n"
                  << "1. Add supplier\n2. Edit supplier\n"
                  << "3. Remove supplier\n4. Find supplier by ID\n"
                  << "5. List suppliers\n0. Back\n";

        switch (readInt("Choose: ")) {
            case 1: addSupplier(inv); break;
            case 2: editSupplier(inv); break;
            case 3: removeSupplier(inv); break;
            case 4: {
                int id = readInt("Supplier ID: ");
                Supplier *s = inventory_find_supplier(inv, id);
                if (s) displaySupplier(*s);
                else std::cout << "Supplier not found.\n";
                break;
            }
            case 5: listSuppliers(inv); break;
            case 0: return;
            default: std::cout << "Invalid option.\n";
        }
    }
}

static void recordStockTransaction(Inventory *inv, TransactionType type)
{
    int productId = readInt("Enter product ID: ");
    Product *product = inventory_find_product(inv, productId);

    if (!product) {
        std::cout << "Product not found.\n";
        return;
    }

    displayProduct(*product);

    int quantity = readInt(
        type == TX_IN ? "Quantity received: " :
        type == TX_OUT ? "Quantity sold/removed: " :
                         "Quantity change (+/-): ");

    if (quantity == 0 || quantity == -1) {
        std::cout << "Invalid quantity.\n";
        return;
    }

    int delta = quantity;

    if (type == TX_OUT) {
        if (quantity < 0) {
            std::cout << "Enter a positive quantity for stock out.\n";
            return;
        }

        if (quantity > product->quantity_on_hand) {
            std::cout << "Insufficient stock. Transaction cancelled.\n";
            return;
        }

        delta = -quantity;
    }

    if (type == TX_IN && quantity < 0) {
        std::cout << "Stock received must be positive.\n";
        return;
    }

    if (type == TX_ADJUSTMENT) {
        long long newQuantity =
            static_cast<long long>(product->quantity_on_hand) + delta;

        if (newQuantity < 0 || newQuantity > 2147483647LL) {
            std::cout << "Adjustment would create an invalid stock quantity.\n";
            return;
        }
    }

    Transaction transaction{};

    transaction.type = type;
    transaction.product_id = productId;
    transaction.quantity_delta = delta;
    transaction.supplier_id = -1;

    if (type == TX_IN) {
        int supplierId = readInt(
            "Supplier ID (enter 0 if not applicable): ");

        if (supplierId < 0) {
            std::cout << "Invalid supplier ID.\n";
            return;
        }

        if (supplierId > 0) {
            if (!inventory_find_supplier(inv, supplierId)) {
                std::cout << "Supplier not found.\n";
                return;
            }

            transaction.supplier_id = supplierId;
        }
    }

    std::string note = readText("Transaction note: ");
    copyText(transaction.note, note);

    std::time_t now = std::time(nullptr);
    std::tm localTime{};

    if (now == static_cast<std::time_t>(-1)) {
        std::cout << "Could not obtain the current time.\n";
        return;
    }

#ifdef _WIN32
    if (localtime_s(&localTime, &now) != 0) {
        std::cout << "Could not obtain the current time.\n";
        return;
    }
#else
    if (localtime_r(&now, &localTime) == nullptr) {
        std::cout << "Could not obtain the current time.\n";
        return;
    }
#endif

    if (std::strftime(
            transaction.timestamp,
            sizeof(transaction.timestamp),
            "%Y-%m-%dT%H:%M:%S",
            &localTime) == 0) {
        std::cout << "Could not format the timestamp.\n";
        return;
    }

    int transactionId = inventory_record_transaction(inv, &transaction);

    if (transactionId == 0) {
        std::cout << "Transaction failed. Stock was not changed.\n";
        return;
    }

    std::cout << "Transaction recorded successfully.\n"
              << "Transaction ID: " << transactionId
              << "\nUpdated quantity: " << product->quantity_on_hand
              << "\n";

   if (persistence_save(inv, DATA_FILE)) {
    std::cout << "Transaction and inventory saved successfully.\n";
} else {
    std::cerr << "ERROR: Automatic save failed.\n";
    std::cerr << "Check the data folder and inventory file path.\n";
}
}

static void showTransactionHistory(Inventory *inv)
{
    size_t count = inventory_transaction_count(inv);

    if (count == 0) {
        std::cout << "No transactions recorded.\n";
        return;
    }

    std::cout << "\n========== TRANSACTION HISTORY ==========\n";

    for (size_t i = 0; i < count; ++i) {
        const Transaction *t = inventory_transaction_at(inv, i);

        if (!t) {
            continue;
        }

        const char *typeName =
            t->type == TX_IN ? "STOCK IN" :
            t->type == TX_OUT ? "STOCK OUT" :
                                "ADJUSTMENT";

        std::cout << "\nTransaction ID: " << t->id
                  << "\nDate: " << t->timestamp
                  << "\nType: " << typeName
                  << "\nProduct ID: " << t->product_id
                  << "\nQuantity change: " << t->quantity_delta
                  << "\nSupplier ID: " << t->supplier_id
                  << "\nNote: " << t->note << "\n";
    }
}

static void transactionMenu(Inventory *inv)
{
    while (true) {
        std::cout << "\n========== TRANSACTIONS ==========\n"
                  << "1. Stock in\n"
                  << "2. Stock out\n"
                  << "3. Stock adjustment\n"
                  << "4. View transaction history\n"
                  << "0. Back to main menu\n";

        int choice = readInt("Choose an option: ");

        switch (choice) {
            case 1:
                recordStockTransaction(inv, TX_IN);
                break;
            case 2:
                recordStockTransaction(inv, TX_OUT);
                break;
            case 3:
                recordStockTransaction(inv, TX_ADJUSTMENT);
                break;
            case 4:
                showTransactionHistory(inv);
                break;
            case 0:
                return;
            default:
                std::cout << "Invalid option.\n";
                break;
        }
    }
}

static void reportsMenu(Inventory *inv)
{
    while (true) {
        std::cout << "\n--- REPORTS ---\n"
                  << "1. Current stock\n2. Low-stock products\n"
                  << "3. Total inventory value\n0. Back\n";

        switch (readInt("Choose: ")) {
            case 1: report_current_stock(inv, stdout); break;
            case 2: report_low_stock(inv, stdout); break;
            case 3:
                std::cout << "Total inventory value: "
              << report_total_value(inv) << "\n";
            break;
            case 0: return;
            default: std::cout << "Invalid option.\n";
        }
    }
}

int main()
{
    InventoryHandle inventory;

    if (!inventory) {
        std::fprintf(stderr, "Could not create inventory.\n");
        return 1;
    }

    if (!persistence_load(inventory.get(), DATA_FILE)) {
        std::fprintf(stderr, "Warning: Could not load inventory data.\n");
    }

    bool running = true;

    while (running) {
        std::cout << "\n============================================\n"
                  << " GROCERY SHOP INVENTORY MANAGEMENT SYSTEM\n"
                  << "============================================\n"
                  << "1. Products\n2. Suppliers\n3. Transactions\n"
                  << "4. Reports\n5. Save inventory\n6. Load inventory\n0. Exit\n";

        switch (readInt("Choose an option: ")) {
            case 1: productMenu(inventory.get()); break;
            case 2: supplierMenu(inventory.get()); break;
            case 3:
                transactionMenu(inventory.get());
             break;
            case 4: reportsMenu(inventory.get()); break;
            case 5:
                std::cout << (persistence_save(inventory.get(), DATA_FILE)
                    ? "Inventory saved.\n" : "Save failed.\n");
                break;
            case 6:
                std::cout << (persistence_load(inventory.get(), DATA_FILE)
                    ? "Load operation completed.\n" : "Load failed.\n");
                break;
            case 0:
                if (!persistence_save(inventory.get(), DATA_FILE))
                    std::cout << "Warning: Save failed.\n";
                else
                    std::cout << "Inventory saved. Goodbye!\n";
                running = false;
                break;
            default: std::cout << "Invalid option.\n";
        }
    }

    return 0;
}
