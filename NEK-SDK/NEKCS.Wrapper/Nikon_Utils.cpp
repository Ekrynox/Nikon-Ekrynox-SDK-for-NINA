#include "pch.h"

#include "Nikon_Utils.h"

#include <assert.h>


using namespace NEKCS;



//MtpReponseParams
MtpReponseParams::MtpReponseParams() { m_nativeClass = new nek::mtp::MtpReponseParams(); }
MtpReponseParams::MtpReponseParams(nek::mtp::MtpReponseParams responseParams) : MtpReponseParams() { *m_nativeClass = responseParams; }
MtpReponseParams::~MtpReponseParams() { this->!MtpReponseParams(); }
MtpReponseParams::!MtpReponseParams() { 
	if (m_nativeClass != nullptr) {
		delete m_nativeClass;
		m_nativeClass = nullptr;
	}
}



//MtpParams
MtpParams::MtpParams() { m_nativeClass = new nek::mtp::MtpParams(); }
MtpParams::MtpParams(nek::mtp::MtpParams params) : MtpParams() { *m_nativeClass = params; }
MtpParams::~MtpParams() { this->!MtpParams(); }
MtpParams::!MtpParams() {
	if (m_nativeClass != nullptr) {
		delete m_nativeClass;
		m_nativeClass = nullptr;
	}
}

void MtpParams::addUint32(System::UInt32 param) { m_nativeClass->addUint32(param); }
void MtpParams::addUint16(System::UInt16 param) { m_nativeClass->addUint16(param); }
void MtpParams::addInt32(System::Int32 param) { m_nativeClass->addInt32(param); }
void MtpParams::addInt16(System::Int16  param) { m_nativeClass->addInt16(param); }



//MtpResponse
MtpResponse::MtpResponse() {
	responseCode = (NikonMtpResponseCode)0;
	responseParams_ = gcnew MtpReponseParams();
	data = gcnew array<System::Byte>(0);
}
MtpResponse::MtpResponse(nek::mtp::MtpResponse response) {
	assert(response.data.size() <= static_cast<size_t>(INT_MAX));
	responseCode = (NikonMtpResponseCode)response.responseCode;
	responseParams_ = gcnew MtpReponseParams(response.GetParams());
	data = gcnew array<System::Byte>(static_cast<int>(response.data.size()));
	if (data->Length > 0) {
		pin_ptr<System::Byte> dataptr = &data[0];
		std::memcpy(dataptr, response.data.data(), response.data.size());
	}
}
MtpResponse::~MtpResponse() { this->!MtpResponse(); }
MtpResponse::!MtpResponse() { 
	if (responseParams_ != nullptr) {
		delete responseParams_;
		responseParams_ = nullptr;
	}
}

MtpReponseParams^ MtpResponse::GetParams() { return responseParams_; }



//MtpEvent
MtpEvent::MtpEvent(nek::mtp::MtpEvent event) {
	assert(event.eventParams.size() <= static_cast<size_t>(INT_MAX));
	eventCode = (NikonMtpEventCode)event.eventCode;
	eventParams = gcnew array<System::UInt32>(static_cast<int>(event.eventParams.size()));
	if (eventParams->Length > 0) {
		pin_ptr<System::UInt32> dataptr = &eventParams[0];
		std::memcpy(dataptr, event.eventParams.data(), event.eventParams.size() * sizeof(uint32_t));
	}
}