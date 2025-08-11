#include "pch.h"

#include "Nikon_Except.h"



using namespace NEKCS;



static System::String^ MtpExPhaseToString(MtpExPhase phase) {
	switch (phase) {
		case MtpExPhase::COM_INIT: return L"COM Initialization";
		case MtpExPhase::MANAGER_INIT: return L"Manager Initialization";
		case MtpExPhase::MANAGER_DEVICELIST: return L"Manager Device List";
		case MtpExPhase::DEVICECLIENT_INIT: return L"Device Client Initialization";
		case MtpExPhase::DEVICE_INIT: return L"Device Initialization";
		case MtpExPhase::OPERATION_INIT: return L"Operation Initialization";
		case MtpExPhase::OPERATION_SEND: return L"Operation Send";
		case MtpExPhase::OPERATION_RESPONSE: return L"Operation Response";
		case MtpExPhase::OPERATIONREAD_INIT: return L"Operation Read Initialization";
		case MtpExPhase::OPERATIONREAD_SEND: return L"Operation Read Send";
		case MtpExPhase::OPERATIONREAD_RESPONSE: return L"Operation Read Response";
		case MtpExPhase::DATAREAD_INIT: return L"Data Read Initialization";
		case MtpExPhase::DATAREAD_SEND: return L"Data Read Send";
		case MtpExPhase::ENDREAD_INIT: return L"End Read Initialization";
		case MtpExPhase::ENDREAD_SEND: return L"End Read Send";
		case MtpExPhase::ENDREAD_RESPONSE: return L"End Read Response";
		case MtpExPhase::OPERATIONWRITE_INIT: return L"Operation Write Initialization";
		case MtpExPhase::OPERATIONWRITE_SEND: return L"Operation Write Send";
		case MtpExPhase::OPERATIONWRITE_RESPONSE: return L"Operation Write Response";
		case MtpExPhase::DATAWRITE_INIT: return L"Data Write Initialization";
		case MtpExPhase::DATAWRITE_SEND: return L"Data Write Send";
		case MtpExPhase::ENDWRITE_INIT: return L"End Write Initialization";
		case MtpExPhase::ENDWRITE_SEND: return L"End Write Send";
		case MtpExPhase::ENDWRITE_RESPONSE: return L"End Write Response";
		case MtpExPhase::DEVICE_NOT_CONNECTED: return L"Device Not Connected";
		default: return L"Unknown Phase";
	}
}

static System::String^ MtpExCodeToString(MtpExCode code) {
	switch (code) {
		case MtpExCode::NO_ERR: return L"No Error";
		case MtpExCode::UNKNOW_ERR: return L"Unknown Error";
		case MtpExCode::UNEXPECTED: return L"Unexpected Error";
		case MtpExCode::MEMORY: return L"Memory Error";
		case MtpExCode::ALREADY_INITIALIZED: return L"Already Initialized";
		case MtpExCode::INVALID_ARG: return L"Invalid Argument";
		case MtpExCode::INVALID_TYPE: return L"Invalid Type";
		case MtpExCode::MISSING: return L"Missing Parameter";
		case MtpExCode::DEVICE_DISCONNECTED: return L"Device Disconnected";
		default: return L"Unknown Code";
	}
}


MtpDeviceException::MtpDeviceException(MtpExPhase phase, MtpExCode code, System::String^ message) : System::Exception(message) {
	_phase = phase;
	_code = code;
}
MtpDeviceException::MtpDeviceException(MtpExPhase phase, MtpExCode code) :
	MtpDeviceException(phase, code, "MTP Device Exception - Phase: " + MtpExPhaseToString(phase) + ", Code: " + MtpExCodeToString(code)) {}
MtpDeviceException::MtpDeviceException(const nek::mtp::MtpDeviceException& e) :
	MtpDeviceException(static_cast<MtpExPhase>(e.phase), static_cast<MtpExCode>(e.code), gcnew System::String(e.what())) {}



MtpException::MtpException(NikonMtpOperationCode operationCode, NikonMtpResponseCode responseCode, System::String^ message) : System::Exception(message) {
	_operationCode = operationCode;
	_responseCode = responseCode;
}
MtpException::MtpException(NikonMtpOperationCode operationCode, NikonMtpResponseCode responseCode) :
	MtpException(operationCode, responseCode, "MTP Error - Operation: " + operationCode.ToString() + ", Response: " + responseCode.ToString()) { }
MtpException::MtpException(const nek::mtp::MtpException& e) :
	MtpException(static_cast<NikonMtpOperationCode>(e.operationCode), static_cast<NikonMtpResponseCode>(e.responseCode)) {}