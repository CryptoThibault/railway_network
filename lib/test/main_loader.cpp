#include "loader.hpp"
#include <iostream>

int main() {
    // Load the JSON data
    FieldMap data = Loader::loadJSON("data.json");

    // Access the "People" array
    const FieldVector& peopleVec = data.at("People");

    // Loop through each person
    for (size_t i = 0; i < peopleVec.size(); ++i)
    {
        const FieldMap& personMap = peopleVec[i];

        std::string name = personMap.at("name");
        int age = personMap.at("age");
        bool active = personMap.at("active");
    
        // Hobbies as vector<string>
        std::vector<std::string> hobbies = personMap.at("hobbies").asVectorOf<std::string>();

        // Print the person
        std::cout << "Person " << i+1 << ":\n";
        std::cout << "  Name: " << name << "\n";
        std::cout << "  Age: " << age << "\n";
        std::cout << "  Active: " << (active ? "true" : "false") << "\n";

        std::cout << "  Hobbies: ";
        for (const auto& h : hobbies) std::cout << h << " ";
        std::cout << "\n\n";
    }

    return 0;
}