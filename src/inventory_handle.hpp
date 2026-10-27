#ifndef INVENTORY_HANDLE_HPP
#define INVENTORY_HANDLE_HPP

#include "inventory.h"
#include <utility>

class InventoryHandle {
public:
    InventoryHandle() : inventory_(inventory_create()) {}

    ~InventoryHandle()
    {
        inventory_destroy(inventory_);
    }

    InventoryHandle(const InventoryHandle&) = delete;
    InventoryHandle& operator=(const InventoryHandle&) = delete;

    InventoryHandle(InventoryHandle&& other) noexcept
        : inventory_(other.inventory_)
    {
        other.inventory_ = nullptr;
    }

    InventoryHandle& operator=(InventoryHandle&& other) noexcept
    {
        if (this != &other) {
            inventory_destroy(inventory_);
            inventory_ = other.inventory_;
            other.inventory_ = nullptr;
        }

        return *this;
    }

    Inventory *get() const
    {
        return inventory_;
    }

    explicit operator bool() const
    {
        return inventory_ != nullptr;
    }

private:
    Inventory *inventory_;
};

#endif