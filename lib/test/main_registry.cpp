#include <iostream>
#include "registry.hpp"

struct MyObject
{
    MyObject(int v, std::string n) : value(v), name(std::move(n)) {}

    void print() const
    {
        std::cout << name << ": " << value << "\n";
    }

    int value;
    std::string name;
};

int main()
{
    // Create the singleton instance of the registry
    Registry<MyObject>::instantiate();
    auto reg = Registry<MyObject>::instance();

    // Add objects to the registry
    reg->add(MyObject(10, "Alice"));
    reg->add(MyObject(20, "Bob"));
    reg->add(MyObject(30, "Charlie"));
    reg->add(MyObject(25, "David"));

    // Display all stored objects
    std::cout << "All objects:\n";
    for (auto& obj : reg->getAll()) {
        obj.print();
    }

    // Find the first object matching a condition
    std::cout << "\nFind value > 20:\n";
    MyObject* found = reg->find([](const MyObject& o){
        return o.value > 20;
    });

    if (found) {
        found->print();
    }

    // Remove all objects matching a condition
    std::cout << "\nRemove name starting with 'A'\n";
    reg->remove([](const MyObject& o){
        return o.name[0] == 'A';
    });

    // Display remaining objects
    for (auto& obj : reg->getAll()) {
        obj.print();
    }

    // Display current size
    std::cout << "\nSize: " << reg->size() << "\n";

    // Clear the registry
    reg->clear();
    std::cout << "After clear size: " << reg->size() << "\n";

    return 0;
}