#pragma once

#include "mtp/nek_mtp_except.hpp"
#include "Nikon_Enum.h"



namespace NEKCS {
	public enum class MtpExPhase : System::UInt16 {
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

	public enum class MtpExCode : System::UInt16 {
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


	public ref class MtpDeviceException : public System::Exception {
	public:
		MtpDeviceException(MtpExPhase phase, MtpExCode code, System::String^ message);
		MtpDeviceException(MtpExPhase phase, MtpExCode code);
		MtpDeviceException(const nek::mtp::MtpDeviceException &e);

		property MtpExPhase Phase {
			MtpExPhase get() { return _phase; }
		}

		property MtpExCode Code {
			MtpExCode get() { return _code; }
		}

	private:
		MtpExPhase _phase;
		MtpExCode _code;
	};


	public ref class MtpException : public System::Exception {
	public:
		MtpException(NikonMtpOperationCode operationCode, NikonMtpResponseCode responseCode, System::String^ message);
		MtpException(NikonMtpOperationCode operationCode, NikonMtpResponseCode responseCode);
		MtpException(const nek::mtp::MtpException& e);

		property NikonMtpOperationCode OperationCode {
			NikonMtpOperationCode get() { return _operationCode; }
		}

		property NikonMtpResponseCode ResponseCode {
			NikonMtpResponseCode get() { return _responseCode; }
		}

	private:
		NikonMtpOperationCode _operationCode;
		NikonMtpResponseCode _responseCode;
	};
}