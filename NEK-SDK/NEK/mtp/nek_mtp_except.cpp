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
MtpDeviceException::MtpDeviceException(MtpExPhase phase, MtpExCode code) : phase(phase), code(code) {};

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