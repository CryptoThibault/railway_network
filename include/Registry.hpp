#pragma once
#include <vector>
#include <cstddef>
#include <memory>
#include <string> 

template <typename T>
class Registry
{
private:
    std::vector<std::unique_ptr<T>> list;

    Registry() = default;
    ~Registry() = default;

public:
    inline static Registry& getInstance()
    {
        static Registry instance;
        return instance;
    }

    Registry(const Registry&) = delete;
    Registry& operator=(const Registry&) = delete;

    void add(std::unique_ptr<T> item)
    {
        list.push_back(std::move(item));
    }

    bool remove(T* obj)
    {
        for (auto it = list.begin(); it != list.end(); ++it)
        {
            if  (it->get() == obj) {
                list.erase(it);
                return true;
            }
        }
        return false;
    }

    T* findByName(const std::string& name) const
    {
        for (auto& s : list) {
            if (s->getName() == name)
                return s.get();
        }
        return nullptr;
    }

    T* get(size_t i) const { return (i < list.size()) ? list[i].get() : nullptr; }
    size_t size() const { return list.size(); }
    const std::vector<std::unique_ptr<T>>& getList() const { return list; }

    auto begin() { return list.begin(); }
    auto end() { return list.end(); }

    auto begin() const { return list.begin(); }
    auto end() const { return list.end(); }
};
