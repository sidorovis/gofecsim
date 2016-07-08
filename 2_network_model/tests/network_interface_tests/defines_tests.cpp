#include "test_register.h"

#include "defines.h"

void fnm_tests::rtp_packet_tests() {
    fnm::rtp_packet packet(5);
    CHECK_EQUAL(5U, packet._header[5]);
}
