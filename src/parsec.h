#pragma once

#include <vector>
#include <string>
#include <functional>
#include <utility>

// TODO: replace parse stream with immutable container and iterators
struct ParseStream {
    std::vector<uint8_t> data;
    size_t pos = 0;

    ParseStream() = delete;
    ParseStream(std::vector<uint8_t> v)
        : data{std::move(v)}
    {}

    char peek() const {
        return (char)data[pos];
    }
    void advance() {
        ++pos;
    }
    size_t curpos() const { return pos; }
    void setpos(size_t a) { pos = a; }

    char next() {
        return (char)data[pos++];
    }
    bool has_data() const { return pos < data.size(); }
    bool empty() const { return !has_data(); }
};

template <typename T>
struct Parser {
    using ParseFunc = std::function<std::pair<bool, T>(ParseStream&)>;
    ParseFunc parse;

    Parser() = delete;
    Parser(const Parser&) = default;
    Parser(Parser&& p)
        : parse{std::move(p.parse)}
    {}
    Parser(ParseFunc&& f)
        : parse{std::move(f)}
    {}
};

template <typename T>
inline std::pair<bool, T> run_parser(const Parser<T>& p, ParseStream&& s) {
    auto r = p.parse(s);
    if (r.first && s.empty())
        return std::make_pair(true, r.second);

    return std::make_pair(false, T());
}

// Functor
// fmap :: (a -> b) -> f a -> f b
template <typename F, typename T, typename U = typename std::result_of<F(T)>::type>
inline Parser<U> fmap(F&& f, const Parser<T>& p) {
    auto r = [f, p](ParseStream& s) -> std::pair<bool, U> {
        auto a = p.parse(s);
        if (a.first)
            return std::make_pair(true, f(a.second));

        return std::make_pair(false, U());
    };
    return Parser<U>(r);
}

// Applicative
// (<*>) :: f (a -> b) -> f a -> f b 
template <typename F, typename T, typename U = typename std::result_of<F(T)>::type>
inline Parser<U> applicative(const Parser<F>& fp, const Parser<T>& p) {
    auto r = [fp, p](ParseStream& s) -> std::pair<bool, U> {
        auto f = fp.parse(s);
        if (f.first) {
            auto a = p.parse(s);
            if (a.first)
                return std::make_pair(true, f.second(a.second));
        }

        return std::make_pair(false, U());
    };
    return Parser<U>(r);
}

template <typename T>
struct Result {
    using type = T;
};

template <typename T>
struct Result <Parser<T>> {
    using type = T;
};

// Monad
// bind :: Parser a -> (a -> Parser b) -> Parser b
template <typename F, typename T, typename U = typename Result<typename std::result_of<F(T)>::type>::type>
inline Parser<U> bind(Parser<T> p, F&& f) {
    auto r = [p = std::move(p), f](ParseStream& s) -> std::pair<bool, U> {
        auto a = p.parse(s);
        if (a.first)
            return f(a.second).parse(s);

        return std::make_pair(false, U());
    };
    return Parser<U>(r);
}

// bind operator
template <typename F, typename T, typename U = typename Result<typename std::result_of<F(T)>::type>::type>
inline auto operator>>=(Parser<T> p, F&& f) -> Parser<U> {
    return bind(std::move(p), std::move(f));
}

// Monad unit
template <typename T>
inline Parser<T> unit(T t) {
    auto p = [t = std::move(t)](ParseStream&) {
        return std::make_pair(true, std::move(t));
    };
    return Parser<T>(p);
}

template <typename T>
inline Parser<T> failure() {
    auto p = [](ParseStream&) {
        return std::make_pair(false, T());
    };
    return Parser<T>(p);
}

template <typename T>
inline Parser<T> option(const Parser<T>& p, const Parser<T>& q) {    
    auto r = [p, q](ParseStream& s) {
        size_t pos = s.curpos();
        auto a = p.parse(s);
        if (a.first)
            return a;

        s.setpos(pos); // rewind
        return q.parse(s);
    };
    return Parser<T>(r);
}

template <typename T>
inline Parser<T> operator|(const Parser<T>& p, const Parser<T>& q) {
    return option(p, q);
}

inline Parser<std::string> many(const Parser<char>& v) {
    auto p = [v](ParseStream& s) {
        std::string r;
        while (1) {
            size_t pos = s.curpos();
            auto a = v.parse(s);
            if (!a.first) {
                s.setpos(pos);
                return std::make_pair(true, r);
            }

            r.push_back(a.second);
        }
    };
    return Parser<std::string>(p);
}

inline Parser<std::string> some(const Parser<char>& v) {
    auto p = [v](ParseStream& s) {
        std::string r;
        while (1) {
            size_t pos = s.curpos();
            auto a = v.parse(s);
            if (!a.first) {
                s.setpos(pos);
                return std::make_pair(r.size() > 0, r);
            }

            r.push_back(a.second);
        }
    };
    return Parser<std::string>(p);
}

template <typename T>
inline Parser<std::vector<T>> many_v(const Parser<T>& v) {
    auto p = [v](ParseStream& s) {
        std::vector<T> r;
        while (1) {
            size_t pos = s.curpos();
            auto a = v.parse(s);
            if (!a.first) {
                s.setpos(pos);
                return std::make_pair(true, r);
            }

            r.push_back(a.second);
        }
    };
    return Parser<std::vector<T>>(p);
}

