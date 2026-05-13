template<typename T>
ThreadSafeIOStream& ThreadSafeIOStream::operator<<(const T& value)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_prefix.empty() && !_prefixAdded)
    {
        (*_output) << _prefix;
        _prefixAdded = true;
    }

    (*_output) << value;
    (*_output) << std::flush;

    std::ostringstream oss;
    oss << value;
    if (oss.str().find('\n') != std::string::npos)
        _prefixAdded = false;

    return *this;
}

template<typename T>
ThreadSafeIOStream& ThreadSafeIOStream::operator>>(T& value)
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::cin >> value;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return *this;
}

template<typename T>
void ThreadSafeIOStream::prompt(const std::string& question, T& dest)
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::cout << _prefix << question;
    std::cout.flush();
    std::cin >> dest;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}