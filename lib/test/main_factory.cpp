#include "factory.hpp"
#include "loader.hpp"
#include <iostream>

class Person
{
public:
    std::string name;
    int age;
    bool active;
    std::vector<std::string> hobbies;

    Person(const std::string& n, int a, bool act, const std::vector<std::string>& h)
        : name(n), age(a), active(act), hobbies(h) {}
};

int main() {
    // Register creator of Person class in Factory
    Factory::instantiate();
    Factory::instance()->registerCreator<Person>(
        "Person",
        [](const FieldMap& m)
        {
            return Person(
                m.at("name"),
                m.at("age"),
                m.at("active"),
                m.at("hobbies").asVectorOf<std::string>()
            );
        }
    );

    // Load JSON file
    FieldMap data = Loader::loadJSON("data.json");
    FieldVector peopleVec = data.at("People");

    // Use Factory to create Person objects from a vector of People
    std::vector<Person> people = Factory::instance()->createAll<Person>("Person", peopleVec);
    FieldMap personMap = {
        {"name",    Field("eve")},
        {"age",     Field(20)},
        {"active",  Field(false)},
        {"hobbies", Field(FieldVector{ Field("running") })}
    };
    people.push_back(Factory::instance()->create<Person>("Person", personMap));

    // Print the people
    for (size_t i = 0; i < people.size(); ++i)
    {
            std::cout << "Person " << i + 1 << ":\n";
            std::cout << "  Name: " << people[i].name << "\n";
            std::cout << "  Age: " << people[i].age << "\n";
            std::cout << "  Active: " << (people[i].active ? "true" : "false") << "\n";

            std::cout << "  Hobbies: ";
            for (const auto& h : people[i].hobbies) std::cout << h << " ";
            std::cout << "\n\n";
    }

    return 0;
}