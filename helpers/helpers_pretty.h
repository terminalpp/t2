#pragma once

#include <ostream>

#define PRETTY(...) PrettyPrinter{__VA_ARGS__}

template<typename T>
void PrettyPrint(std::ostream & s, T const & x) { x.prettyPrint(s); }

template<>
inline void PrettyPrint(std::ostream & s, char const & x) { s << '\'' << x << "' (" << (int)x << ')'; }

template<typename T>
class PrettyPrinter {
public:
    PrettyPrinter(T const & x): x_{x} { }
    PrettyPrinter(PrettyPrinter const &) = delete;
    PrettyPrinter(PrettyPrinter &&) = delete;
    PrettyPrinter & operator = (PrettyPrinter const &) = delete;
    PrettyPrinter & operator = (PrettyPrinter &&) = delete;
private:
    T const & x_;

    friend std::ostream & operator << (std::ostream & s, PrettyPrinter const & p) {
        PrettyPrint(s, p.x_);
        return s;
    }
}; // PrettyPrinter

