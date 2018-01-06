#pragma once

#include "json.h"
#include "parsec.h"

// json parser

Parser<std::string> property_name() {
    return some(alphanumeric() | one_of(" _")) >>= [](std::string s) {
        return unit(s);
    };
}


inline Parser<JSonObject> json_object();

template <typename T>
inline Parser<T> value_parser();

inline Parser<bool> true_value() {
    return reserved_cstr("true") >>= [](Empty) {
        return unit(true);
    };
}

inline Parser<bool> false_value() {
    return reserved_cstr("false") >>= [](Empty) {
        return unit(false);
    };
}

template <>
inline Parser<bool> value_parser() {
    return true_value() | false_value();
}

template <>
inline Parser<int> value_parser() {
    return number();
}

// TODO: add escaped chars
//      parse escape char followed by char
Parser<std::string> string_value() {
    return many(alphanumeric() | one_of(" _")) >>= [](std::string s) {
        return unit(s);
    };
}

template <>
inline Parser<std::string> value_parser() {
    return quoted(string_value());
}

template <>
inline Parser<JSonObject> value_parser() {
    return json_object();
}

// array
inline Parser<JSonArray> json_array_rest();

template <typename T>
inline Parser<JSonArray> json_array_value() {
    return value_parser<T>() >>= [](T s) {
        return json_array_rest() >>= [s = std::move(s)](JSonArray a) {
            JSonArray r(std::move(a));
            r.emplace_back(JSonValue(std::move(s)));
            return unit(std::move(r));
        };    
    };    
}

inline Parser<JSonArray> json_array_contents() {
    return json_array_value<std::string>()
         | json_array_value<bool>()
         | json_array_value<int>()
         | json_array_value<JSonObject>();
}

inline Parser<JSonArray> json_array_rest() {
    return spaces_skip() >>= [](Empty) {
        return (comma() >>= [](Empty) { return json_array_contents(); })
            | unit(JSonArray());
    };
}

inline Parser<JSonArray> json_array() {
    return spaces_skip() >>= [](Empty) {
        return reserved_cstr("[") >>= [](Empty) {
            return json_array_contents() >>= [](JSonArray a) {
                return reserved_cstr("]") >>= [a = std::move(a)](Empty) {
                    return unit(std::move(a));
                };
            };
        };
    };
}

// property
using JSonProperty = std::pair<std::string, JSonValue>;

template <typename T>
inline Parser<JSonProperty> json_value(std::string name) {
    return value_parser<T>() >>= [name = std::move(name)](T t) {
        return unit(std::make_pair(std::move(name), JSonValue(std::move(t))));
    };
}

inline Parser<JSonProperty> json_value_array(std::string name) {
    return json_array() >>= [name = std::move(name)](JSonArray a) {
        return unit(std::make_pair(std::move(name), JSonValue(std::move(a))));
    };
}

inline Parser<JSonProperty> json_property() {
    return quoted(property_name()) >>= [](std::string name) {
        return reserved_cstr(":") >>= [name = std::move(name)](Empty) {
            return json_value<std::string>(name)
                 | json_value<bool>(name)
                 | json_value<int>(name)
                 | json_value<JSonObject>(name)
                 | json_value_array(name);
        };
    };
}

inline Parser<JSonObject> json_properties();

inline Parser<JSonObject> json_properties_rest() {
    return spaces_skip() >>= [](Empty) {
        return (comma() >>= [](Empty) { return json_properties(); })
            | unit(JSonObject());
    };
}

inline Parser<JSonObject> json_properties() {
    return json_property() >>= [](JSonProperty p) {
        return json_properties_rest() >>= [p = std::move(p)](JSonObject v) {
            JSonObject obj(std::move(v));
            obj.emplace(p);
            return unit(std::move(obj));
        };
    };
}

inline Parser<JSonObject> json_object() {
    return spaces_skip() >>= [](Empty) {
        return reserved_cstr("{") >>= [](Empty) {
            return json_properties() >>= [](JSonObject obj) {
                return reserved_cstr("}") >>= [obj = std::move(obj)](Empty) {
                    return unit(std::move(obj));
                };
            };
        };
    };
}
