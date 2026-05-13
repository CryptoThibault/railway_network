#pragma once
#include "singleton.hpp"
#include "field.hpp"
#include <vector>
#include <functional>
#include <stdexcept>

class Factory : public Singleton<Factory>
{
public:
    template <typename T>
    T create(const std::string& name, const FieldMap& map);
    
    template <typename T>
    std::vector<T> createAll(const std::string& name, const FieldVector& vec);

    template <typename T>
    void registerCreator(const std::string& name, const std::function<T(const FieldMap&)>& creator);

private:
    std::map<std::string, std::function<void*(const FieldMap&)>> _creators;
};

#include "factory.tpp"