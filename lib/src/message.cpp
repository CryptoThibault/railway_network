#include "message.hpp"

Message::Message(int type) : _type(type) {}

DataBuffer& Message::data() { return _data; }
const DataBuffer& Message::data() const { return _data; }
int Message::type() const { return _type; }