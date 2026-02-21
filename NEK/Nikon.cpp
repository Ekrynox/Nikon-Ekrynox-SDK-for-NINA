#include "nikon.hpp"
#include "mtp/nek_mtp_except.hpp"

#include <chrono>
#include <time.h>
#include <stdexcept>



using namespace nek;



std::map<std::wstring, mtp::MtpDeviceInfoDS> NikonCamera::listNikonCameras(bool onlyOn) {
	mtp::MtpManager* deviceManager = &nek::mtp::MtpManager::Instance();
	auto cameras = deviceManager->listMtpDevices();
	std::map<std::wstring, mtp::MtpDeviceInfoDS> nikonCameras;

	for (auto &camera : cameras) {
		//Check if Nikon
		std::wstring id(camera.first);
		std::transform(id.begin(), id.end(), id.begin(), ::towlower);
		if (id.find(L"vid_04b0") != std::wstring::npos) {
			if (onlyOn == false) {
				nikonCameras.insert(camera);
			}
			else if (std::find(camera.second.OperationsSupported.begin(), camera.second.OperationsSupported.end(), NikonMtpOperationCode::InitiateCaptureRecInSdram) != camera.second.OperationsSupported.end()) {
				nikonCameras.insert(camera);
			}
		}
	}

	return nikonCameras;
}

size_t NikonCamera::countNikonCameras(bool onlyOn) {
	return listNikonCameras(onlyOn).size();
}


NikonCamera::NikonCamera(std::wstring devicePath, uint8_t additionalThread) : nek::mtp::MtpDevice::MtpDevice() {
	devicePath_ = devicePath;

	startThreads();
}


void NikonCamera::mainThreadTask() {
	mutexTasks_.lock();
	mutexDevice_.lock();

	//Init & Connect
	try {
		initCom();
		initDevice();
	}
	catch (...) {
		mutexDevice_.unlock();
		mutexTasks_.unlock();
		throw;
	}

	mutexDevice_.unlock();
	mutexTasks_.unlock();
	GetDeviceInfo();
	cvTasks_.notify_all();

	//Thread Loop
	threadTask();

	//Uninit
	mutexTasks_.lock();
	mutexDevice_.lock();

	if (eventCookie_ != nullptr) {
		device_->Unadvise(eventCookie_);
	}
	device_.Release();
	device_.p = nullptr;
	CoUninitialize();

	mutexDevice_.unlock();
	mutexTasks_.unlock();
	cvTasks_.notify_all();
}

