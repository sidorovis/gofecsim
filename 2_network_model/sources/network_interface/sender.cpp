#include "sender.h"

#include "defines.h"

#include <boost/bind.hpp>

fnm::sender::sender(const std::string& host, const std::string& port) 
    : _host( host )
    , _port( port )
    , _nack_requests(0)
    , _packets_to_send(0)
    , _sent_packets(0)
{
}

fnm::sender::~sender() {
}

void fnm::sender::send_video_stream(const uint32_t packets_to_send) {
    using boost::asio::ip::udp;

    udp::resolver resolver(_io_service);
    udp::resolver::query query(udp::v4(), _host, _port);
    udp::endpoint remote_endpoint = *resolver.resolve(query);

    udp::socket local_socket(_io_service);
    local_socket.open(udp::v4());

    try {
	std::unique_lock<std::mutex> protect(_send_video_mutex);
	_packets_to_send = packets_to_send;
	
	for (uint32_t i = 0 ; i < _packets_to_send ; ++i ) {
		rtp_packet packet(1024U);
		local_socket.send_to(boost::asio::buffer(packet._data, 1024U), remote_endpoint);
		_sent_packets++;
	}
    } catch (...) {
    }
    
    _send_video_finished.notify_one();

    local_socket.close();
}

void fnm::sender::wait() {
    std::unique_lock<std::mutex> lock(_send_video_mutex);
    while (sent_packets() < _packets_to_send) {
	_send_video_finished.wait(lock);
    }
}

uint32_t fnm::sender::nack_requests() const {
    return _nack_requests.load();
}

uint32_t fnm::sender::sent_packets() const {
    return _sent_packets.load();
}

