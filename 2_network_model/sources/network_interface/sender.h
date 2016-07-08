#ifndef _FNM_NETWORK_INTERFACE_SENDER_H_
#define _FNM_NETWORK_INTERFACE_SENDER_H_

#include <atomic>
#include <mutex>
#include <condition_variable>

namespace fnm {

    class sender {
    public:
	explicit sender(const std::string& host, const std::string& port);
	~sender();
	
	void send_video_stream(const uint32_t packets_to_send);
	void wait();
	
	uint32_t nack_requests() const;
	uint32_t sent_packets() const;
    private:
	const std::string _host, _port;

	std::mutex _send_video_mutex;
	std::condition_variable _send_video_finished;
	
	std::atomic_uint_fast32_t _nack_requests;
	uint32_t _packets_to_send;
	std::atomic_uint_fast32_t _sent_packets;
    };

}

#endif // _FNM_NETWORK_INTERFACE_SENDER_H_
