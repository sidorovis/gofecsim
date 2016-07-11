#include "test_register.h"

#include "defines.h"

void fnm_tests::rtp_packet_tests() {
    fnm::rtp_packet<5u> packet;
    CHECK_EQUAL(5U, packet._header[5]);
    CHECK_EQUAL(5U, packet.get_length());

    packet.set_sequence_number(616u);
    CHECK_EQUAL(616U, packet.get_sequence_number());
}
