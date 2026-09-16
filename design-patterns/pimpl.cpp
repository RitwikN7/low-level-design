// Header File Widget.h

#ifndef WIDGET_H
#define WIDGET_H

#include <memory>

class Widget
{
public:
    Widget();

    // The destructor MUST be declared in the header but defined in the .cpp file.
    // If defaulted here, the compiler complains about an "incomplete type".
    ~Widget();

    // Move semantics need to be explicitly declared and defined in the .cpp file
    Widget(Widget&& other) noexcept;
    Widget& operator=(Widget&& other) noexcept;

    // Public API
    void do_something();

private:
    struct Impl;                 // Forward declaration of the hidden struct
    std::unique_ptr<Impl> pimpl; // Opaque pointer to the implementation
};

#endif // WIDGET_H

// CPP File

#include "Widget.h"

#include <iostream>
#include <vector> // Example of a heavy dependency hidden from the header

// 1. Define the actual internal structure
struct Widget::Impl
{
    std::vector<int> internal_data; // Hidden from users

    void secret_logic()
    {
        std::cout << "Processing data hidden from the header!\n";
    }
};

// 2. Define the constructor and allocate the implementation object
Widget::Widget()
    : pimpl(std::make_unique<Impl>())
{
}

// 3. Define the destructor in the .cpp where Impl is a complete type
Widget::~Widget() = default;

// 4. Define move operations in the .cpp
Widget::Widget(Widget&& other) noexcept = default;
Widget& Widget::operator=(Widget&& other) noexcept = default;

// 5. Delegate public API functions to the implementation
void Widget::do_something()
{
    pimpl->internal_data.push_back(42);
    pimpl->secret_logic();
}
