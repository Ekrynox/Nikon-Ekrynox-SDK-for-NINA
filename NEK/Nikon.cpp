#include "nikon.hpp"
#include "mtp/nek_mtp_except.hpp"

#include <chrono>
#include <future>
#include <time.h>
#include <stdexcept>



using namespace nek;



std::vector<std::pair<mtp::backend::MtpConnectionInfo, mtp::MtpDeviceInfoDS>> NikonCamera::listNikonCameras(bool onlyOn) {
	std::vector<std::pair<mtp::backend::MtpConnectionInfo, mtp::MtpDeviceInfoDS>> result;

	mtp::MtpManager manager;
	auto devices = manager.listAllDevices();

	for (auto& d : devices) {
		if (d.first.usbPath.has_value()) {
			std::wstring id = d.first.usbPath.value();
			std::transform(id.begin(), id.end(), id.begin(), ::towupper);
			if (id.find(L"VID_04B0") != std::wstring::npos) {
				auto cam = NikonCamera(std::move(d.second));
				auto info = cam.GetDeviceInfo();

				if (!onlyOn || std::find_if(info.OperationsSupported.begin(), info.OperationsSupported.end(), [](const uint32_t& x) { return x == NikonMtpOperationCode::InitiateCapture || x == NikonMtpOperationCode::InitiateCaptureRecInSdram || x == NikonMtpOperationCode::InitiateCaptureRecInMedia; }) != info.OperationsSupported.end()) {
					result.push_back(std::make_pair(std::move(d.first), std::move(info)));
				}
			}
		}
	}

	return result;
}

std::vector<std::pair<NikonCamera, mtp::MtpDeviceInfoDS>> NikonCamera::getNikonCameras(bool onlyOn) {
	std::vector<std::pair<NikonCamera, mtp::MtpDeviceInfoDS>> result;

	mtp::MtpManager manager;
	auto devices = manager.listAllDevices();

	for (auto& d : devices) {
		if (d.first.usbPath.has_value()) {
			std::wstring id = d.first.usbPath.value();
			std::transform(id.begin(), id.end(), id.begin(), ::towupper);
			if (id.find(L"VID_04B0") != std::wstring::npos) {
				auto cam = NikonCamera(std::move(d.second));
				auto info = cam.GetDeviceInfo();

				if (!onlyOn || std::find_if(info.OperationsSupported.begin(), info.OperationsSupported.end(), [](const uint32_t& x) { return x == NikonMtpOperationCode::InitiateCapture || x == NikonMtpOperationCode::InitiateCaptureRecInSdram || x == NikonMtpOperationCode::InitiateCaptureRecInMedia; }) != info.OperationsSupported.end()) {
					result.push_back(std::make_pair(NikonCamera(std::move(d.first), false), std::move(info)));
				}
			}
		}
	}

	return result;
}

size_t NikonCamera::countNikonCameras(bool onlyOn) {
	return listNikonCameras(onlyOn).size();
}


NikonCamera::NikonCamera(std::unique_ptr<mtp::backend::IMtpTransport> backend, bool autoConnect) : nek::mtp::MtpDevice::MtpDevice(std::move(backend), false) {
	if (autoConnect) Connect();
}
NikonCamera::NikonCamera(mtp::backend::MtpConnectionInfo connectionInfo, bool autoConnect) : nek::mtp::MtpDevice::MtpDevice(connectionInfo, false) {
	if (autoConnect) Connect();
}


