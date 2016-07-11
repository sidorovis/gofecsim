#include "receiver.h"

#include <boost/array.hpp>
#include <boost/bind.hpp>

#include "defines.h"

#include <iostream>

namespace {
    unsigned short parse_port(const std::string& port) {
	unsigned short res = 0;
	sscanf(port.c_str(), "%hu", &res);
	return res;
    }
}


fnm::receiver::receiver(const std::string& port)
    : _video_receiving(false)
    , _nack_requests(0)
    , _received_packets(0)
    , _lost_packets(0)
    , _local_socket(_io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), parse_port(port)))
{
    for (size_t i = 0 ; i < 100000 ; ++i) {
        _free_rtp_buffers.push(std::shared_ptr<rtp_packet<packet_size>>(new rtp_packet<packet_size>()));
    }
}

fnm::receiver::~receiver() {
}

void fnm::receiver::start_receive_video_stream() {
    std::unique_lock<std::mutex> protect(_receive_video_mutex);
    _running = true;
    _process_video_thread = std::thread(&receiver::process_messages, this);
    _receive_video_thread = std::thread(&receiver::receive_video_stream, this);
    while (!_video_receiving) {
        _receive_video_started.wait(protect);
    }
}

void fnm::receiver::receive_video_stream() {
    using boost::asio::ip::udp;
    udp::endpoint remote_endpoint;
    {
	std::unique_lock<std::mutex> protect(_receive_video_mutex);
	_video_receiving = true;
    }
    _receive_video_started.notify_one();
    {
	    std::shared_ptr<rtp_packet<packet_size>> packet = _free_rtp_buffers.front();
	    _free_rtp_buffers.pop();
	    _local_socket.async_receive(boost::asio::buffer(packet.get(), packet_size), 
		boost::bind(&receiver::handle_receive, this, packet, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
    _io_service.run();
}

void fnm::receiver::stop_receiving() {
    _running = false;
    _local_socket.close();
    _io_service.stop();
    _rtp_buffers_in_use_changed.notify_one();
}

void fnm::receiver::wait() {
    _receive_video_thread.join();
    _process_video_thread.join();
}

uint32_t fnm::receiver::nack_requests() const {
    return _nack_requests.load();
}

uint32_t fnm::receiver::received_packets() const {
    return _received_packets.load();
}

uint32_t fnm::receiver::lost_packets() const {
    return _lost_packets.load();
}

void fnm::receiver::handle_receive(std::shared_ptr<rtp_packet<packet_size>> packet, const boost::system::error_code& error, std::size_t ) {
    if (!error || error == boost::asio::error::message_size) 
    {
	{
		std::unique_lock<std::mutex> lock(_rtp_buffers_in_use_mutex);
		_rtp_buffers_in_use.push(packet);
	}
	_rtp_buffers_in_use_changed.notify_one();
	if (_running) {
		std::shared_ptr<rtp_packet<packet_size>> next_packet;
		{
			std::unique_lock<std::mutex> lock(_free_rtp_buffers_mutex);
			if (_free_rtp_buffers.empty()) {
				next_packet = std::shared_ptr<rtp_packet<packet_size>>(new rtp_packet<packet_size>());
			} else {
				next_packet = _free_rtp_buffers.front();
				_free_rtp_buffers.pop();
			}
		}
		_local_socket.async_receive(boost::asio::buffer(next_packet.get(), packet_size),
			boost::bind(&receiver::handle_receive, this, next_packet, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
    }
}

void fnm::receiver::process_messages() {
    while (_running) 
    {
	std::shared_ptr<rtp_packet<packet_size>> packet;
	{
		std::unique_lock<std::mutex> lock(_rtp_buffers_in_use_mutex);
		while (_running && _rtp_buffers_in_use.empty()) {
			_rtp_buffers_in_use_changed.wait(lock);
		}
		if (!_running) {
			break;
		}
		packet = _rtp_buffers_in_use.front();
		_rtp_buffers_in_use.pop();
	}
	process_message(*packet);
	{
		std::unique_lock<std::mutex> lock(_free_rtp_buffers_mutex);
		_free_rtp_buffers.push(packet);
	}
    }
}

void fnm::receiver::process_message(const rtp_packet<packet_size>& packet) {
	static std::atomic_uint_fast32_t last_max_sequence_number(0);
	if (packet.get_sequence_number() > last_max_sequence_number) {
		std::cout << " 1  " << packet.get_sequence_number() << " max: " << last_max_sequence_number << std::endl;
		_lost_packets.fetch_add(packet.get_sequence_number() - last_max_sequence_number);
		last_max_sequence_number.store(packet.get_sequence_number() + 1);
		std::cout << " 1+ " << packet.get_sequence_number() << " max: " << last_max_sequence_number << "    lost packets: " << _lost_packets << std::endl;
	} else if (packet.get_sequence_number() < last_max_sequence_number) {
		std::cout << " 2  " << packet.get_sequence_number() << " max: " << last_max_sequence_number << std::endl;
		_lost_packets--;
	} else {
		std::cout << " 3  " << packet.get_sequence_number() << " max: " << last_max_sequence_number << std::endl;
		_received_packets++;
		last_max_sequence_number++;
	}
}

