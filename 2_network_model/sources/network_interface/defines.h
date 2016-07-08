#ifndef _FNM_NETWORK_INTERFACE_DEFINES_H_
#define _FNM_NETWORK_INTERFACE_DEFINES_H_

#include <cstdint>

namespace fnm {

    struct rtp_packet {
	uint32_t _header[6];
	uint8_t *_data;
	
	explicit rtp_packet(uint32_t length) {
		_header[5] = length;
		_data = new uint8_t[length];
	}
	~rtp_packet() {
		delete [] _data;
	}
    };

}

#endif // _FNM_NETWORK_INTERFACE_DEFINES_H_
