#include "sender.h"

#include <boost/asio.hpp>

#include "defines.h"

fnm::sender::sender(const std::string& host, const std::string& port) 
    : _host( host )
    , _port( port )
    , _video_sending(false)
    , _nack_requests(0)
    , _packets_to_send(0)
    , _sent_packets(0)
{
}

fnm::sender::~sender() {
}

void fnm::sender::start_send_video_stream(const uint32_t packets_to_send) {
    std::unique_lock<std::mutex> protect(_send_video_mutex);
    _packets_to_send = packets_to_send;
    _send_video_thread = std::thread(&sender::send_video_stream, this);
    while (!_video_sending) {
	_send_video_started.wait(protect);
    }
}

void fnm::sender::send_video_stream() {
    using boost::asio::ip::udp;

    boost::asio::io_service _io_service;

    udp::resolver resolver(_io_service);
    udp::resolver::query query(udp::v4(), _host, _port);
    udp::endpoint remote_endpoint = *resolver.resolve(query);

    udp::socket local_socket(_io_service);
    local_socket.open(udp::v4());

    {
	std::unique_lock<std::mutex> protect(_send_video_mutex);
	_video_sending = true;
    }
    _send_video_started.notify_one();

    try {
	for (uint32_t i = 0 ; i < _packets_to_send ; ++i ) {
		rtp_packet<1024u> packet;
		packet.set_sequence_number(i);
		strcpy((char*)packet._data, "rtp message on udp");
		local_socket.send_to(boost::asio::buffer(&packet, sizeof(packet._header) + 1024U), remote_endpoint);
		_sent_packets++;
	}
    } catch (...) {
    }

    local_socket.close();
}

void fnm::sender::wait() {
    _send_video_thread.join();
}

uint32_t fnm::sender::nack_requests() const {
    return _nack_requests.load();
}

uint32_t fnm::sender::sent_packets() const {
    return _sent_packets.load();
}

