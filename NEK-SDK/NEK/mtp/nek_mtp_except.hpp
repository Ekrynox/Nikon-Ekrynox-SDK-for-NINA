#pragma once
#include "../nek.hpp"

#include <stdlib.h>
#include <exception>
#include <string>

#include <atlbase.h>
#include <PortableDevice.h>



namespace nek::mtp {

	enum MtpExPhase {
		COM_INIT = 0,
		MANAGER_INIT = 1,
		MANAGER_DEVICELIST = 2,
		DEVICECLIENT_INIT = 3,
		DEVICE_INIT = 4,
		OPERATION_INIT = 5,
		OPERATION_SEND = 6,
		OPERATION_RESPONSE = 7,
		OPERATIONREAD_INIT = 8,
		OPERATIONREAD_SEND = 9,
		OPERATIONREAD_RESPONSE = 10,
		DATAREAD_INIT = 11,
		DATAREAD_SEND = 12,
		ENDREAD_INIT = 13,
		ENDREAD_SEND = 14,
		ENDREAD_RESPONSE = 15,
		OPERATIONWRITE_INIT = 16,
		OPERATIONWRITE_SEND = 17,
		OPERATIONWRITE_RESPONSE = 18,
		DATAWRITE_INIT = 19,
		DATAWRITE_SEND = 20,
		ENDWRITE_INIT = 21,
		ENDWRITE_SEND = 22,
		ENDWRITE_RESPONSE = 23,
		DEVICE_NOT_CONNECTED = 24,
	};

	enum MtpExCode {
		NO_ERR = 0,
		UNKNOW_ERR = 1,
		UNEXPECTED = 2,
		MEMORY = 3,
		ALREADY_INITIALIZED = 4,
		INVALID_ARG = 5,
		INVALID_TYPE = 6,
		MISSING = 7,
		DEVICE_DISCONNECTED = 8,
	};



	class MtpDeviceException : public std::exception {
	public:
		NEK_API MtpDeviceException(MtpExPhase phase, HRESULT hr);
		NEK_API const char* what() const noexcept override;

		const MtpExPhase phase;
		const MtpExCode code;

	private:
		static MtpExCode computeCode(MtpExPhase phase, HRESULT hr);
	};



	class MtpException : public std::exception {
	public:
		NEK_API MtpException(uint32_t operationCode, uint32_t responseCode);
		NEK_API const char* what() const noexcept override;

		const uint32_t operationCode;
		const uint32_t responseCode;
	};

}