void NikonCamera::Connect() {
	if (!backend_->isConnected()) backend_->connect();
	if (backendCallbackId_.has_value() || eventPolling.joinable()) return; //Event acquisition already initialized

	mtp::MtpDeviceInfoDS info = GetDeviceInfo();
	std::vector<mtp::MtpEvent> events;
	bool canGetEventEx = std::find(info.OperationsSupported.begin(), info.OperationsSupported.end(), NikonMtpOperationCode::GetEventEx) != info.OperationsSupported.end();
	if (!canGetEventEx) { //Enforce testing as some camera does not correctly repport their capabilities (D3200, ...)
		try {
			events = GetEventEx();
			canGetEventEx = true;
			for (const auto& e : events) OnEvent(e);
		}
		catch (...) {}
	}

	bool canGetEvent = std::find(info.OperationsSupported.begin(), info.OperationsSupported.end(), NikonMtpOperationCode::GetEvent) != info.OperationsSupported.end();
	if (canGetEventEx && !canGetEvent) { //Enforce testing as some camera does not correctly repport their capabilities (D3200, ...)
		try {
			events = GetEvent();
			canGetEvent = true;
			for (const auto& e : events) OnEvent(e);
		}
		catch (...) {}
	}

	if (canGetEventEx) {
		eventPolling = std::jthread([this](std::stop_token stop_token) {
			while (!stop_token.stop_requested() && this->isConnected()) {
				try {
					for (const auto& e : GetEventEx()) OnEvent(e);
				}
				catch (const mtp::MtpDeviceException& e) {
					if (e.code == nek::mtp::MtpExCode::DEVICE_DISCONNECTED) {
						this->Disconnect();
						return;
					}
				}
				catch (...) {}
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			});
	}
	else if (canGetEvent) {
		eventPolling = std::jthread([this](std::stop_token stop_token) {
			while (!stop_token.stop_requested() && this->isConnected()) {
				try {
					for (const auto& e : GetEvent()) OnEvent(e);
				}
				catch (const mtp::MtpDeviceException& e) {
					if (e.code == nek::mtp::MtpExCode::DEVICE_DISCONNECTED) {
						this->Disconnect();
						return;
					}
				}
				catch (...) {}
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			});
	}
	else backendCallbackId_ = backend_->subscribe([this](const mtp::MtpEvent& event) { return this->OnEvent(event); });
}

void NikonCamera::Disconnect() {
	if (backend_) {
		if (backendCallbackId_.has_value()) backend_->unsubscribe(backendCallbackId_.value());
		backendCallbackId_ = std::nullopt;
		backend_->disconnect();
		OnEvent(nek::mtp::MtpEvent(NikonMtpEventCode::DeviceInfoChanged)); // Notify Disconnection: DeviceInfoChanged
	}

	if (eventPolling.joinable()) {
		eventPolling.request_stop();
		if(std::this_thread::get_id() != eventPolling.get_id()) eventPolling.join();
	}
}



NikonDeviceInfoDS NikonCamera::GetDeviceInfo() {
	NikonDeviceInfoDS deviceInfo = mtp::MtpDevice::GetDeviceInfo();

	//Additional VendorCodes
	mtp::MtpResponse response = SendCommandAndRead(NikonMtpOperationCode::GetVendorPropCodes, {});
	if (response.responseCode == NikonMtpResponseCode::OK) {
		uint32_t len = *(uint32_t*)response.data.data();
		size_t offset = sizeof(uint32_t);
		uint32_t code;

		for (uint32_t i = 0; i < len; i++) {
			code = static_cast<uint32_t>(*(uint16_t*)(response.data.data() + offset));
			offset += sizeof(uint16_t);

			if (std::find(deviceInfo.DevicePropertiesSupported.begin(), deviceInfo.DevicePropertiesSupported.end(), code) == deviceInfo.DevicePropertiesSupported.end()) {
				deviceInfo.DevicePropertiesSupported.push_back(code);
			}
		}
	}

	//OpCode
	response = SendCommandAndRead(NikonMtpOperationCode::GetVendorCodes, { 0x09 });
	if (response.responseCode == NikonMtpResponseCode::OK) {

		uint32_t len = *(uint32_t*)response.data.data();
		size_t offset = sizeof(uint32_t);
		uint32_t code;

		for (uint32_t i = 0; i < len; i++) {
			code = *(uint32_t*)(response.data.data() + offset);
			offset += sizeof(uint32_t);

			if (std::find(deviceInfo.OperationsSupported.begin(), deviceInfo.OperationsSupported.end(), code) == deviceInfo.OperationsSupported.end()) {
				deviceInfo.OperationsSupported.push_back(code);
			}
		}
	}
	
	//PropCode
	response = SendCommandAndRead(NikonMtpOperationCode::GetVendorCodes, { 0x0D });
	if (response.responseCode == NikonMtpResponseCode::OK) {

		uint32_t len = *(uint32_t*)response.data.data();
		size_t offset = sizeof(uint32_t);
		uint32_t code;

		for (uint32_t i = 0; i < len; i++) {
			code = *(uint32_t*)(response.data.data() + offset);
			offset += sizeof(uint32_t);

			if (std::find(deviceInfo.DevicePropertiesSupported.begin(), deviceInfo.DevicePropertiesSupported.end(), code) == deviceInfo.DevicePropertiesSupported.end()) {
				deviceInfo.DevicePropertiesSupported.push_back(code);
			}
		}
	}


	mutexDeviceInfo_.lock();
	deviceInfo_ = deviceInfo;
	mutexDeviceInfo_.unlock();
	return deviceInfo;
}

std::vector<mtp::MtpEvent> NikonCamera::GetEvent() {
	nek::mtp::MtpResponse response = SendCommandAndRead(NikonMtpOperationCode::GetEvent, {});
	if (response.responseCode != NikonMtpResponseCode::OK) {
		throw mtp::MtpException(NikonMtpOperationCode::GetEvent, response.responseCode);
	}

	std::vector<mtp::MtpEvent> events;

	uint16_t eventCode;
	uint32_t eventParam;
	uint16_t count = *(uint16_t*)(response.data.data());
	for (uint16_t i = 0; i < count; i++) {
		eventCode = *(uint16_t*)(response.data.data() + 2 + i * 6);
		eventParam = *(uint32_t*)(response.data.data() + 4 + i * 6);
		events.push_back(nek::mtp::MtpEvent(eventCode, { eventParam }));
	}

	return events;
}
std::vector<mtp::MtpEvent> NikonCamera::GetEventEx() {
	nek::mtp::MtpResponse response = SendCommandAndRead(NikonMtpOperationCode::GetEventEx, {});
	if (response.responseCode != NikonMtpResponseCode::OK) {
		throw mtp::MtpException(NikonMtpOperationCode::GetEventEx, response.responseCode);
	}

	std::vector<mtp::MtpEvent> events;

	uint16_t count_params;
	uint16_t eventCode;
	std::vector<uint32_t> eventParams;

	uint32_t count = *(uint32_t*)(response.data.data());
	size_t offset = sizeof(uint32_t);
	for (uint32_t i = 0; i < count; i++) {
		eventCode = *(uint16_t*)(response.data.data() + offset);
		offset += sizeof(uint16_t);
		count_params = *(uint16_t*)(response.data.data() + offset);
		offset += sizeof(uint16_t);
		eventParams.clear();
		for (uint32_t j = 0; j < count_params; j++) {
			eventParams.push_back(*(uint32_t*)(response.data.data() + offset));
			offset += sizeof(uint32_t);
		}
		events.push_back(nek::mtp::MtpEvent(eventCode, eventParams));
	}

	return events;
}


mtp::MtpDevicePropDescDSV NikonCamera::GetDevicePropDesc(uint32_t devicePropCode) {
	mutexDeviceInfo_.lock();
	if (std::find(deviceInfo_.OperationsSupported.begin(), deviceInfo_.OperationsSupported.end(), NikonMtpOperationCode::GetDevicePropDescEx) != deviceInfo_.OperationsSupported.end()) {
		mutexDeviceInfo_.unlock();

		mtp::MtpResponse response = SendCommandAndRead(NikonMtpOperationCode::GetDevicePropDescEx, { devicePropCode });

		if (response.responseCode != NikonMtpResponseCode::OK) {
			throw mtp::MtpException(NikonMtpOperationCode::GetDevicePropDescEx, response.responseCode);
		}

		//PropCode is 4 bytes instead of 2 for the standard command
		uint32_t devicePropCodeReturned = *(uint32_t*)(response.data.data());
		response.data.erase(response.data.begin(), response.data.begin() + 2);

		mtp::MtpDevicePropDescDSV result = GetDevicePropDesc_(response);
		result.DevicePropertyCode = devicePropCodeReturned;

		mutexDeviceInfo_.lock();
		if (devicePropDataType_.find(result.DevicePropertyCode) == devicePropDataType_.end()) {
			devicePropDataType_.insert(std::pair<uint32_t, uint16_t>(result.DevicePropertyCode, result.DataType)); //Register Type for futur use in Get Value
		}
		mutexDeviceInfo_.unlock();
		return result;
	}
	mutexDeviceInfo_.unlock();

	return mtp::MtpDevice::GetDevicePropDesc(devicePropCode);
}
mtp::MtpDatatypeVariant NikonCamera::GetDevicePropValue(uint32_t devicePropCode) {
	mutexDeviceInfo_.lock();
	if (std::find(deviceInfo_.OperationsSupported.begin(), deviceInfo_.OperationsSupported.end(), NikonMtpOperationCode::GetDevicePropValueEx) != deviceInfo_.OperationsSupported.end()) {
		mutexDeviceInfo_.unlock();
		mutexDeviceInfo_.lock();
		if (devicePropDataType_.find(devicePropCode) == devicePropDataType_.end()) {
			mutexDeviceInfo_.unlock();
			return GetDevicePropDesc(devicePropCode).CurrentValue; //Get the current value and cache the Type
		}
		uint16_t dataType = devicePropDataType_[devicePropCode];
		mutexDeviceInfo_.unlock();

		mtp::MtpResponse response = SendCommandAndRead(NikonMtpOperationCode::GetDevicePropValueEx, { devicePropCode });

		if (response.responseCode != mtp::MtpResponseCode::OK) {
			throw mtp::MtpException(NikonMtpOperationCode::GetDevicePropValueEx, response.responseCode);
		}

		return GetDevicePropValue_(response, dataType);
	}
	mutexDeviceInfo_.unlock();

	return mtp::MtpDevice::GetDevicePropValue(devicePropCode);
}
void NikonCamera::SetDevicePropValue(uint32_t devicePropCode, mtp::MtpDatatypeVariant data) {
	mutexDeviceInfo_.lock();
	if (std::find(deviceInfo_.OperationsSupported.begin(), deviceInfo_.OperationsSupported.end(), NikonMtpOperationCode::SetDevicePropValueEx) != deviceInfo_.OperationsSupported.end()) {
		mutexDeviceInfo_.unlock();
		std::vector<uint8_t> rawdata = SetDevicePropValue_(data);

		mtp::MtpResponse response = SendCommandAndWrite(NikonMtpOperationCode::SetDevicePropValueEx, { devicePropCode }, rawdata);

		if (response.responseCode != mtp::MtpResponseCode::OK) {
			throw mtp::MtpException(NikonMtpOperationCode::SetDevicePropValueEx, response.responseCode);
		}

		return;
	}
	mutexDeviceInfo_.unlock();

	return mtp::MtpDevice::SetDevicePropValue(devicePropCode, data);
}
void NikonCamera::SetDevicePropValueTypesafe(uint32_t devicePropCode, mtp::MtpDatatypeVariant data) {
	uint16_t dataType = mtp::MtpDatatypeCode::Undefined;
	mutexDeviceInfo_.lock();
	if (devicePropDataType_.find(devicePropCode) == devicePropDataType_.end()) {
		mutexDeviceInfo_.unlock();
		dataType = GetDevicePropDesc(devicePropCode).DataType;
	}
	else {
		dataType = devicePropDataType_[devicePropCode];
		mutexDeviceInfo_.unlock();
	}

	mtp::MtpDatatypeVariant newdata;
	if (SetDevicePropValueTypesafe_(dataType, data, newdata)) return SetDevicePropValue(devicePropCode, newdata);

	mutexDeviceInfo_.lock();
	if (std::find(deviceInfo_.OperationsSupported.begin(), deviceInfo_.OperationsSupported.end(), NikonMtpOperationCode::SetDevicePropValueEx) != deviceInfo_.OperationsSupported.end()) {
		mutexDeviceInfo_.unlock();
		throw mtp::MtpException(NikonMtpOperationCode::SetDevicePropValueEx, NikonMtpResponseCode::Invalid_DeviceProp_Format);
	}
	mutexDeviceInfo_.unlock();
	throw mtp::MtpException(NikonMtpOperationCode::SetDevicePropValue, NikonMtpResponseCode::Invalid_DeviceProp_Format);
}


uint32_t NikonCamera::DeviceReady() {
	return SendCommandAndRead(NikonMtpOperationCode::DeviceReady, {}).responseCode;
}
uint32_t NikonCamera::DeviceReadyWhile(uint32_t whileResponseCode, std::stop_token stopToken, size_t sleepTimems) {
	uint32_t responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, {}).responseCode;
	while ((responseCode == whileResponseCode) && !stopToken.stop_requested()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimems));
		responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, {}).responseCode;
	}
	return responseCode;
}
uint32_t NikonCamera::DeviceReadyWhile(std::vector<uint32_t> whileResponseCodes, std::stop_token stopToken, size_t sleepTimems) {
	uint32_t responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, {}).responseCode;
	while ((std::find(whileResponseCodes.begin(), whileResponseCodes.end(), responseCode) != whileResponseCodes.end()) && !stopToken.stop_requested()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimems));
		responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, {}).responseCode;
	}
	return responseCode;
}
uint32_t NikonCamera::DeviceReadyWhileNot(uint32_t whileNotResponseCode, std::stop_token stopToken, size_t sleepTimems) {
	uint32_t responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, {}).responseCode;
	while ((responseCode != whileNotResponseCode) && !stopToken.stop_requested()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimems));
		responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, {}).responseCode;
	}
	return responseCode;
}
uint32_t NikonCamera::DeviceReadyWhileNot(std::vector<uint32_t> whileNotResponseCodes, std::stop_token stopToken, size_t sleepTimems) {
	uint32_t responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, {}).responseCode;
	while ((std::find(whileNotResponseCodes.begin(), whileNotResponseCodes.end(), responseCode) == whileNotResponseCodes.end()) && !stopToken.stop_requested()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimems));
		responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, {}).responseCode;
	}
	return responseCode;
}


