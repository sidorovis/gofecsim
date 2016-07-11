#ifndef _FNM_NETWORK_INTERFACE_DEFINES_H_
#define _FNM_NETWORK_INTERFACE_DEFINES_H_

#include <cstdint>

namespace fnm {

    template<uint32_t data_size>
    class rtp_packet {
    public:
	uint32_t _header[6];
	uint8_t _data[ data_size ];
	
	explicit rtp_packet() {
		_header[5] = data_size;
	}
	~rtp_packet() {
	}
	
	void set_length(const uint32_t len) {
	    _header[5] = len;
	}

	void set_sequence_number(const uint32_t sn) {
	    _header[1] = sn;
	}
	
	uint32_t get_length() const {
	    return _header[5];
	}

	uint32_t get_sequence_number() const {
	    return _header[1];
	}
    };

}

#endif // _FNM_NETWORK_INTERFACE_DEFINES_H_
