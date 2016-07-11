#include "test_register.h"

#include "sender.h"

void fnm_tests::sender_tests() {

    fnm::sender s("127.0.0.1", "34000");
    CHECK_EQUAL(0u, s.nack_requests());
    CHECK_EQUAL(0u, s.sent_packets());

    s.start_send_video_stream(1);
    s.wait();
    CHECK_EQUAL(1u, s.sent_packets());
}
