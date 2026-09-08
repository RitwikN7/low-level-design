/*
requirements:
1. multiple warehouses
2. handle inventory updates for a catalog of products
3. add stock to a specific warehouse
4. remove stock from a specific warehouse
5. alert system per warehouse per product
6. check availability: given a product and quantity, check which warehouse
7. Transfer stock between warehouses
8. Thread safe

entities:
1. warehouse
    - unordered_map<productID, quantity>
    - threshold for alerts
    - alerting object

2. alerting interface
    - alert based on productID, warehouseID, threshold, quantity

3. inventory manager
    - unordered_map<warehouseID, warehouse>
    - addStock(productID, warehouseID)
    - removeStock(productID, warehouseID) -> reject if not possible
    - checkStock(productID, quantity)
    - transferStock(warehouse1, warehouse2) -> reject if not possible
*/

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class IAlertSystem
{
public:
    virtual ~IAlertSystem() = default;

    virtual void alert(const std::string& warehouseID, const std::string& productID, int quantity,
                       int threshold) = 0;
};

class EmailAlertSystem : public IAlertSystem
{
    void alert(const std::string& warehouseID, const std::string& productID, int quantity,
               int threshold) override
    {
        std::cout << "Low stock alert: warehouse[" << warehouseID << "] product[" << productID
                  << "] quantity[" << quantity << "] threshold[" << threshold << "]\n";
    }
};

class FileAlertSystem : public IAlertSystem
{
public:
    FileAlertSystem()
    {
        file_.open(fileName_, std::ios::app);
    }

    void alert(const std::string& warehouseID, const std::string& productID, int quantity,
               int threshold) override
    {
        std::lock_guard<std::mutex> lock(mtx_);
        file_ << "Low stock alert: warehouse[" << warehouseID << "] product[" << productID
              << "] quantity[" << quantity << "] threshold[" << threshold << "]\n";
    }

    ~FileAlertSystem()
    {
        file_.close();
    }

private:
    std::string fileName_{"alerts.txt"};
    std::ofstream file_;
    std::mutex mtx_;
};

class Warehouse
{
public:
    Warehouse(std::string warehouseID, std::shared_ptr<IAlertSystem>& alertSystemPtr, int threshold)
        : warehouseID_(std::move(warehouseID)),
          alertSystemPtr_(alertSystemPtr),
          threshold_(threshold)
    {
    }

    const std::string& getWarehouseID() const
    {
        return warehouseID_;
    }

    void addStock(const std::string& productID, int quantity)
    {
        std::unique_lock<std::shared_mutex> lock(mtx_);

        auto iter = products_.find(productID);
        if (iter == products_.end())
            iter = products_.insert({productID, 0}).first;

        iter->second += quantity;
        std::cout << "Added product[" << productID << "] quantity[" << quantity << "] to warehouse["
                  << warehouseID_ << "]\n";
    }

    int checkStock(const std::string& productID)
    {
        std::shared_lock<std::shared_mutex> lock(mtx_);

        auto iter = products_.find(productID);
        if (iter == products_.end())
            return 0;

        return iter->second;
    }

    bool removeStock(const std::string& productID, int quantity)
    {
        std::unique_lock<std::shared_mutex> lock(mtx_);

        auto iter = products_.find(productID);
        if (iter == products_.end())
        {
            std::cerr << "No product with ID[" << productID << "] at warehouse[" << warehouseID_
                      << "]\n";
            return false;
        }

        if (quantity > iter->second)
        {
            std::cerr << "Product[" << productID << "] at warehouse[" << warehouseID_
                      << "] does not have enough quantity[" << iter->second
                      << "] to fullfill request[" << quantity << "]\n";
            return false;
        }

        iter->second -= quantity;

        if (iter->second < threshold_)
            alertSystemPtr_->alert(warehouseID_, productID, iter->second, threshold_);

        return true;
    }

private:
    std::string warehouseID_;
    std::unordered_map<std::string, int> products_;
    std::shared_ptr<IAlertSystem> alertSystemPtr_{nullptr};
    std::shared_mutex mtx_;
    int threshold_{};
};

class InventoryManager
{
public:
    InventoryManager(std::vector<std::unique_ptr<Warehouse>>& warehouses)
    {
        for (auto& warehouse : warehouses)
        {
            warehouses_.insert({warehouse->getWarehouseID(), std::move(warehouse)});
        }
    }

    void addStock(const std::string& warehouseID, const std::string& productID, int quantity)
    {
        auto iter = warehouses_.find(warehouseID);
        if (iter == warehouses_.end())
        {
            std::cerr << "No warehouse with ID[" << warehouseID << "]\n";
            return;
        }

        iter->second->addStock(productID, quantity);
    }

    void removeStock(const std::string& warehouseID, const std::string& productID, int quantity)
    {
        auto iter = warehouses_.find(warehouseID);
        if (iter == warehouses_.end())
        {
            std::cerr << "No warehouse with ID[" << warehouseID << "]\n";
            return;
        }

        if (iter->second->removeStock(productID, quantity))
        {
            std::cout << "Removed product[" << productID << "] quantity[" << quantity
                      << "] from warehouse[" << warehouseID << "]\n";
        }
    }

    void checkStock(const std::string& productID, int quantity)
    {
        for (const auto& iter : warehouses_)
        {
            int warehouseQty = iter.second->checkStock(productID);
            if (warehouseQty >= quantity)
            {
                std::cout << "Warehouse[" << iter.second->getWarehouseID() << "] Quantity["
                          << warehouseQty << "]\n";
            }
        }
    }

    void transferStock(const std::string& fromWarehouse, const std::string& toWarehouse,
                       const std::string& productID, int quantity)
    {
        auto fromIter = warehouses_.find(fromWarehouse);
        if (fromIter == warehouses_.end())
        {
            std::cerr << "No warehouse with ID[" << fromWarehouse << "]\n";
            return;
        }

        auto toIter = warehouses_.find(toWarehouse);
        if (toIter == warehouses_.end())
        {
            std::cerr << "No warehouse with ID[" << toWarehouse << "]\n";
            return;
        }

        if (fromIter->second->removeStock(productID, quantity))
        {
            toIter->second->addStock(productID, quantity);
            std::cout << "Transferred product[" << productID << "] quantity[" << quantity
                      << "from warehouse[" << fromWarehouse << "] to warehouse[" << toWarehouse
                      << "]\n";
        }
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Warehouse>> warehouses_;
};