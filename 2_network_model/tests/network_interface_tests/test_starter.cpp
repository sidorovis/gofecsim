#include <iostream>

#include "test_register.h"

namespace _fnm_test_ {

    typedef void (*test_method)();

    // returns true in case
    bool run_test(test_method method) {
        try {
            method();
        } catch(const std::exception& e) {
            std::cout << "Test failed, unexpected exception catched: " << e.what() << std::endl;
            return true;
        }
        catch(...) {
            std::cout << "Test failed with unknown exception." << std::endl;
            return true;
        }
        return false;
    }
}


#define TEST(failed, method) \
    failed = _fnm_test_::run_test(method) || failed

int main() {
    bool failed = false;
    TEST(failed, fnm_tests::rtp_packet_tests );
    TEST(failed, fnm_tests::sender_tests );
    TEST(failed, fnm_tests::receiver_tests );
    return failed;
}
