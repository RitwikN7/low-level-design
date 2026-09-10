#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

class Observer
{
public:
    virtual ~Observer() = default;
    virtual void update(const std::string& symbol, double price) = 0;
};

class Subject
{
public:
    virtual ~Subject() = default;
    virtual void attach(Observer* observer) = 0;
    virtual void detach(Observer* observer) = 0;
    virtual void notifyObservers() = 0;
};

class Stock : public Subject
{
public:
    explicit Stock(std::string symbol)
        : symbol(std::move(symbol)),
          price(0.0)
    {
    }

    void attach(Observer* observer) override
    {
        observers.push_back(observer);
    }

    void detach(Observer* observer) override
    {
        std::ranges::remove(observers, observer), observers.end();
    }

    void setPrice(double newPrice)
    {
        price = newPrice;
        notifyObservers();
    }

    void notifyObservers() override
    {
        for (auto* observer : observers)
        {
            observer->update(symbol, price);
        }
    }

private:
    std::vector<Observer*> observers;
    std::string symbol;
    double price;
};

class PriceDisplay : public Observer
{
public:
    void update(const std::string& symbol, double price) override
    {
        std::cout << "Display updated: " << symbol << " = $" << price << "\n";
    }
};

class PriceAlert : public Observer
{
public:
    explicit PriceAlert(double threshold)
        : threshold(threshold)
    {
    }

    void update(const std::string& symbol, double price) override
    {
        if (price > threshold)
        {
            std::cout << "Alert! " << symbol << " exceeded $" << threshold << "\n";
        }
    }

private:
    double threshold;
};

// Usage
// Stock stock("AAPL");
// PriceDisplay display;
// PriceAlert alert(150.00);
// stock.attach(&display);
// stock.attach(&alert);
// stock.setPrice(145.00);
// stock.setPrice(155.00);
