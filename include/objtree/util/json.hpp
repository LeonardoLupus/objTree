#pragma once

#include <map>
#include <string>
#include <variant>

namespace simplejson {

// Значение: null | bool | number(double) | string.
// Массивов/вложенных объектов нет — конфиг плоский.
using JsonValue  = std::variant<std::nullptr_t, bool, double, std::string>;
using JsonObject = std::map<std::string, JsonValue>;

// Бросает std::runtime_error при синтаксической ошибке.
JsonObject  parse(const std::string& text);
std::string dump (const JsonObject& obj, int indent = 4);

} // namespace simplejson