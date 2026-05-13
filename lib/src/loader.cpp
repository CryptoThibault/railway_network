#include "loader.hpp"
#include <fstream>
#include <sstream>
#include <cctype>
#include <stdexcept>

Field Loader::loadJSON(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("Could not open file: " + filepath);

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string s = buffer.str();

    size_t i = 0;
    Field result = load(s, i);

    skipWhitespace(s, i);

    if (i != s.size()) {
        throw std::runtime_error("JSON: unexpected trailing content at position " + std::to_string(i));
    }

    return result;
}

Field Loader::load(const std::string& s, size_t& i)
{
    skipWhitespace(s, i);
    if (i >= s.size()) throw std::runtime_error("JSON: unexpected end of input");

    if (s[i] == '"')
    {
        std::string str;
        i++;
        while (i < s.size() && s[i] != '"')
        {
            if (s[i] == '\\' && i + 1 < s.size())
            {
                i++;
                switch (s[i])
                {
                    case '\"': str += '\"'; break;
                    case '\\': str += '\\'; break;
                    case '/': str += '/'; break;
                    case 'b': str += '\b'; break;
                    case 'f': str += '\f'; break;
                    case 'n': str += '\n'; break;
                    case 'r': str += '\r'; break;
                    case 't': str += '\t'; break;
                    default: str += s[i]; break;
                }
            }
            else str += s[i];
            i++;
        }
        if (i >= s.size()) throw std::runtime_error("JSON: unterminated string");
        i++;
        return str;
    }
    else if (std::isdigit(s[i]) || s[i] == '-')
    {
        size_t start = i;
        if (s[i] == '-') i++;
        while (i < s.size() && (std::isdigit(s[i]) || s[i] == '.' || s[i] == 'e' || s[i] == 'E')) i++;
        if (i > 0 && (s[i-1] == 'e' || s[i-1] == 'E') && i < s.size() && (s[i] == '-' || s[i] == '+'))
        {
            i++;
            while (i < s.size() && std::isdigit(s[i])) i++;
        }
        return std::stod(s.substr(start, i - start));
    }
    else if (s.compare(i, 4, "true") == 0) { i += 4; return true; }
    else if (s.compare(i, 5, "false") == 0) { i += 5; return false; }
    else if (s.compare(i, 4, "null") == 0) { i += 4; return nullptr; }
    else if (s[i] == '{') return loadObject(s, i);
    else if (s[i] == '[') return loadArray(s, i);

    throw std::runtime_error("JSON: unexpected character at position " + std::to_string(i));
}

FieldVector Loader::loadArray(const std::string& s, size_t& i)
{
    FieldVector vec;
    skipWhitespace(s, i);
    i++; // skip '['

    skipWhitespace(s, i);
    if (s[i] == ']') { i++; return vec; }

    while (true)
    {
        vec.push_back(load(s, i));

        skipWhitespace(s, i);
        if (s[i] == ',')
        {
            i++;
            skipWhitespace(s, i);
        }
        else if (s[i] == ']')
        {
            i++;
            break;
        }
        else
        {
            throw std::runtime_error("JSON: expected ',' or ']' in array");
        }
    }

    return vec;
}

FieldMap Loader::loadObject(const std::string& s, size_t& i)
{
    FieldMap map;
    skipWhitespace(s, i);
    i++;

    skipWhitespace(s, i);
    if (s[i] == '}') { i++; return map; }

    while (true)
    {
        if (s[i] != '"') throw std::runtime_error("JSON: expected string key in object");
        Field keyField = load(s, i);
        std::string key = keyField;

        skipWhitespace(s, i);
        if (s[i] != ':') throw std::runtime_error("JSON: expected ':' after object key");
        i++;

        skipWhitespace(s, i);
        Field value = load(s, i);
        map.emplace(key, value);

        skipWhitespace(s, i);
        if (s[i] == ',')
        {
            i++;
            skipWhitespace(s, i);
        }
        else if (s[i] == '}')
        {
            i++;
            break;
        }
        else
        {
            throw std::runtime_error("JSON: expected ',' or '}' in object");
        }
    }

    return map;
}

void Loader::skipWhitespace(const std::string& s, size_t& i)
{
    while (i < s.size() && std::isspace(s[i])) i++;
}