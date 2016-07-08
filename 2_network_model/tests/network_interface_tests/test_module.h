#ifndef _TEST_MODULE_H_
#define _TEST_MODULE_H_

#include <string>
#include <sstream>
#include <stdexcept>

namespace fnm_tests {

    template<typename T1, typename T2>
    inline bool check_equal(
            T1 expected_value,
            T2 actual_value,
            const std::string& expected,
            const std::string& actual,
            const std::string& filename,
            const size_t line) {
        if (expected_value != actual_value) {
            std::stringstream ss;
            ss << "Expected: " << expected_value << " actual: " << actual_value << " || "
               << expected << " != " << actual << ". At "  << filename << ":" << line << ".";
            throw std::logic_error(ss.str());
            return false;
        }
        return true;
    }

    template<>
    inline bool check_equal<const char*, std::string>(
            const char* expected_value,
            std::string actual_value,
            const std::string& expected,
            const std::string& actual,
            const std::string& filename,
            const size_t line) {
        if (std::string(expected_value) != actual_value) {
            std::stringstream ss;
            ss << "Expected: " << expected_value << " actual: " << actual_value << " || "
               << expected << " != " << actual << ". At "  << filename << ":" << line << ".";
            throw std::logic_error(ss.str());
            return false;
        }
        return true;
    }

    #define CHECK_EQUAL(expected, actual)                                                           \
    {                                                                                               \
        const auto expected_value = expected;                                                       \
        const auto actual_value = actual;                                                           \
        check_equal(expected_value, actual_value, #expected, #actual, __FILE__, __LINE__ );         \
    }

}

#endif // _TEST_MODULE_H_

