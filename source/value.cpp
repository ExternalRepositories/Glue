#include <glue/anymap.h>
#include <glue/keys.h>
#include <glue/value.h>

using namespace glue;

MapValue glue::createAnyMap() { return MapValue{std::make_shared<AnyMap>()}; }

MapValue Value::asMap() const { return MapValue{data.getShared<Map>()}; }

AnyFunction Value::asFunction() const {
  if (auto f = data.as<AnyFunction>()) {
    return *f;
  } else {
    return AnyFunction();
  }
}

void MapValue::forEach(std::function<bool(const std::string &, Value)> f) const {
  data->forEach([&](auto &&key, auto &&value) { return f(key, value); });
}

std::vector<std::string> MapValue::keys() const {
  std::vector<std::string> keys;
  data->forEach([&](auto &&key, auto &&) {
    keys.push_back(key);
    return false;
  });
  return keys;
}

Value MapValue::get(const std::string &key) const {
  if (auto result = data->get(key)) {
    return result;
  } else if (Value extends = data->get(keys::extendsKey)) {
    if (MapValue map = extends.asMap()) {
      return map.get(key);
    } else if (auto callback = extends.asFunction()) {
      return callback(*this, key);
    }
  }
  return Value();
}

void MapValue::setExtends(Value v) const { (*this)[keys::extendsKey] = std::move(v); }