#include "pch.h"

#include "Nikon_Utils.h"

#include <assert.h>


using namespace NEKCS;



//MtpResponse
MtpResponse::MtpResponse() {
	_responseCode = (NikonMtpResponseCode)0;
	_parameters = gcnew array<System::UInt32>(0);
	_data = gcnew array<System::Byte>(0);
}
MtpResponse::MtpResponse(nek::mtp::MtpResponse response) {
	assert(response.parameters.size() <= static_cast<size_t>(INT_MAX));
	assert(response.data.size() <= static_cast<size_t>(INT_MAX));
	_responseCode = (NikonMtpResponseCode)response.responseCode;

	_parameters = gcnew array<System::UInt32>(static_cast<int>(response.parameters.size()));
	if (_parameters->Length > 0) {
		pin_ptr<System::UInt32> dataptr = &_parameters[0];
		std::memcpy(dataptr, response.parameters.data(), response.parameters.size() * sizeof(uint32_t));
	}

	_data = gcnew array<System::Byte>(static_cast<int>(response.data.size()));
	if (_data->Length > 0) {
		pin_ptr<System::Byte> dataptr = &_data[0];
		std::memcpy(dataptr, response.data.data(), response.data.size() * sizeof(uint8_t));
	}
}



//MtpEvent
MtpEvent::MtpEvent(nek::mtp::MtpEvent event) {
	assert(event.parameters.size() <= static_cast<size_t>(INT_MAX));
	_eventCode = (NikonMtpEventCode)event.eventCode;

	_parameters = gcnew array<System::UInt32>(static_cast<int>(event.parameters.size()));
	if (_parameters->Length > 0) {
		pin_ptr<System::UInt32> dataptr = &_parameters[0];
		std::memcpy(dataptr, event.parameters.data(), event.parameters.size() * sizeof(uint32_t));
	}
}