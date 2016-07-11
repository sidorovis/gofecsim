#include "test_register.h"

#include "sender.h"
#include "receiver.h"

#include <iostream>

void fnm_tests::receiver_tests() {
    fnm::receiver r("34000");
    r.start_receive_video_stream();

    fnm::sender s("127.0.0.1", "34000");

    uint32_t packets_count = 1000000;

    s.start_send_video_stream(packets_count);
    s.wait();

    std::this_thread::sleep_for(std::chrono::milliseconds(5500));
    r.stop_receiving();
    r.wait();

//    std::cout << r.received_packets() << std::endl;
//    std::cout << r.lost_packets() << std::endl;
//    CHECK_EQUAL(packets_count, r.received_packets() + r.lost_packets());
}
