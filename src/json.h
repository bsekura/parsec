#pragma once

#include <string>
#include <vector>
#include <map>

enum class JSonValueType : uint8_t {
    Null,
    Bool,
    Number,
    String,
    Object,
    Array
};

class JSonValue;

using JSonString = std::string;
using JSonObject = std::map<std::string, JSonValue>;
using JSonArray = std::vector<JSonValue>;

class JSonValue {
public:
    JSonValue()
        : m_type{JSonValueType::Null}
    {}
    JSonValue(const JSonValue& other)
        : m_type{other.m_type}
    {
        switch (m_type) {
        case JSonValueType::Bool:
            m_bool = other.m_bool;
            break;
        case JSonValueType::Number:
            m_number = other.m_number;
            break;
        case JSonValueType::String:
            new (&m_string) auto(other.m_string);
            break;
        case JSonValueType::Object:
            new (&m_object) auto(other.m_object);
            break;
        case JSonValueType::Array:
            new (&m_array) auto(other.m_array);
            break;
        default:
            break;
        }
    }    
    JSonValue(JSonValue&& other)
        : m_type{other.m_type}
    {
        switch (m_type) {
        case JSonValueType::Bool:
            m_bool = other.m_bool;
            break;
        case JSonValueType::Number:
            m_number = other.m_number;
            break;
        case JSonValueType::String:
            new (&m_string) auto(std::move(other.m_string));
            other.m_string.~JSonString();
            other.m_type = JSonValueType::Null;
            break;
        case JSonValueType::Object:
            new (&m_object) auto(std::move(other.m_object));
            other.m_object.~JSonObject();
            other.m_type = JSonValueType::Null;
            break;
        case JSonValueType::Array:
            new (&m_array) auto(std::move(other.m_array));
            other.m_array.~JSonArray();
            other.m_type = JSonValueType::Null;
            break;
        default:
            break;
        }
    }
    JSonValue(bool val)
        : m_type{JSonValueType::Bool}
        , m_bool{val}
    {}
    JSonValue(int val)
        : m_type{JSonValueType::Number}
        , m_number{val}
    {}
    JSonValue(const char* val)
        : m_type{JSonValueType::String}
    {
        new (&m_string) std::string(val);
    }
    JSonValue(const std::string& val)
        : m_type{JSonValueType::String}
    {
        new (&m_string) auto(val);
    }
    JSonValue(std::string&& val)
        : m_type{JSonValueType::String}
    {
        new (&m_string) auto(std::move(val));
    }

    JSonValue(const JSonObject& val)
        : m_type{JSonValueType::Object}
    {
        new (&m_object) auto(val);
    }
    JSonValue(JSonObject&& val)
        : m_type{JSonValueType::Object}
    {
        new (&m_object) auto(std::move(val));
    }
    JSonValue(const JSonArray& val)
        : m_type{JSonValueType::Array}
    {
        new (&m_array) auto(val);
    }
    JSonValue(JSonArray&& val)
        : m_type{JSonValueType::Array}
    {
        new (&m_array) auto(std::move(val));
    }

    ~JSonValue() {
        switch (m_type) {
        case JSonValueType::Null:
        case JSonValueType::Bool:
        case JSonValueType::Number:
            break;
        case JSonValueType::String:
            m_string.~JSonString();
            break;
        case JSonValueType::Object:
            m_object.~JSonObject();
            break;
        case JSonValueType::Array:
            m_array.~JSonArray();
            break;
        }
    }

    JSonValueType type() const { return m_type; }
    bool boolean() const { return m_bool; }
    int number() const { return m_number; }
    const JSonString& string() const { return m_string; }
    const JSonObject& object() const { return m_object; }
    const JSonArray& array() const { return m_array; }

private:
    JSonValueType m_type;
    union {
        bool m_bool;
        int m_number;
        JSonString m_string;
        JSonObject m_object;
        JSonArray m_array;
    };    
};

void json_dump(const JSonObject& obj);
