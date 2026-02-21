#include "nek_mtp_except.hpp"

#include <sstream>



using namespace nek::mtp;



std::string MtpExPhaseToString(MtpExPhase phase) {
	switch (phase) {
	case COM_INIT: return "COM Initialization";
	case MANAGER_INIT: return "Manager Initialization";
	case MANAGER_DEVICELIST: return "Manager Device List";
	case DEVICECLIENT_INIT: return "Device Client Initialization";
	case DEVICE_INIT: return "Device Initialization";
	case OPERATION_INIT: return "Operation Initialization";
	case OPERATION_SEND: return "Operation Send";
	case OPERATION_RESPONSE: return "Operation Response";
	case OPERATIONREAD_INIT: return "Operation Read Initialization";
	case OPERATIONREAD_SEND: return "Operation Read Send";
	case OPERATIONREAD_RESPONSE: return "Operation Read Response";
	case DATAREAD_INIT: return "Data Read Initialization";
	case DATAREAD_SEND: return "Data Read Send";
	case ENDREAD_INIT: return "End Read Initialization";
	case ENDREAD_SEND: return "End Read Send";
	case ENDREAD_RESPONSE: return "End Read Response";
	case OPERATIONWRITE_INIT: return "Operation Write Initialization";
	case OPERATIONWRITE_SEND: return "Operation Write Send";
	case OPERATIONWRITE_RESPONSE: return "Operation Write Response";
	case DATAWRITE_INIT: return "Data Write Initialization";
	case DATAWRITE_SEND: return "Data Write Send";
	case ENDWRITE_INIT: return "End Write Initialization";
	case ENDWRITE_SEND: return "End Write Send";
	case ENDWRITE_RESPONSE: return "End Write Response";
	case DEVICE_NOT_CONNECTED: return "Device Not Connected";
	default: return "Unknown Phase";
	}
}

std::string MtpExCodeToString(MtpExCode code) {
	switch (code) {
	case NO_ERR: return "No Error";
	case UNKNOW_ERR: return "Unknown Error";
	case UNEXPECTED: return "Unexpected Error";
	case MEMORY: return "Memory Error";
	case ALREADY_INITIALIZED: return "Already Initialized";
	case INVALID_ARG: return "Invalid Argument";
	case INVALID_TYPE: return "Invalid Type";
	case MISSING: return "Missing Parameter";
	case DEVICE_DISCONNECTED: return "Device Disconnected";
	default: return "Unknown Code";
	}
}



//MtpDeviceException
MtpDeviceException::MtpDeviceException(MtpExPhase phase, HRESULT hr) : phase(phase), code(computeCode(phase, hr)) {};
MtpDeviceException::MtpDeviceException(MtpExPhase phase, MtpExCode code) : phase(phase), code(code) {};

MtpExCode MtpDeviceException::computeCode(MtpExPhase phase, HRESULT hr) {
	if (hr == S_OK) return NO_ERR;
	if (hr == E_OUTOFMEMORY) return MEMORY;
	if (hr == E_UNEXPECTED) return UNEXPECTED;
	if (hr == E_INVALIDARG) return INVALID_ARG;
	if (hr == E_POINTER) return INVALID_ARG;

	switch (phase) {
	case COM_INIT:
		switch (hr) {
		case S_FALSE:
		case RPC_E_CHANGED_MODE:
			return ALREADY_INITIALIZED;
		default:
			return UNKNOW_ERR;
		}

	case MANAGER_INIT:
		switch (hr) {
		case E_NOINTERFACE:
		case CLASS_E_NOAGGREGATION:
		case REGDB_E_CLASSNOTREG:
			return INVALID_ARG;
		default:
			return UNKNOW_ERR;
		}

	case DEVICE_INIT:
		switch (hr) {
		case E_WPD_DEVICE_ALREADY_OPENED:
			return ALREADY_INITIALIZED;
		default:
			return UNKNOW_ERR;
		}

	case OPERATION_RESPONSE:
	case OPERATIONREAD_RESPONSE:
	case OPERATIONWRITE_RESPONSE:
	case ENDREAD_RESPONSE:
	case ENDWRITE_RESPONSE:
		switch (hr) {
		case DISP_E_TYPEMISMATCH:
			return INVALID_TYPE;
		case HRESULT_FROM_WIN32(ERROR_NOT_FOUND):
			return MISSING;
		default:
			return UNKNOW_ERR;
		}

	case OPERATION_SEND:
	case OPERATIONREAD_SEND:
	case OPERATIONWRITE_SEND:
	case DATAWRITE_SEND:
	case DATAREAD_SEND:
	case ENDREAD_SEND:
	case ENDWRITE_SEND:
		switch (hr) {
		case UI_E_SHUTDOWN_CALLED:
			return DEVICE_DISCONNECTED;
		default:
			return UNKNOW_ERR;
		}

	default:
		return UNKNOW_ERR;
	}
}

const char* MtpDeviceException::what() const noexcept {
	std::ostringstream message;
	message << "MTP Device Exception - Phase: " << MtpExPhaseToString(phase) << ", Code: " << MtpExCodeToString(code);
	return message.str().c_str();
}



//MtpException
MtpException::MtpException(uint32_t operationCode, uint32_t responseCode) : operationCode(operationCode), responseCode(responseCode) {}

const char* MtpException::what() const noexcept {
	std::ostringstream message;
	message << "MTP Error - Operation: 0x" << std::hex << operationCode;
	message << ", Response: 0x" << std::hex << responseCode;
	return message.str().c_str();
}