template <typename T>
inline Parser<std::vector<T>> some_v(const Parser<T>& v) {    
    auto p = [v](ParseStream& s) {
        std::vector<T> r;
        while (1) {
            size_t pos = s.curpos();
            auto a = v.parse(s);
            if (!a.first) {
                s.setpos(pos);
                return std::make_pair(r.size() > 0, r);
            }

            r.push_back(a.second);
        }
    };
    return Parser<std::vector<T>>(p);
}

struct Empty {};

inline Parser<Empty> many_skip(const Parser<char>& v) {
    auto p = [v](ParseStream& s) {
        while (1) {
            size_t pos = s.curpos();
            auto a = v.parse(s);
            if (!a.first) {
                s.setpos(pos);
                return std::make_pair(true, Empty());
            }
        }
    };
    return Parser<Empty>(p);
}


inline Parser<char> item() {
    auto p = [](ParseStream& s) {
        if (!s.empty())
            return std::make_pair(true, s.next());

        return std::make_pair(false, '\0');
    };
    return Parser<char>(p);
}

template <typename P>
inline Parser<char> satisfy(P&& fp) {
    auto a = [fp](char c) {
        if (fp(c))
            return unit(c);
        else
            return failure<char>();
    };

    return bind(item(), a);
}

inline Parser<char> one_of(const char* str) {
    auto p = [str](ParseStream& s) {
        if (s.has_data()) {
            char c = s.peek();
            const char* p = str;
            while (*p) {
                if (*p++ == c) {
                    s.advance();
                    return std::make_pair(true, c);
                }
            }
        }

        return std::make_pair(false, '\0');
    };
    return Parser<char>(p);
}

inline bool match_string(ParseStream& s, const char* str, size_t len)
{
    size_t n = 0;
    while (n < len && s.has_data()) {
        char c = s.next();
        if (c != str[n])
            break;
        ++n;
    }

    return (n == len);
}

inline Parser<std::string> string(std::string str) {    
    auto p = [str = std::move(str)](ParseStream& s) {
        size_t pos = s.curpos();
        if (match_string(s, str.c_str(), str.length()))
            return std::make_pair(true, str);

        s.setpos(pos);
        return std::make_pair(false, std::string());
    };
    return Parser<std::string>(p);
}

inline Parser<Empty> cstring_skip(const char* str) {    
    auto p = [str](ParseStream& s) {
        size_t pos = s.curpos();
        if (match_string(s, str, strlen(str)))
            return std::make_pair(true, Empty());

        s.setpos(pos);
        return std::make_pair(false, Empty());
    };
    return Parser<Empty>(p);
}

inline Parser<std::string> spaces() {
    return many(one_of(" \t\r\n"));
}

inline Parser<Empty> spaces_skip() {
    return many_skip(one_of(" \t\r\n"));
}

//do { a <- p; spaces ; return a}
template <typename T>
inline Parser<T> token(const Parser<T>& p) {
    return p >>= [](T a) {
        return spaces_skip() >>= [a = std::move(a)](Empty) {
            return unit<T>(std::move(a));
        };
    };
}

inline Parser<std::string> reserved(const std::string& str) {
    return token(string(str));
}

inline Parser<Empty> reserved_cstr(const char* str) {
    return token(cstring_skip(str));
}

inline bool is_digit(char c) {
    return isdigit(c) != 0;
}

inline bool is_alphanumeric(char c) {
    return isalnum(c) != 0;
}

inline int to_int(const std::string& s) {
    return atoi(s.c_str());
}

inline Parser<char> digit() {
    return satisfy(is_digit);
}

inline Parser<char> alphanumeric() {
    return satisfy(is_alphanumeric);
}

inline Parser<int> natural() {
    return fmap(to_int, some(satisfy(is_digit)));
}

Parser<std::string> literal() {
    return some(alphanumeric()) >>= [](std::string s) {
        return unit(s);
    };
}

/*
number :: Parser Int
number = do
s <- string "-" <|> return []
cs <- some digit
return $ read (s ++ cs)
*/

inline Parser<int> number() {
    return option(string("-"), unit(std::string())) >>= [](const std::string& s) {
        return some(digit()) >>= [s](const std::string& cs) {
            return unit(to_int(s + cs));
        };
    };
}

/*
parens :: Parser a -> Parser a
parens m = do
reserved "("
n <- m
reserved ")"
return n
*/

template <typename T>
inline Parser<T> parens(Parser<T> p) {
    return reserved_cstr("(") >>= [p = std::move(p)](Empty) {
        return p >>= [](T n) {
            return reserved_cstr(")") >>= [n = std::move(n)](Empty) {
                return unit(n);
            };
        };
    };
}

template <typename T>
inline Parser<T> quoted(Parser<T> p) {
    return reserved_cstr("\"") >>= [p = std::move(p)](Empty) {
        return p >>= [](T n) {
            return reserved_cstr("\"") >>= [n = std::move(n)](Empty) {
                return unit(n);
            };
        };
    };
}

// comma followed by whitespace
inline Parser<Empty> comma() {
    //return satisfy([](char c) { return c == ','; });
    return satisfy([](char c) { return c == ','; }) >>= [](char) {
        return spaces_skip();
    };
}