uint32_t NikonCamera::StartLiveView(bool wait, std::stop_token stopToken, size_t sleepTimems) {
	auto lvstate = GetDevicePropValue(NikonMtpDevicePropCode::RemoteLiveViewStatus);
	if (std::get<uint8_t>(lvstate) == 1) {
		return NikonMtpResponseCode::OK; //Liveview already On
	}

	mtp::MtpResponse response = SendCommand(NikonMtpOperationCode::StartLiveView, {});
	if (response.responseCode != NikonMtpResponseCode::OK) {
		throw mtp::MtpException(NikonMtpOperationCode::StartLiveView, response.responseCode);
	}

	if (!wait) return response.responseCode;
	return DeviceReadyWhile(NikonMtpResponseCode::Device_Busy, stopToken, sleepTimems);
}
void NikonCamera::EndLiveView() {
	auto lvstate = GetDevicePropValue(NikonMtpDevicePropCode::RemoteLiveViewStatus);
	if (std::get<uint8_t>(lvstate) != 1) {
		return; //Liveview is not On
	}

	mtp::MtpResponse response = SendCommand(NikonMtpOperationCode::EndLiveView, {});
	if (response.responseCode != NikonMtpResponseCode::OK) {
		throw mtp::MtpException(NikonMtpOperationCode::EndLiveView, response.responseCode);
	}
}