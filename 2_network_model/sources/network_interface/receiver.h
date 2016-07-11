#ifndef _FNM_NETWORK_INTERFACE_RECEIVER_H_
#define _FNM_NETWORK_INTERFACE_RECEIVER_H_

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include <queue>

#include <boost/asio.hpp>

namespace fnm {

    template<uint32_t packet_size>
    class rtp_packet;
    
    static const uint32_t packet_size = 2048u;

    class receiver {
    public:
	explicit receiver(const std::string& port);
	~receiver();

	void start_receive_video_stream();
	void receive_video_stream();
	void on_video_packet_receive();
	
	void stop_receiving();
	void wait();

	uint32_t nack_requests() const;
	uint32_t received_packets() const;
	uint32_t lost_packets() const;
    private:
	volatile bool _running;

	std::thread _receive_video_thread;
	std::thread _process_video_thread;

	std::mutex _receive_video_mutex;
	volatile bool _video_receiving;
	std::condition_variable _receive_video_started;

	std::atomic_uint_fast32_t _nack_requests;
	std::atomic_uint_fast32_t _received_packets;
	std::atomic_uint_fast32_t _lost_packets;
	
	boost::asio::io_service _io_service;
	boost::asio::ip::udp::socket _local_socket;
	
	std::mutex _free_rtp_buffers_mutex;
	std::queue<std::shared_ptr<rtp_packet<packet_size>>> _free_rtp_buffers;
	
	std::mutex _rtp_buffers_in_use_mutex;
	std::condition_variable _rtp_buffers_in_use_changed;
	std::queue<std::shared_ptr<rtp_packet<packet_size>>> _rtp_buffers_in_use;
	
	void handle_receive(std::shared_ptr<rtp_packet<packet_size>> packet, const boost::system::error_code& error, std::size_t bytes_transferred);
	void process_messages();
	void process_message(const rtp_packet<packet_size>& packet);
    };

}

#endif // _FNM_NETWORK_INTERFACE_SENDER_H_
