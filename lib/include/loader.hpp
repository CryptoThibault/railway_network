#pragma once
#include "field.hpp"
#include <string>

class Loader
{
public:
    static Field loadJSON(const std::string& filepath);
private:
    static Field load(const std::string& s, size_t& i);
    static FieldVector loadArray(const std::string& s, size_t& i);
    static FieldMap loadObject(const std::string& s, size_t& i);
    static void skipWhitespace(const std::string& s, size_t& i);
};