#pragma once 
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <stdexcept>

class Field;
using FieldVector = std::vector<Field>;
using FieldMap    = std::map<std::string, Field>;

class Field
{
public:
    using Variant = std::variant<std::string, double, bool, std::nullptr_t, FieldVector, FieldMap>;

    Field();
    Field(const std::string& s);
    Field(const char* s);
    Field(double n);
    Field(float n);
    Field(int n);
    Field(long n);
    Field(bool b);
    Field(std::nullptr_t);
    Field(const FieldVector& v);
    Field(const FieldMap& m);

    bool isString() const;
    bool isNumber() const;
    bool isBool() const;
    bool isNull() const;
    bool isVector() const;
    bool isMap() const;

    operator std::string() const;
    operator double() const;
    operator float() const;
    operator int() const;
    operator long() const;
    operator bool() const;
    operator std::nullptr_t() const;
    operator FieldVector() const;
    operator FieldMap() const;

    template<typename T>
    std::vector<T> asVectorOf() const;

    template<typename T>
    std::map<std::string, T> asMapOf() const;

private:
    Variant _value;
};

#include "field.tpp"