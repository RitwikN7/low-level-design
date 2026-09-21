#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

// 1. The Common Interface
class Animal
{
public:
    virtual ~Animal() = default;
    virtual void speak() const = 0;
};

// 2. The Central Registry / Factory Component
class AnimalRegistry
{
public:
    using CreatorFunc = std::function<std::unique_ptr<Animal>()>;

    // Prevent SIOF using a function-local static variable (Construct on First Use)
    static std::unordered_map<std::string, CreatorFunc>& getRegistryMap()
    {
        static std::unordered_map<std::string, CreatorFunc> instance;
        return instance;
    }

    // Register a creation function matching an identifier
    static bool registerType(const std::string& name, CreatorFunc func)
    {
        getRegistryMap()[name] = std::move(func);
        return true;
    }

    // Factory method to instantiate the object dynamically
    static std::unique_ptr<Animal> create(const std::string& name)
    {
        auto& map = getRegistryMap();
        auto it = map.find(name);
        if (it != map.end())
        {
            return it->second(); // Execute the creation lambda
        }
        return nullptr;
    }
};

// 3. Helper Class for Automatic Registration via Static Variables
template <typename T>
struct AnimalRegistrar
{
    AnimalRegistrar(const std::string& name)
    {
        AnimalRegistry::registerType(name, []() {
            return std::make_unique<T>();
        });
    }
};

// 4. Concrete Implementations that register themselves automatically
class Dog : public Animal
{
public:
    void speak() const override
    {
        std::cout << "Woof!\n";
    }
};

// Global static object forces registration before main() runs
static AnimalRegistrar<Dog> registerDog("dog");

class Cat : public Animal
{
public:
    void speak() const override
    {
        std::cout << "Meow!\n";
    }
};
static AnimalRegistrar<Cat> registerCat("cat");

// 5. Execution Lifecycle
int main()
{
    // Objects are created cleanly using only strings, completely decoupled
    auto myDog = AnimalRegistry::create("dog");
    auto myCat = AnimalRegistry::create("cat");

    if (myDog)
        myDog->speak(); // Outputs: Woof!
    if (myCat)
        myCat->speak(); // Outputs: Meow!

    return 0;
}
