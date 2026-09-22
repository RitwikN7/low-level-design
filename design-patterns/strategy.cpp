/*
    - Use strategy pattern to change behavior at runtime
*/

#include <iostream>
#include <memory>
#include <string>

class PaymentStrategy
{
public:
    virtual ~PaymentStrategy() = default;
    virtual bool pay(double amount) = 0;
};

class CreditCardPayment : public PaymentStrategy
{
public:
    explicit CreditCardPayment(std::string cardNumber)
        : cardNumber(std::move(cardNumber))
    {
    }

    bool pay(double amount) override
    {
        std::cout << "Paid " << amount << " with credit card\n";
        return true;
    }

private:
    std::string cardNumber;
};

class PayPalPayment : public PaymentStrategy
{
public:
    explicit PayPalPayment(std::string email)
        : email(std::move(email))
    {
    }

    bool pay(double amount) override
    {
        std::cout << "Paid " << amount << " with PayPal\n";
        return true;
    }

private:
    std::string email;
};

class ShoppingCart
{
public:
    void setPaymentStrategy(std::unique_ptr<PaymentStrategy> strategy)
    {
        paymentStrategy = std::move(strategy);
    }

    void checkout(double amount)
    {
        if (paymentStrategy)
        {
            paymentStrategy->pay(amount);
        }
    }

private:
    std::unique_ptr<PaymentStrategy> paymentStrategy;
};

// Usage
// ShoppingCart cart;
// cart.setPaymentStrategy(std::make_unique<CreditCardPayment>("1234-5678"));
// cart.checkout(100.00);
// cart.setPaymentStrategy(std::make_unique<PayPalPayment>("user@example.com"));
// cart.checkout(50.00);
