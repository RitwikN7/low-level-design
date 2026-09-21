#include <iostream>

// Pre C++23
template <typename Derived>
class Base
{
public:
    void interface()
    {
        // The "cursed" traditional static downcast
        static_cast<Derived*>(this)->implementation();
    }
};

class Derived : public Base<Derived>
{
public:
    void implementation()
    {
        std::cout << "Derived implementation!\n";
    }
};

#include <concepts>
#include <iostream>

// Enforcing C++20 Concepts

// Define a Concept to enforce the static interface contract
template <typename T>
concept ImplementsStaticInterface = requires(T t) {
                                        { t.implementation() } -> std::same_as<void>;
                                    };

template <typename Derived>
class Base
{
public:
    void interface()
    {
        // Enforce constraint right here
        static_assert(ImplementsStaticInterface<Derived>,
                      "Derived class must implement 'void implementation()'");

        static_cast<Derived*>(this)->implementation();
    }
};

class SuccessClass : public Base<SuccessClass>
{
public:
    void implementation()
    {
        std::cout << "Works perfectly!\n";
    }
};

// C++23 Explicit Object Parameters (deducing "this")

// Base is NO LONGER a template class!
class ModernBase
{
public:
    // 'Self&& self' replaces the traditional 'this' pointer
    template <typename Self>
    void interface(this Self&& self)
    {
        // No explicit static_cast needed!
        // The compiler deduces if 'self' is a Derived reference.
        self.implementation();
    }
};

class ModernDerived : public ModernBase
{
public:
    void implementation()
    {
        std::cout << "Clean C++23 static polymorphism!\n";
    }
};

int main()
{
    ModernDerived d;
    d.interface(); // Automatically deduces 'Self' as ModernDerived
}
