#pragma once
#include "../nek.hpp"
#include "nek_mtp_enum.hpp"

#include <vector>



namespace nek::mtp {

	struct MtpResponse_ {
		uint16_t responseCode = MtpResponseCode::Undefined;
		std::vector<uint32_t> parameters;
		std::vector<uint8_t> data;
	};
	typedef struct MtpResponse_ MtpResponse;


	struct MtpEvent_ {
		uint16_t eventCode = MtpEventCode::Undefined;
		std::vector<uint32_t> parameters;
	};
	typedef struct MtpEvent_ MtpEvent;

}