void NikonCamera::eventThreadTask() {
	initCom();

	//Event Handler Detection
	mtp::MtpDeviceInfoDS info = GetDeviceInfo();
	if (std::find(info.OperationsSupported.begin(), info.OperationsSupported.end(), NikonMtpOperationCode::GetEventEx) != info.OperationsSupported.end()) { //GetEventEX
		cvTasks_.notify_all();

		//Event Loop
		nek::mtp::MtpParams params = nek::mtp::MtpParams();
		size_t offset;
		uint32_t count;
		uint16_t count_params;
		uint16_t eventCode;
		std::vector<uint32_t> eventParams;
		while (running_) {
			mutexDevice_.lock();
			nek::mtp::MtpResponse result;
			try {
				result = SendCommandAndRead_(device_, NikonMtpOperationCode::GetEventEx, params);
				mutexDevice_.unlock();
			}
			catch (const nek::mtp::MtpDeviceException& e) {
				mutexDevice_.unlock();
				if (e.code == nek::mtp::MtpExCode::DEVICE_DISCONNECTED) {
					Disconnect();
					CoUninitialize();
					return;
				}
				result.responseCode = NikonMtpResponseCode::General_Error;
			}

			if (result.responseCode == NikonMtpResponseCode::OK) {
				count = *(uint32_t*)(result.data.data());
				offset = sizeof(uint32_t);
				for (uint32_t i = 0; i < count; i++) {
					eventCode = *(uint16_t*)(result.data.data() + offset);
					offset += sizeof(uint16_t);
					count_params = *(uint16_t*)(result.data.data() + offset);
					offset += sizeof(uint16_t);
					eventParams.clear();
					for (uint32_t j = 0; j < count_params; j++) {
						eventParams.push_back(*(uint32_t*)(result.data.data() + offset));
						offset += sizeof(uint32_t);
					}
					mutexTasks_.lock();
					tasksEvent_.push_back([this, eventCode, eventParams] { eventCallback_->OnEvent(nek::mtp::MtpEvent(eventCode, eventParams)); });
					mutexTasks_.unlock();
					cvTasks_.notify_one();
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	else if (std::find(info.OperationsSupported.begin(), info.OperationsSupported.end(), NikonMtpOperationCode::GetEvent) != info.OperationsSupported.end()) { //GetEvent
		cvTasks_.notify_all();

		//Event Loop
		nek::mtp::MtpParams params = nek::mtp::MtpParams();
		uint16_t count;
		uint16_t eventCode;
		uint32_t eventParam;
		while (running_) {
			nek::mtp::MtpResponse result;
			mutexDevice_.lock();
			try {
				result = SendCommandAndRead_(device_, NikonMtpOperationCode::GetEvent, params);
				mutexDevice_.unlock();
			}
			catch (const nek::mtp::MtpDeviceException& e) {
				mutexDevice_.unlock();
				if (e.code == nek::mtp::MtpExCode::DEVICE_DISCONNECTED) {
					Disconnect();
					CoUninitialize();
					return;
				}
				result.responseCode = NikonMtpResponseCode::General_Error;
			}

			if (result.responseCode == NikonMtpResponseCode::OK) {
				count = *(uint16_t*)(result.data.data());
				for (uint16_t i = 0; i < count; i++) {
					eventCode = *(uint16_t*)(result.data.data() + 2 + i * 6);
					eventParam = *(uint32_t*)(result.data.data() + 4 + i * 6);
					mutexTasks_.lock();
					tasksEvent_.push_back([this, eventCode, eventParam] { eventCallback_->OnEvent(nek::mtp::MtpEvent(eventCode, eventParam)); });
					mutexTasks_.unlock();
					cvTasks_.notify_one();
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	else { //Default Mtp event system (Incomplete: missing event code, ...)
		mutexDevice_.lock();

		device_->Advise(0, this->eventCallback_, nullptr, &this->eventCookie_);

		mutexDevice_.unlock();
		cvTasks_.notify_all();
	}

	CoUninitialize();
}

void NikonCamera::threadTask() {
	while (running_) {
		mutexTasks_.lock();
		if (tasksEvent_.size() > 0) {
			std::function<void ()> task = tasksEvent_.front();
			tasksEvent_.pop_front();
			mutexTasks_.unlock();

			task();
		}
		else if (tasks_.size() > 0) {
			std::function<void ()> task = tasks_.front();
			tasks_.pop_front();
			mutexTasks_.unlock();

			task();
		}
		else {
			mutexTasks_.unlock();

			std::unique_lock lk(mutexTasks_);
			cvTasks_.wait(lk, [this] { return !this->running_ || (this->tasks_.size() + tasksEvent_.size() > 0); });
			lk.unlock();
		}
	}
}


void NikonCamera::startThreads() {
	running_ = true;
	//Start main thread
	mutexThreads_.lock();
	threads_.push_back(std::thread([this] { this->mainThreadTask(); }));
	mutexThreads_.unlock();
	std::unique_lock lk(mutexTasks_);
	cvTasks_.wait(lk);

	//Event Thread
	mutexThreads_.lock();
	threads_.push_back(std::thread([this] { this->eventThreadTask(); }));
	mutexThreads_.unlock();
	cvTasks_.wait(lk);

	//Additional Threads
	for (uint8_t i = 0; i < additionalThreadsNb_; i++) {
		mutexThreads_.lock();
		threads_.push_back(std::thread([this] { this->additionalThreadsTask(); }));
		mutexThreads_.unlock();
	}
}



mtp::MtpDeviceInfoDS NikonCamera::GetDeviceInfo() {
	auto deviceInfo = mtp::MtpDevice::GetDeviceInfo();

	//Additional VendorCodes
	if (std::find(deviceInfo.OperationsSupported.begin(), deviceInfo.OperationsSupported.end(), NikonMtpOperationCode::GetVendorPropCodes) != deviceInfo.OperationsSupported.end()) {
		mtp::MtpParams params = mtp::MtpParams();
		mtp::MtpResponse response = SendCommandAndRead(NikonMtpOperationCode::GetVendorPropCodes, params);

		if (response.responseCode != NikonMtpResponseCode::OK) {
			throw mtp::MtpException(NikonMtpOperationCode::GetVendorPropCodes, response.responseCode);
		}

		uint32_t len = *(uint32_t*)response.data.data();
		size_t offset = sizeof(uint32_t);
		uint16_t code;

		for (uint32_t i = 0; i < len; i++) {
			code = *(uint16_t*)(response.data.data() + offset);
			offset += sizeof(uint16_t);

			if (std::find(deviceInfo.DevicePropertiesSupported.begin(), deviceInfo.DevicePropertiesSupported.end(), code) == deviceInfo.DevicePropertiesSupported.end()) {
				deviceInfo.DevicePropertiesSupported.push_back(code);
			}
		}
	}
	if (std::find(deviceInfo.OperationsSupported.begin(), deviceInfo.OperationsSupported.end(), NikonMtpOperationCode::GetVendorCodes) != deviceInfo.OperationsSupported.end()) {
		//OpCode
		mtp::MtpParams params = mtp::MtpParams();
		params.addUint32(0x09);
		mtp::MtpResponse response = SendCommandAndRead(NikonMtpOperationCode::GetVendorCodes, params);

		if (response.responseCode != NikonMtpResponseCode::OK) {
			throw mtp::MtpException(NikonMtpOperationCode::GetVendorCodes, response.responseCode);
		}

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
		
		//PropCode
		params = mtp::MtpParams();
		params.addUint32(0x0D);
		response = SendCommandAndRead(NikonMtpOperationCode::GetVendorCodes, params);

		if (response.responseCode != NikonMtpResponseCode::OK) {
			throw mtp::MtpException(NikonMtpOperationCode::GetVendorCodes, response.responseCode);
		}

		len = *(uint32_t*)response.data.data();
		offset = sizeof(uint32_t);

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


mtp::MtpDevicePropDescDSV NikonCamera::GetDevicePropDesc(uint32_t devicePropCode) {
	mutexDeviceInfo_.lock();
	if (std::find(deviceInfo_.OperationsSupported.begin(), deviceInfo_.OperationsSupported.end(), NikonMtpOperationCode::GetDevicePropDescEx) != deviceInfo_.OperationsSupported.end()) {
		mutexDeviceInfo_.unlock();

		mtp::MtpParams params;
		params.addUint32(devicePropCode);
		mtp::MtpResponse response = SendCommandAndRead(NikonMtpOperationCode::GetDevicePropDescEx, params);

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

		mtp::MtpParams params;
		params.addUint32(devicePropCode);
		mtp::MtpResponse response = SendCommandAndRead(NikonMtpOperationCode::GetDevicePropValueEx, params);

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

		mtp::MtpParams params;
		params.addUint32(devicePropCode);
		mtp::MtpResponse response = SendCommandAndWrite(NikonMtpOperationCode::SetDevicePropValueEx, params, rawdata);

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
	mtp::MtpParams params;
	return SendCommandAndRead(NikonMtpOperationCode::DeviceReady, params).responseCode;
}
uint32_t NikonCamera::DeviceReadyWhile(uint32_t whileResponseCode, std::stop_token stopToken, size_t sleepTimems) {
	mtp::MtpParams params;
	uint32_t responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, params).responseCode;
	while ((responseCode == whileResponseCode) && !stopToken.stop_requested()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimems));
		responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, params).responseCode;
	}
	return responseCode;
}
uint32_t NikonCamera::DeviceReadyWhile(std::vector<uint32_t> whileResponseCodes, std::stop_token stopToken, size_t sleepTimems) {
	mtp::MtpParams params;
	uint32_t responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, params).responseCode;
	while ((std::find(whileResponseCodes.begin(), whileResponseCodes.end(), responseCode) != whileResponseCodes.end()) && !stopToken.stop_requested()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimems));
		responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, params).responseCode;
	}
	return responseCode;
}
uint32_t NikonCamera::DeviceReadyWhileNot(uint32_t whileNotResponseCode, std::stop_token stopToken, size_t sleepTimems) {
	mtp::MtpParams params;
	uint32_t responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, params).responseCode;
	while ((responseCode != whileNotResponseCode) && !stopToken.stop_requested()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimems));
		responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, params).responseCode;
	}
	return responseCode;
}
uint32_t NikonCamera::DeviceReadyWhileNot(std::vector<uint32_t> whileNotResponseCodes, std::stop_token stopToken, size_t sleepTimems) {
	mtp::MtpParams params;
	uint32_t responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, params).responseCode;
	while ((std::find(whileNotResponseCodes.begin(), whileNotResponseCodes.end(), responseCode) == whileNotResponseCodes.end()) && !stopToken.stop_requested()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimems));
		responseCode = SendCommandAndRead(NikonMtpOperationCode::DeviceReady, params).responseCode;
	}
	return responseCode;
}


uint32_t NikonCamera::StartLiveView(bool wait, std::stop_token stopToken, size_t sleepTimems) {
	auto lvstate = GetDevicePropValue(NikonMtpDevicePropCode::RemoteLiveViewStatus);
	if (std::get<uint8_t>(lvstate) == 1) {
		return NikonMtpResponseCode::OK; //Liveview already On
	}

	mtp::MtpParams params = mtp::MtpParams();
	mtp::MtpResponse response = SendCommand(NikonMtpOperationCode::StartLiveView, params);
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

	mtp::MtpParams params = mtp::MtpParams();
	mtp::MtpResponse response = SendCommand(NikonMtpOperationCode::EndLiveView, params);
	if (response.responseCode != NikonMtpResponseCode::OK) {
		throw mtp::MtpException(NikonMtpOperationCode::EndLiveView, response.responseCode);
	}
}