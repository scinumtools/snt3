#include "snt/c/puq.h"

#include <cstring>
#include <exception>
#include <snt/puq/calc/calculator.h>
#include <snt/puq/quantity.h>
#include <stdexcept>
#include <string>

struct snt_puq_quantity {
    snt::puq::Quantity value;
    explicit snt_puq_quantity(snt::puq::Quantity q) : value(std::move(q)) {}
};
namespace {
    thread_local std::string last;
    int fail(snt_puq_error* e, const std::exception& x) {
        last = x.what();
        if (e) {
            e->code = 1;
            e->message = last.c_str();
        }
        return 1;
    }
} // namespace
extern "C" int snt_puq_quantity_eval(const char* expr, snt_puq_quantity** out, snt_puq_error* e) {
    try {
        if (!expr || !out)
            throw std::invalid_argument("expression and output are required");
        *out = new snt_puq_quantity(snt::puq::Calculator().eval(expr).value);
        if (e) {
            e->code = 0;
            e->message = nullptr;
        }
        return 0;
    } catch (const std::exception& x) {
        return fail(e, x);
    }
}
extern "C" int snt_puq_quantity_convert(const snt_puq_quantity* q, const char* units, snt_puq_quantity** out, snt_puq_error* e) {
    try {
        if (!q || !units || !out)
            throw std::invalid_argument("quantity, units, and output are required");
        *out = new snt_puq_quantity(q->value.convert(units));
        if (e) {
            e->code = 0;
            e->message = nullptr;
        }
        return 0;
    } catch (const std::exception& x) {
        return fail(e, x);
    }
}
extern "C" int snt_puq_quantity_format(const snt_puq_quantity* q, char* buf, size_t n, snt_puq_error* e) {
    try {
        if (!q || !buf || !n)
            throw std::invalid_argument("quantity, buffer, and capacity are required");
        auto s = q->value.to_string();
        if (s.size() + 1 > n)
            throw std::invalid_argument("output buffer is too small");
        std::memcpy(buf, s.c_str(), s.size() + 1);
        if (e) {
            e->code = 0;
            e->message = nullptr;
        }
        return 0;
    } catch (const std::exception& x) {
        return fail(e, x);
    }
}
extern "C" void snt_puq_quantity_free(snt_puq_quantity* q) {
    delete q;
}
