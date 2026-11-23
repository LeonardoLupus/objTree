#include <objtree/util/json.hpp>

#include <cctype>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace simplejson {

namespace {

struct Parser {
    const std::string& s;
    std::size_t pos = 0;

    explicit Parser(const std::string& t) : s(t) {}

    [[noreturn]] void fail(const std::string& msg) const {
        throw std::runtime_error("JSON error at " + std::to_string(pos) + ": " + msg);
    }

    void skip_ws() {
        while (pos < s.size() &&
               std::isspace(static_cast<unsigned char>(s[pos]))) ++pos;
    }

    char peek() {
        skip_ws();
        if (pos >= s.size()) fail("unexpected end");
        return s[pos];
    }

    void expect(char c) {
        skip_ws();
        if (pos >= s.size() || s[pos] != c) fail(std::string("expected '") + c + "'");
        ++pos;
    }

    std::string parse_string() {
        expect('"');
        std::string out;
        while (pos < s.size() && s[pos] != '"') {
            char c = s[pos++];
            if (c != '\\') { out += c; continue; }
            if (pos >= s.size()) fail("bad escape");
            char e = s[pos++];
            switch (e) {
                case '"':  out += '"';  break;
                case '\\': out += '\\'; break;
                case '/':  out += '/';  break;
                case 'b':  out += '\b'; break;
                case 'f':  out += '\f'; break;
                case 'n':  out += '\n'; break;
                case 'r':  out += '\r'; break;
                case 't':  out += '\t'; break;
                case 'u': {
                    if (pos + 4 > s.size()) fail("bad \\u escape");
                    unsigned code = 0;
                    for (int i = 0; i < 4; ++i) {
                        char h = s[pos++];
                        code <<= 4;
                        if      (h >= '0' && h <= '9') code |= unsigned(h - '0');
                        else if (h >= 'a' && h <= 'f') code |= unsigned(h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F') code |= unsigned(h - 'A' + 10);
                        else fail("bad hex in \\u");
                    }
                    out += (code < 0x80) ? static_cast<char>(code) : '?';
                    break;
                }
                default: fail("unknown escape");
            }
        }
        if (pos >= s.size()) fail("unterminated string");
        ++pos; // closing quote
        return out;
    }

    JsonValue parse_value() {
        char c = peek();
        if (c == '"') return parse_string();
        if (c == 't') { if (s.compare(pos,4,"true")  == 0) { pos += 4; return true;  } fail("bad literal"); }
        if (c == 'f') { if (s.compare(pos,5,"false") == 0) { pos += 5; return false; } fail("bad literal"); }
        if (c == 'n') { if (s.compare(pos,4,"null")  == 0) { pos += 4; return nullptr; } fail("bad literal"); }

        std::size_t start = pos;
        if (pos < s.size() && (s[pos] == '-' || s[pos] == '+')) ++pos;
        while (pos < s.size()) {
            char d = s[pos];
            if (std::isdigit(static_cast<unsigned char>(d)) ||
                d == '.' || d == 'e' || d == 'E' || d == '+' || d == '-') ++pos;
            else break;
        }
        if (pos == start) fail("expected value");
        try {
            return std::stod(s.substr(start, pos - start));
        } catch (...) {
            fail("bad number");
        }
    }

    JsonObject parse_object() {
        JsonObject obj;
        expect('{');
        skip_ws();
        if (pos < s.size() && s[pos] == '}') { ++pos; return obj; }
        while (true) {
            std::string key = parse_string();
            expect(':');
            obj[key] = parse_value();
            skip_ws();
            if (pos < s.size() && s[pos] == ',') { ++pos; continue; }
            break;
        }
        expect('}');
        return obj;
    }
};

std::string escape(const std::string& in) {
    std::string out;
    out.reserve(in.size() + 2);
    for (char c : in) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<int>(c));
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

std::string dump_value(const JsonValue& v) {
    if (std::holds_alternative<std::nullptr_t>(v)) return "null";
    if (auto b = std::get_if<bool>(&v))   return *b ? "true" : "false";
    if (auto d = std::get_if<double>(&v)) {
        std::ostringstream oss;
        oss << std::setprecision(10) << *d;
        return oss.str();
    }
    if (auto str = std::get_if<std::string>(&v)) return "\"" + escape(*str) + "\"";
    return "null";
}

} // namespace

JsonObject parse(const std::string& text) {
    Parser p(text);
    JsonObject obj = p.parse_object();
    p.skip_ws();
    if (p.pos != text.size()) p.fail("trailing garbage after object");
    return obj;
}

std::string dump(const JsonObject& obj, int indent) {
    std::string pad(static_cast<std::size_t>(std::max(0, indent)), ' ');
    std::ostringstream oss;
    oss << "{\n";
    std::size_t i = 0;
    for (const auto& [k, v] : obj) {
        oss << pad << "\"" << escape(k) << "\": " << dump_value(v);
        if (++i < obj.size()) oss << ',';
        oss << '\n';
    }
    oss << "}\n";
    return oss.str();
}

} // namespace simplejson