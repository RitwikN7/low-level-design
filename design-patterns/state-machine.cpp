#include <iostream>
#include <memory>

class VendingMachine;

class VendingMachineState
{
public:
    virtual ~VendingMachineState() = default;
    virtual void insertCoin(VendingMachine& machine) = 0;
    virtual void selectProduct(VendingMachine& machine) = 0;
    virtual void dispense(VendingMachine& machine) = 0;
};

class NoCoinState : public VendingMachineState
{
public:
    void insertCoin(VendingMachine& machine) override;
    void selectProduct(VendingMachine&) override
    {
        std::cout << "Insert coin first\n";
    }
    void dispense(VendingMachine&) override
    {
        std::cout << "Insert coin first\n";
    }
};

class HasCoinState : public VendingMachineState
{
public:
    void insertCoin(VendingMachine&) override
    {
        std::cout << "Coin already inserted\n";
    }
    void selectProduct(VendingMachine& machine) override;
    void dispense(VendingMachine&) override
    {
        std::cout << "Select product first\n";
    }
};

class DispenseState : public VendingMachineState
{
public:
    void insertCoin(VendingMachine&) override
    {
        std::cout << "Please wait, dispensing\n";
    }
    void selectProduct(VendingMachine&) override
    {
        std::cout << "Please wait, dispensing\n";
    }
    void dispense(VendingMachine& machine) override;
};

class VendingMachine
{
public:
    VendingMachine()
        : currentState(std::make_unique<NoCoinState>())
    {
    }

    void insertCoin()
    {
        currentState->insertCoin(*this);
    }
    void selectProduct()
    {
        currentState->selectProduct(*this);
    }
    void dispense()
    {
        currentState->dispense(*this);
    }

    void setState(std::unique_ptr<VendingMachineState> state)
    {
        currentState = std::move(state);
    }

private:
    std::unique_ptr<VendingMachineState> currentState;
};

void NoCoinState::insertCoin(VendingMachine& machine)
{
    std::cout << "Coin inserted\n";
    machine.setState(std::make_unique<HasCoinState>());
}

void HasCoinState::selectProduct(VendingMachine& machine)
{
    std::cout << "Product selected\n";
    machine.setState(std::make_unique<DispenseState>());
}

void DispenseState::dispense(VendingMachine& machine)
{
    std::cout << "Dispensing product\n";
    machine.setState(std::make_unique<NoCoinState>());
}

// Usage
// VendingMachine machine;
// machine.selectProduct();  // "Insert coin first"
// machine.insertCoin();     // "Coin inserted"
// machine.selectProduct();  // "Product selected"
// machine.dispense();       // "Dispensing product"
