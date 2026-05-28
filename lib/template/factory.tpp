template <typename T>
T Factory::create(const std::string& name, const FieldMap& map)
{
    auto it = _creators.find(name);
    if (it == _creators.end())
        throw std::runtime_error("Creator not found for: " + name);
    void* obj = it->second(map);
    T* typedObj = static_cast<T*>(obj);
    T result = std::move(*typedObj);
    delete typedObj;
    return std::move(result);
}

template <typename T>
std::vector<T> Factory::createAll(const std::string& name, const FieldVector& vec)
{
    std::vector<T> result;
    result.reserve(vec.size());
    for (const auto& obj : vec)
    {
        const FieldMap& map = obj;
        result.push_back(std::move(create<T>(name, map)));
    }
    return std::move(result);
}

template <typename T>
void Factory::registerCreator(const std::string& name, const std::function<T(const FieldMap&)>& creator)
{
    _creators[name] = [creator](const FieldMap& map) -> void* { return new T(creator(map)); };
}