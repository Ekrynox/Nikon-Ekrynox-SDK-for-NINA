#define NOMINMAX

#include "nek_mtp.hpp"

#include <algorithm>
#include <stdexcept>



using namespace nek::mtp;



//MtpManager
MtpManager& MtpManager::Instance() {
	static MtpManager instance = MtpManager();
	return instance;
}

MtpManager::MtpManager() {
	deviceManager_.p = nullptr;

	startThread();
	std::unique_lock lk(mutexTasks_);
	cvTasks_.wait(lk);
}

void MtpManager::threadTask() {
	mutexTasks_.lock();
	mutexDevice_.lock();

	//Com context
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hr)) {

		//Device Manager
		hr = CoCreateInstance(CLSID_PortableDeviceManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceManager_));
		mutexDevice_.unlock();
		mutexTasks_.unlock();
		if (SUCCEEDED(hr)) {
			cvTasks_.notify_all();

			//Thread Loop
			nek::utils::ThreadedClass::threadTask();

			//Uninit
			mutexTasks_.lock();
			mutexDevice_.lock();
			cvTasks_.notify_one();

			deviceManager_.Release();
			CoUninitialize();

			mutexDevice_.unlock();
			mutexTasks_.unlock();

			return;
		}
		throw MtpDeviceException(MtpExPhase::MANAGER_INIT, hr);
	}

	mutexDevice_.unlock();
	mutexTasks_.unlock();
	throw MtpDeviceException(MtpExPhase::COM_INIT, hr);
}

std::vector<std::wstring> MtpManager::listMtpDevicesPath() {
	return sendTaskWithResult<std::vector<std::wstring>>([this] {
		std::vector<std::wstring> wpdDevices;

		DWORD devicesNb = 0;
		PWSTR* devices = nullptr;
		HRESULT hr;

		mutexDevice_.lock();

		//Update WPD devices list
		deviceManager_->RefreshDeviceList();

		//Get the number of WPD devices
		hr = deviceManager_->GetDevices(NULL, &devicesNb);
		if (FAILED(hr)) {
			mutexDevice_.unlock();
			throw MtpDeviceException(MtpExPhase::MANAGER_DEVICELIST, hr);
		}

		//At least one device
		if (devicesNb > 0) {
			devices = new PWSTR[devicesNb]();
			HRESULT hr = deviceManager_->GetDevices(devices, &devicesNb);
			if (FAILED(hr)) {
				mutexDevice_.unlock();
				delete[] devices;
				throw MtpDeviceException(MtpExPhase::MANAGER_DEVICELIST, hr);
			}

			mutexDevice_.unlock();

			for (DWORD i = 0; i < devicesNb; i++) {
				if (devices[i] != 0) {
					wpdDevices.push_back(std::wstring(devices[i]));
					CoTaskMemFree(devices[i]);
				}
			}

			delete[] devices;
		}
		else {
			mutexDevice_.unlock();
		}

		return wpdDevices;
		});
}

std::map<std::wstring, MtpDeviceInfoDS> MtpManager::listMtpDevices() {
	return sendTaskWithResult<std::map<std::wstring, MtpDeviceInfoDS>>([this] {
		std::map<std::wstring, MtpDeviceInfoDS> wpdDevices;

		DWORD devicesNb = 0;
		PWSTR* devices = nullptr;
		HRESULT hr;

		mutexDevice_.lock();

		//Update WPD devices list
		deviceManager_->RefreshDeviceList();

		//Get the number of WPD devices
		hr = deviceManager_->GetDevices(NULL, &devicesNb);
		if (FAILED(hr)) {
			mutexDevice_.unlock();
			throw MtpDeviceException(MtpExPhase::MANAGER_DEVICELIST, hr);
		}

		//At least one device
		if (devicesNb > 0) {
			devices = new PWSTR[devicesNb]();
			HRESULT hr = deviceManager_->GetDevices(devices, &devicesNb);
			if (FAILED(hr)) {
				mutexDevice_.unlock();
				delete[] devices;
				throw MtpDeviceException(MtpExPhase::MANAGER_DEVICELIST, hr);
			}

			mutexDevice_.unlock();

			for (DWORD i = 0; i < devicesNb; i++) {
				if (devices[i] != 0) {
					wpdDevices.insert(std::pair(std::wstring(devices[i]), MtpDevice(devices[i]).GetDeviceInfo()));
					CoTaskMemFree(devices[i]);
				}
			}

			delete[] devices;
		}
		else {
			mutexDevice_.unlock();
		}

		return wpdDevices;
		});
}

bool MtpManager::isDeviceConnected(std::wstring devicePath) {
	return sendTaskWithResult<bool>([this, devicePath] {
		DWORD devicesNb = 0;
		PWSTR* devices = nullptr;
		HRESULT hr;

		mutexDevice_.lock();

		//Update WPD devices list
		deviceManager_->RefreshDeviceList();

		//Get the number of WPD devices
		hr = deviceManager_->GetDevices(NULL, &devicesNb);
		if (FAILED(hr)) {
			mutexDevice_.unlock();
			throw MtpDeviceException(MtpExPhase::MANAGER_DEVICELIST, hr);
		}

		//At least one device
		if (devicesNb > 0) {
			devices = new PWSTR[devicesNb]();
			HRESULT hr = deviceManager_->GetDevices(devices, &devicesNb);
			if (FAILED(hr)) {
				mutexDevice_.unlock();
				delete[] devices;
				throw MtpDeviceException(MtpExPhase::MANAGER_DEVICELIST, hr);
			}

			mutexDevice_.unlock();

			for (DWORD i = 0; i < devicesNb; i++) {
				if (devices[i] != 0) {
					if (devicePath == std::wstring(devices[i])) return true;
					CoTaskMemFree(devices[i]);
				}
			}

			delete[] devices;
		}
		else {
			mutexDevice_.unlock();
		}

		return false;
		});
}

size_t MtpManager::countMtpDevices() {
	return listMtpDevices().size();
}



//MtpDevice
MtpDevice::MtpDevice(std::wstring devicePath, uint8_t additionalThreadsNb) {
	devicePath_ = devicePath;
	connected_ = false;
	eventCookie_ = nullptr;
	eventCallback_ = new nek::mtp::MtpEventCallback();
	additionalThreadsNb_ = additionalThreadsNb;

	startThreads();
}

MtpDevice::MtpDevice() {
	devicePath_ = (PWSTR)L"";
	connected_ = false;
	eventCookie_ = nullptr;
	eventCallback_ = new nek::mtp::MtpEventCallback();
	additionalThreadsNb_ = 0;
}

MtpDevice::~MtpDevice() {
	Disconnect();
	eventCallback_.Release();
};


void MtpDevice::mainThreadTask() {
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

	device_->Advise(0, eventCallback_, nullptr, &eventCookie_);

	mutexDevice_.unlock();
	mutexTasks_.unlock();
	GetDeviceInfo();
	cvTasks_.notify_all();

	//Thread Loop
	threadTask();

	//Uninit
	mutexTasks_.lock();
	mutexDevice_.lock();

	connected_ = false;
	device_->Unadvise(eventCookie_);
	device_.Release();
	CoUninitialize();

	mutexDevice_.unlock();
	mutexTasks_.unlock();
	cvTasks_.notify_all();
}

void MtpDevice::additionalThreadsTask() {
	//Com context
	initCom();

	//Thread Loop
	threadTask();

	//Uninit
	CoUninitialize();
}



MtpResponse MtpDevice::SendCommand_(CComPtr<IPortableDevice> device, uint16_t operationCode, MtpParams params) {
	MtpResponse result = MtpResponse();
	HRESULT hr;

	CComPtr<IPortableDeviceValues> command;
	hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&command));
	if (FAILED(hr)) {
		throw MtpDeviceException(MtpExPhase::OPERATION_INIT, hr);
	}

	// Set command category and ID
	command->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.fmtid);
	command->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.pid);

	// Set operation code and parameters
	command->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, operationCode);
	command->SetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, params.GetCollection());

	// Send command
	CComPtr<IPortableDeviceValues> commandResult;
	hr = device->SendCommand(0, command, &commandResult);
	if (FAILED(hr)) {
		command.Release();
		throw MtpDeviceException(MtpExPhase::OPERATION_SEND, hr);
	}

	// Extract response code
	ULONG tempCode;
	hr = commandResult->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &tempCode);
	result.responseCode = static_cast<uint16_t>(tempCode);
	if (FAILED(hr)) {
		command.Release();
		commandResult.Release();
		throw MtpDeviceException(MtpExPhase::OPERATION_RESPONSE, hr);
	}

	// Extract response parameters
	CComPtr<IPortableDevicePropVariantCollection> parametersCollection;
	hr = commandResult->GetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_RESPONSE_PARAMS, &parametersCollection);
	if (FAILED(hr)) {
		command.Release();
		commandResult.Release();
		throw MtpDeviceException(MtpExPhase::OPERATION_RESPONSE, hr);
	}

	result.GetParams().SetCollection(parametersCollection);
	parametersCollection.Release();
	command.Release();
	commandResult.Release();

	return result;
}

MtpResponse MtpDevice::SendCommandAndRead_(CComPtr<IPortableDevice> device, uint16_t operationCode, MtpParams params) {
	MtpResponse result = MtpResponse();
	HRESULT hr;

	CComPtr<IPortableDeviceValues> command;
	hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&command));
	if (FAILED(hr)) {
		throw MtpDeviceException(MtpExPhase::OPERATIONREAD_INIT, hr);
	}

	// Set command category and ID
	command->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_READ.fmtid);
	command->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_READ.pid);

	// Set operation code and parameters
	command->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, operationCode);
	command->SetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, params.GetCollection());

	// Send command
	CComPtr<IPortableDeviceValues> commandResult;
	hr = device->SendCommand(0, command, &commandResult);
	if (FAILED(hr)) {
		command.Release();
		throw MtpDeviceException(MtpExPhase::OPERATIONREAD_SEND, hr);
	}

	LPWSTR context;
	hr = commandResult->GetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, &context);
	if (FAILED(hr)) {
		command.Release();
		commandResult.Release();
		throw MtpDeviceException(MtpExPhase::OPERATIONREAD_RESPONSE, hr);
	}

	ULONG totalSize;
	hr = commandResult->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_TOTAL_DATA_SIZE, &totalSize);
	if (FAILED(hr)) {
		command.Release();
		commandResult.Release();
		throw MtpDeviceException(MtpExPhase::OPERATIONREAD_RESPONSE, hr);
	}

	ULONG optimalSize;
	hr = commandResult->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPTIMAL_TRANSFER_BUFFER_SIZE, &optimalSize);
	if (FAILED(hr)) {
		command.Release();
		commandResult.Release();
		throw MtpDeviceException(MtpExPhase::OPERATIONREAD_RESPONSE, hr);
	}

	command.Release();
	commandResult.Release();


	// Start Data Transfert
	hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&command));
	if (FAILED(hr)) {
		CoTaskMemFree(context);
		throw MtpDeviceException(MtpExPhase::DATAREAD_INIT, hr);
	}
	command->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_READ_DATA.fmtid);
	command->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_READ_DATA.pid);
	command->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, context);
	optimalSize = std::min(optimalSize, totalSize);
	uint8_t* buffer = new uint8_t[optimalSize];
	result.data.resize(totalSize);
	command->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_NUM_BYTES_TO_READ, optimalSize);
	command->SetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, buffer, optimalSize);

	uint8_t* b = nullptr;
	DWORD bNb = 0;
	DWORD offset = 0;
	size_t retry = 0;
	do {
		hr = device->SendCommand(0, command, &commandResult);
		if (FAILED(hr)) {
			CoTaskMemFree(context);
			command.Release();
			commandResult.Release();
			throw MtpDeviceException(MtpExPhase::DATAREAD_SEND, hr);
		}
		hr = commandResult->GetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, &b, &bNb);
		commandResult.Release();
		if (SUCCEEDED(hr) && (b != nullptr)) {
			if (bNb > 0) {
				std::memcpy(result.data.data() + offset, b, bNb);
				CoTaskMemFree(b);
				b = nullptr;
				offset += bNb;
				retry = 0;
			}
			else {
				retry += 1;
				CoTaskMemFree(b);
				b = nullptr;
				if (retry >= 10) {
					result.responseCode = MtpResponseCode::Incomplete_Transfer;
					CoTaskMemFree(context);
					command.Release();
					commandResult.Release();
					return result;
				}
			}
		}
	} while (totalSize > offset);

	delete[] buffer;
	command.Release();


	// End Data Transfert
	hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&command));
	if (FAILED(hr)) {
		CoTaskMemFree(context);
		throw MtpDeviceException(MtpExPhase::ENDREAD_INIT, hr);
	}
	command->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.fmtid);
	command->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.pid);
	command->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, context);
	hr = device->SendCommand(0, command, &commandResult);
	if (FAILED(hr)) {
		CoTaskMemFree(context);
		command.Release();
		throw MtpDeviceException(MtpExPhase::ENDREAD_SEND, hr);
	}

	// Extract response code
	ULONG tempCode;
	hr = commandResult->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &tempCode);
	result.responseCode = static_cast<uint16_t>(tempCode);
	if (FAILED(hr)) {
		CoTaskMemFree(context);
		command.Release();
		commandResult.Release();
		throw MtpDeviceException(MtpExPhase::ENDREAD_RESPONSE, hr);
	}

	// Extract response parameters
	CComPtr<IPortableDevicePropVariantCollection> parametersCollection;
	hr = commandResult->GetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_RESPONSE_PARAMS, &parametersCollection);
	if (FAILED(hr)) {
		CoTaskMemFree(context);
		command.Release();
		commandResult.Release();
		throw MtpDeviceException(MtpExPhase::ENDREAD_RESPONSE, hr);
	}
	result.GetParams().SetCollection(parametersCollection);

	parametersCollection.Release();
	CoTaskMemFree(context);
	command.Release();
	commandResult.Release();
	return result;
}

MtpResponse MtpDevice::SendCommandAndWrite_(CComPtr<IPortableDevice> device, uint16_t operationCode, MtpParams params, std::vector<uint8_t> data) {
	MtpResponse result = MtpResponse();
	HRESULT hr;

	CComPtr<IPortableDeviceValues> command;
	hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&command));
	if (FAILED(hr)) {
		throw MtpDeviceException(MtpExPhase::OPERATIONWRITE_INIT, hr);
	}

	// Set command category and ID
	command->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_WRITE.fmtid);
	command->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_WRITE.pid);

	// Set operation code and parameters
	command->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, operationCode);
	command->SetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, params.GetCollection());
	command->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_TOTAL_DATA_SIZE, (ULONG)data.size());

	// Send command
	CComPtr<IPortableDeviceValues> commandResult;
	hr = device->SendCommand(0, command, &commandResult);
	if (FAILED(hr)) {
		command.Release();
		commandResult.Release();
		throw MtpDeviceException(MtpExPhase::OPERATIONWRITE_SEND, hr);
	}

	LPWSTR context;
	hr = commandResult->GetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, &context);
	if (FAILED(hr)) {
		command.Release();
		commandResult.Release();
		throw MtpDeviceException(MtpExPhase::OPERATIONWRITE_RESPONSE, hr);
	}

	ULONG optimalSize;
	hr = commandResult->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPTIMAL_TRANSFER_BUFFER_SIZE, &optimalSize);
	if (FAILED(hr)) {
		command.Release();
		commandResult.Release();
		CoTaskMemFree(context);
		throw MtpDeviceException(MtpExPhase::OPERATIONWRITE_RESPONSE, hr);
	}

	command.Release();
	commandResult.Release();


	// Start Data Transfert
	DWORD offset = 0;
	while (offset < data.size()) {
		hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&command));
		if (FAILED(hr)) {
			CoTaskMemFree(context);
			throw MtpDeviceException(MtpExPhase::DATAWRITE_INIT, hr);
		}

		command->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_WRITE_DATA.fmtid);
		command->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_WRITE_DATA.pid);
		command->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, context);

		optimalSize = std::min(optimalSize, (ULONG)(data.size() - offset));
		command->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_NUM_BYTES_TO_WRITE, optimalSize);
		command->SetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, data.data() + offset, optimalSize);

		hr = device->SendCommand(0, command, &commandResult);
		if (FAILED(hr)) {
			command.Release();
			commandResult.Release();
			CoTaskMemFree(context);
			throw MtpDeviceException(MtpExPhase::DATAWRITE_SEND, hr);
		}

		offset += optimalSize;
		commandResult.Release();
		command.Release();
	}


	// End Data Transfert
	hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&command));
	if (FAILED(hr)) {
		command.Release();
		commandResult.Release();
		CoTaskMemFree(context);
		throw MtpDeviceException(MtpExPhase::ENDWRITE_INIT, hr);
	}

	command->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.fmtid);
	command->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.pid);
	command->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, context);
	hr = device->SendCommand(0, command, &commandResult);
	if (FAILED(hr)) {
		command.Release();
		CoTaskMemFree(context);
		throw MtpDeviceException(MtpExPhase::ENDWRITE_SEND, hr);
	}

	// Extract response code
	ULONG tempCode;
	hr = commandResult->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &tempCode);
	result.responseCode = static_cast<uint16_t>(tempCode);
	if (FAILED(hr)) {
		command.Release();
		commandResult.Release();
		CoTaskMemFree(context);
		throw MtpDeviceException(MtpExPhase::ENDWRITE_RESPONSE, hr);
	}

	// Extract response parameters
	CComPtr<IPortableDevicePropVariantCollection> parametersCollection;
	hr = commandResult->GetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_RESPONSE_PARAMS, &parametersCollection);
	if (FAILED(hr)) {
		command.Release();
		commandResult.Release();
		CoTaskMemFree(context);
		throw MtpDeviceException(MtpExPhase::ENDWRITE_RESPONSE, hr);
	}
	result.GetParams().SetCollection(parametersCollection);

	parametersCollection.Release();
	command.Release();
	commandResult.Release();
	CoTaskMemFree(context);
	return result;
}



bool MtpDevice::isConnected() const { 
	return (this != nullptr) && connected_ && running_;
}

void MtpDevice::initCom() {
	//Com context
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (hr == RPC_E_CHANGED_MODE) {
		throw MtpDeviceException(MtpExPhase::COM_INIT, hr);
	}

	//Device Client
	if (deviceClient_ != nullptr) {
		return; //Already initialized
	}
	hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceClient_));
	if (FAILED(hr)) {
		throw MtpDeviceException(MtpExPhase::DEVICECLIENT_INIT, hr);
	}
	deviceClient_->SetStringValue(WPD_CLIENT_NAME, CLIENT_NAME);
	deviceClient_->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, CLIENT_MAJOR_VER);
	deviceClient_->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, CLIENT_MINOR_VER);
	deviceClient_->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, CLIENT_REVISION);
}

void MtpDevice::initDevice() {
	if (isConnected()) {
		return; //Already connected
	}

	HRESULT hr = CoCreateInstance(CLSID_PortableDeviceFTM, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&device_));
	if (FAILED(hr)) {
		throw MtpDeviceException(MtpExPhase::DEVICE_INIT, hr);
	}

	hr = device_->Open(devicePath_.c_str(), deviceClient_);
	if (FAILED(hr)) {
		device_.Release();
		throw MtpDeviceException(MtpExPhase::DEVICE_INIT, hr);
	}

	connected_ = true;
}

void MtpDevice::Disconnect() {
	if (!isConnected()) {
		return; //Already disconnected
	}
	connected_ = false;

	mutexTasks_.lock();
	tasks_.clear();
	mutexTasks_.unlock();
	stopThread();
	
	eventCallback_->OnEvent(nek::mtp::MtpEvent(MtpEventCode::DeviceInfoChanged)); // Notify Disconnection: DeviceInfoChanged
}

void MtpDevice::startThreads() {
	running_ = true;
	mutexThreads_.lock();
	threads_.push_back(std::thread([this] { this->mainThreadTask(); }));
	mutexThreads_.unlock();
	std::unique_lock lk(mutexTasks_);
	cvTasks_.wait(lk, [this] { return eventCookie_ != nullptr; });

	for (uint8_t i = 0; i < additionalThreadsNb_; i++) {
		mutexThreads_.lock();
		threads_.push_back(std::thread([this] { this->additionalThreadsTask(); }));
		mutexThreads_.unlock();
	}
}


MtpResponse MtpDevice::SendCommand(uint16_t operationCode, MtpParams params) {
	if (!isConnected()) {
		throw MtpDeviceException(MtpExPhase::DEVICE_NOT_CONNECTED, MtpExCode::DEVICE_DISCONNECTED);
	}
	auto func = [this, operationCode, &params] {
		MtpResponse result;
		this->mutexDevice_.lock();
		try {
			result = this->SendCommand_(this->device_, operationCode, params);
		}
		catch (MtpDeviceException& e) {
			this->mutexDevice_.unlock();
			if (e.code == nek::mtp::MtpExCode::DEVICE_DISCONNECTED || !isConnected() || !MtpManager::Instance().isDeviceConnected(this->devicePath_)) {
				Disconnect();
			}
			throw;
		}
		this->mutexDevice_.unlock();
		return result;
		};

	auto thId = std::this_thread::get_id();
	for (auto& t : threads_) {
		if (t.get_id() == thId) {
			return func(); // We are in on of the threads of our Camera Class
		}
	}
	return sendTaskWithResult<MtpResponse>(func);
}
MtpResponse MtpDevice::SendCommandAndRead(uint16_t operationCode, MtpParams params) {
	if (!isConnected()) {
		throw MtpDeviceException(MtpExPhase::DEVICE_NOT_CONNECTED, MtpExCode::DEVICE_DISCONNECTED);
	}

	auto func = [this, operationCode, &params] {
		MtpResponse result;
		this->mutexDevice_.lock();
		try {
			result = this->SendCommandAndRead_(this->device_, operationCode, params);
		}
		catch (MtpDeviceException& e) {
			this->mutexDevice_.unlock();
			if (e.code == nek::mtp::MtpExCode::DEVICE_DISCONNECTED || !isConnected() || !MtpManager::Instance().isDeviceConnected(this->devicePath_)) {
				Disconnect();
			}
			throw;
		}
		this->mutexDevice_.unlock();
		return result;
		};

	auto thId = std::this_thread::get_id();
	for (auto& t : threads_) {
		if (t.get_id() == thId) {
			return func(); // We are in on of the threads of our Camera Class
		}
	}
	return sendTaskWithResult<MtpResponse>(func);
}
MtpResponse MtpDevice::SendCommandAndWrite(uint16_t operationCode, MtpParams params, std::vector<uint8_t> data) {
	if (!isConnected()) {
		throw MtpDeviceException(MtpExPhase::DEVICE_NOT_CONNECTED, MtpExCode::DEVICE_DISCONNECTED);
	}
	auto func = [this, operationCode, &params, &data] {
		MtpResponse result;
		this->mutexDevice_.lock();
		try {
			result = this->SendCommandAndWrite_(this->device_, operationCode, params, data);
		}
		catch (MtpDeviceException& e) {
			this->mutexDevice_.unlock();
			if (e.code == nek::mtp::MtpExCode::DEVICE_DISCONNECTED || !isConnected() || !MtpManager::Instance().isDeviceConnected(this->devicePath_)) {
				Disconnect();
			}
			throw;
		}
		this->mutexDevice_.unlock();
		return result;
		};

	auto thId = std::this_thread::get_id();
	for (auto& t : threads_) {
		if (t.get_id() == thId) {
			return func(); // We are in on of the threads of our Camera Class
		}
	}
	return sendTaskWithResult<MtpResponse>(func);
}


size_t MtpDevice::RegisterCallback(std::function<void(MtpEvent)> callback) { return eventCallback_->RegisterCallback(callback); }
void MtpDevice::UnregisterCallback(size_t id) { return eventCallback_->UnregisterCallback(id); }



MtpDeviceInfoDS MtpDevice::GetDeviceInfo() {
	MtpParams params;
	MtpResponse response = SendCommandAndRead(MtpOperationCode::GetDeviceInfo, params);

	if (response.responseCode != MtpResponseCode::OK) {
		throw MtpException(MtpOperationCode::GetDeviceInfo, response.responseCode);
	}

	MtpDeviceInfoDS deviceInfo;
	size_t offset = 0;
	uint32_t len;

	deviceInfo.StandardVersion = *(uint16_t *)(response.data.data() + offset);
	offset += sizeof(uint16_t);
	deviceInfo.VendorExtensionID = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	deviceInfo.VendorExtensionVersion = *(uint16_t*)(response.data.data() + offset);
	offset += sizeof(uint16_t);

	len = *(uint8_t*)(response.data.data() + offset);
	offset += sizeof(uint8_t);
	deviceInfo.VendorExtensionDesc.resize(len);
	std::memcpy(deviceInfo.VendorExtensionDesc.data(), response.data.data() + offset, sizeof(uint16_t) * len);
	offset += sizeof(char16_t) * len;

	deviceInfo.FunctionalMode = *(uint16_t*)(response.data.data() + offset);
	offset += sizeof(uint16_t);

	len = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	deviceInfo.OperationsSupported.resize(len);
	std::memcpy(deviceInfo.OperationsSupported.data(), response.data.data() + offset, sizeof(uint16_t) * len);
	offset += sizeof(uint16_t) * len;

	len = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	deviceInfo.EventsSupported.resize(len);
	std::memcpy(deviceInfo.EventsSupported.data(), response.data.data() + offset, sizeof(uint16_t) * len);
	offset += sizeof(uint16_t) * len;

	len = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	deviceInfo.DevicePropertiesSupported.resize(len);
	std::memcpy(deviceInfo.DevicePropertiesSupported.data(), response.data.data() + offset, sizeof(uint16_t) * len);
	offset += sizeof(uint16_t) * len;

	len = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	deviceInfo.CaptureFormats.resize(len);
	std::memcpy(deviceInfo.CaptureFormats.data(), response.data.data() + offset, sizeof(uint16_t) * len);
	offset += sizeof(uint16_t) * len;

	len = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	deviceInfo.ImageFormats.resize(len);
	std::memcpy(deviceInfo.ImageFormats.data(), response.data.data() + offset, sizeof(uint16_t) * len);
	offset += sizeof(uint16_t) * len;

	len = *(uint8_t*)(response.data.data() + offset);
	offset += sizeof(uint8_t);
	deviceInfo.Manufacture.resize(len);
	std::memcpy(deviceInfo.Manufacture.data(), response.data.data() + offset, sizeof(uint16_t) * len);
	offset += sizeof(char16_t) * len;

	len = *(uint8_t*)(response.data.data() + offset);
	offset += sizeof(uint8_t);
	deviceInfo.Model.resize(len);
	std::memcpy(deviceInfo.Model.data(), response.data.data() + offset, sizeof(uint16_t) * len);
	offset += sizeof(char16_t) * len;

	len = *(uint8_t*)(response.data.data() + offset);
	offset += sizeof(uint8_t);
	deviceInfo.DeviceVersion.resize(len);
	std::memcpy(deviceInfo.DeviceVersion.data(), response.data.data() + offset, sizeof(uint16_t) * len);
	offset += sizeof(char16_t) * len;

	len = *(uint8_t*)(response.data.data() + offset);
	offset += sizeof(uint8_t);
	deviceInfo.SerialNumber.resize(len);
	std::memcpy(deviceInfo.SerialNumber.data(), response.data.data() + offset, sizeof(uint16_t) * len);
	offset += sizeof(char16_t) * len;

	mutexDeviceInfo_.lock();
	deviceInfo_ = deviceInfo;
	mutexDeviceInfo_.unlock();
	return deviceInfo;
}


MtpObjectInfoDS MtpDevice::GetObjectInfo(uint32_t handle) {
	MtpParams params;
	params.addUint32(handle);
	MtpResponse response = SendCommandAndRead(MtpOperationCode::GetObjectInfo, params);

	if (response.responseCode != MtpResponseCode::OK) {
		throw MtpException(MtpOperationCode::GetDeviceInfo, response.responseCode);
	}

	MtpObjectInfoDS objectInfo;
	size_t offset = 0;
	uint32_t len;

	objectInfo.StorageID = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	objectInfo.ObjectFormat = *(uint16_t*)(response.data.data() + offset);
	offset += sizeof(uint16_t);
	objectInfo.ProtectionStatus = *(uint16_t*)(response.data.data() + offset);
	offset += sizeof(uint16_t);
	objectInfo.ObjectCompressedSize = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	objectInfo.ThumbFormat = *(uint16_t*)(response.data.data() + offset);
	offset += sizeof(uint16_t);
	objectInfo.ThumbCompressedSize = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	objectInfo.ThumbPixWidth = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	objectInfo.ThumbPixHeight = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	objectInfo.ImagePixWidth = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	objectInfo.ImagePixHeight = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	objectInfo.ImageBitDepth = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	objectInfo.ParentObject = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	objectInfo.AssociationType = *(uint16_t*)(response.data.data() + offset);
	offset += sizeof(uint16_t);
	objectInfo.AssociationDesc = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);
	objectInfo.SequenceNumber = *(uint32_t*)(response.data.data() + offset);
	offset += sizeof(uint32_t);

	len = *(uint8_t*)(response.data.data() + offset);
	offset += sizeof(uint8_t);
	objectInfo.Filename.resize(len);
	std::memcpy(objectInfo.Filename.data(), response.data.data() + offset, sizeof(uint16_t) * len);
	offset += sizeof(char16_t) * len;

	len = *(uint8_t*)(response.data.data() + offset);
	offset += sizeof(uint8_t);
	objectInfo.CaptureDate.resize(len);
	std::memcpy(objectInfo.CaptureDate.data(), response.data.data() + offset, sizeof(uint16_t) * len);
	offset += sizeof(char16_t) * len;

	len = *(uint8_t*)(response.data.data() + offset);
	offset += sizeof(uint8_t);
	objectInfo.ModificationDate.resize(len);
	std::memcpy(objectInfo.ModificationDate.data(), response.data.data() + offset, sizeof(uint16_t) * len);
	offset += sizeof(char16_t) * len;

	objectInfo.Keywords = *(uint8_t*)(response.data.data() + offset);
	offset += sizeof(uint8_t);

	return objectInfo;
}


MtpDevicePropDescDSV MtpDevice::GetDevicePropDesc(uint16_t devicePropCode) {
	MtpParams params;
	params.addUint32(static_cast<uint32_t>(devicePropCode));
	MtpResponse response = SendCommandAndRead(MtpOperationCode::GetDevicePropDesc, params);

	if (response.responseCode != MtpResponseCode::OK) {
		throw MtpException(MtpOperationCode::GetDevicePropDesc, response.responseCode);
	}

	MtpDevicePropDescDSV result = GetDevicePropDesc_(response);
	mutexDeviceInfo_.lock();
	if (devicePropDataType_.find(result.DevicePropertyCode) == devicePropDataType_.end()) {
		devicePropDataType_.insert(std::pair<uint32_t, uint16_t>(result.DevicePropertyCode, result.DataType)); //Register Type for futur use in Get Value
	}
	mutexDeviceInfo_.unlock();
	return result;
}
MtpDevicePropDescDSV MtpDevice::GetDevicePropDesc_(MtpResponse& response) {
	MtpDevicePropDescDSV result;
	size_t offset = 0;

	result.DevicePropertyCode = static_cast<uint32_t>(*(uint16_t*)(response.data.data() + offset));
	offset += sizeof(uint16_t);

	result.DataType = *(uint16_t*)(response.data.data() + offset);
	offset += sizeof(uint16_t);

	result.GetSet = *(uint8_t*)(response.data.data() + offset);
	offset += sizeof(uint8_t);

	//FactoryDefault & CurrentValue
	switch (result.DataType) {
		case MtpDatatypeCode::Int8:
			result.FactoryDefaultValue = *(int8_t*)(response.data.data() + offset);
			offset += sizeof(int8_t);
			result.CurrentValue = *(int8_t*)(response.data.data() + offset);
			offset += sizeof(int8_t);
			break;
		case MtpDatatypeCode::UInt8:
			result.FactoryDefaultValue = *(uint8_t*)(response.data.data() + offset);
			offset += sizeof(uint8_t);
			result.CurrentValue = *(uint8_t*)(response.data.data() + offset);
			offset += sizeof(uint8_t);
			break;
		case MtpDatatypeCode::Int16:
			result.FactoryDefaultValue = *(int16_t*)(response.data.data() + offset);
			offset += sizeof(int16_t);
			result.CurrentValue = *(int16_t*)(response.data.data() + offset);
			offset += sizeof(int16_t);
			break;
		case MtpDatatypeCode::UInt16:
			result.FactoryDefaultValue = *(uint16_t*)(response.data.data() + offset);
			offset += sizeof(uint16_t);
			result.CurrentValue = *(uint16_t*)(response.data.data() + offset);
			offset += sizeof(uint16_t);
			break;
		case MtpDatatypeCode::Int32:
			result.FactoryDefaultValue = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			result.CurrentValue = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			break;
		case MtpDatatypeCode::UInt32:
			result.FactoryDefaultValue = *(uint32_t*)(response.data.data() + offset);
			offset += sizeof(uint32_t);
			result.CurrentValue = *(uint32_t*)(response.data.data() + offset);
			offset += sizeof(uint32_t);
			break;
		case MtpDatatypeCode::Int64:
			result.FactoryDefaultValue = *(int64_t*)(response.data.data() + offset);
			offset += sizeof(int64_t);
			result.CurrentValue = *(int64_t*)(response.data.data() + offset);
			offset += sizeof(int64_t);
			break;
		case MtpDatatypeCode::UInt64:
			result.FactoryDefaultValue = *(uint64_t*)(response.data.data() + offset);
			offset += sizeof(uint64_t);
			result.CurrentValue = *(uint64_t*)(response.data.data() + offset);
			offset += sizeof(uint64_t);
			break;
		case MtpDatatypeCode::Int128:
			throw std::runtime_error("int128 Not Implemented");
			break;
		case MtpDatatypeCode::UInt128:
			throw std::runtime_error("uint128 Not Implemented");
			break;
		case MtpDatatypeCode::ArrayInt8:
		{
			int32_t len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			auto data = std::vector<int8_t>(len);
			std::memcpy(data.data(), (int8_t*)(response.data.data() + offset), sizeof(int8_t) * len);
			offset += sizeof(int8_t) * len;
			result.FactoryDefaultValue = std::move(data);

			len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			data = std::vector<int8_t>(len);
			std::memcpy(data.data(), (int8_t*)(response.data.data() + offset), sizeof(int8_t) * len);
			offset += sizeof(int8_t) * len;
			result.CurrentValue = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayUInt8:
		{
			int32_t len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			auto data = std::vector<uint8_t>(len);
			std::memcpy(data.data(), (uint8_t*)(response.data.data() + offset), sizeof(uint8_t) * len);
			offset += sizeof(uint8_t) * len;
			result.FactoryDefaultValue = std::move(data);

			len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			data = std::vector<uint8_t>(len);
			std::memcpy(data.data(), (uint8_t*)(response.data.data() + offset), sizeof(uint8_t) * len);
			offset += sizeof(uint8_t) * len;
			result.CurrentValue = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayInt16:
		{
			int32_t len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			auto data = std::vector<int16_t>(len);
			std::memcpy(data.data(), (int16_t*)(response.data.data() + offset), sizeof(int16_t) * len);
			offset += sizeof(int16_t) * len;
			result.FactoryDefaultValue = std::move(data);

			len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			data = std::vector<int16_t>(len);
			std::memcpy(data.data(), (int16_t*)(response.data.data() + offset), sizeof(int16_t) * len);
			offset += sizeof(int16_t) * len;
			result.CurrentValue = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayUInt16:
		{
			int32_t len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			auto data = std::vector<uint16_t>(len);
			std::memcpy(data.data(), (uint16_t*)(response.data.data() + offset), sizeof(uint16_t) * len);
			offset += sizeof(uint16_t) * len;
			result.FactoryDefaultValue = std::move(data);

			len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			data = std::vector<uint16_t>(len);
			std::memcpy(data.data(), (uint16_t*)(response.data.data() + offset), sizeof(uint16_t) * len);
			offset += sizeof(uint16_t) * len;
			result.CurrentValue = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayInt32:
		{
			int32_t len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			auto data = std::vector<int32_t>(len);
			std::memcpy(data.data(), (int32_t*)(response.data.data() + offset), sizeof(int32_t) * len);
			offset += sizeof(int32_t) * len;
			result.FactoryDefaultValue = std::move(data);

			len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			data = std::vector<int32_t>(len);
			std::memcpy(data.data(), (int32_t*)(response.data.data() + offset), sizeof(int32_t) * len);
			offset += sizeof(int32_t) * len;
			result.CurrentValue = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayUInt32:
		{
			int32_t len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			auto data = std::vector<uint32_t>(len);
			std::memcpy(data.data(), (uint32_t*)(response.data.data() + offset), sizeof(uint32_t) * len);
			offset += sizeof(uint32_t) * len;
			result.FactoryDefaultValue = std::move(data);

			len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			data = std::vector<uint32_t>(len);
			std::memcpy(data.data(), (uint32_t*)(response.data.data() + offset), sizeof(uint32_t) * len);
			offset += sizeof(uint32_t) * len;
			result.CurrentValue = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayInt64:
		{
			int32_t len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			auto data = std::vector<int64_t>(len);
			std::memcpy(data.data(), (int64_t*)(response.data.data() + offset), sizeof(int64_t) * len);
			offset += sizeof(int64_t) * len;
			result.FactoryDefaultValue = std::move(data);

			len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			data = std::vector<int64_t>(len);
			std::memcpy(data.data(), (int64_t*)(response.data.data() + offset), sizeof(int64_t) * len);
			offset += sizeof(int64_t) * len;
			result.CurrentValue = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayUInt64:
		{
			int32_t len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			auto data = std::vector<uint64_t>(len);
			std::memcpy(data.data(), (uint64_t*)(response.data.data() + offset), sizeof(uint64_t) * len);
			offset += sizeof(uint64_t) * len;
			result.FactoryDefaultValue = std::move(data);

			len = *(int32_t*)(response.data.data() + offset);
			offset += sizeof(int32_t);
			data = std::vector<uint64_t>(len);
			std::memcpy(data.data(), (uint64_t*)(response.data.data() + offset), sizeof(uint64_t) * len);
			offset += sizeof(uint64_t) * len;
			result.CurrentValue = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayInt128:
			throw std::runtime_error("int128 Not Implemented");
			break;
		case MtpDatatypeCode::ArrayUInt128:
			throw std::runtime_error("uint128 Not Implemented");
			break;
		case MtpDatatypeCode::String:
		{
			auto len = *(uint8_t*)(response.data.data() + offset);
			offset += sizeof(uint8_t);
			auto str = std::vector<char16_t>(len);
			std::memcpy(str.data(), (char16_t*)(response.data.data() + offset), sizeof(char16_t) * len);
			offset += sizeof(char16_t) * len;
			result.FactoryDefaultValue = std::wstring(str.begin(), str.end());

			len = *(uint8_t*)(response.data.data() + offset);
			offset += sizeof(uint8_t);
			str = std::vector<char16_t>(len);
			std::memcpy(str.data(), (char16_t*)(response.data.data() + offset), sizeof(char16_t) * len);
			offset += sizeof(char16_t) * len;
			result.CurrentValue = std::wstring(str.begin(), str.end());
		}
		break;
		default:
			throw std::runtime_error("Unknown Type");
			break;
	}

	result.FormFlag = *(uint8_t*)(response.data.data() + offset);
	offset += sizeof(uint8_t);

	if (result.FormFlag == MtpFormtypeCode::Empty) {
		return result;
	}
	else if (result.FormFlag == MtpFormtypeCode::Range) {
		result.FORM = MtpRangeFormV{};
		switch (result.DataType) {
			case MtpDatatypeCode::Int8:
				std::get<MtpRangeFormV>(result.FORM).min = *(int8_t*)(response.data.data() + offset);
				offset += sizeof(int8_t);
				std::get<MtpRangeFormV>(result.FORM).max = *(int8_t*)(response.data.data() + offset);
				offset += sizeof(int8_t);
				std::get<MtpRangeFormV>(result.FORM).step = *(int8_t*)(response.data.data() + offset);
				offset += sizeof(int8_t);
				break;
			case MtpDatatypeCode::UInt8:
				std::get<MtpRangeFormV>(result.FORM).min = *(uint8_t*)(response.data.data() + offset);
				offset += sizeof(uint8_t);
				std::get<MtpRangeFormV>(result.FORM).max = *(uint8_t*)(response.data.data() + offset);
				offset += sizeof(uint8_t);
				std::get<MtpRangeFormV>(result.FORM).step = *(uint8_t*)(response.data.data() + offset);
				offset += sizeof(uint8_t);
				break;
			case MtpDatatypeCode::Int16:
				std::get<MtpRangeFormV>(result.FORM).min = *(int16_t*)(response.data.data() + offset);
				offset += sizeof(int16_t);
				std::get<MtpRangeFormV>(result.FORM).max = *(int16_t*)(response.data.data() + offset);
				offset += sizeof(int16_t);
				std::get<MtpRangeFormV>(result.FORM).step = *(int16_t*)(response.data.data() + offset);
				offset += sizeof(int16_t);
				break;
			case MtpDatatypeCode::UInt16:
				std::get<MtpRangeFormV>(result.FORM).min = *(uint16_t*)(response.data.data() + offset);
				offset += sizeof(uint16_t);
				std::get<MtpRangeFormV>(result.FORM).max = *(uint16_t*)(response.data.data() + offset);
				offset += sizeof(uint16_t);
				std::get<MtpRangeFormV>(result.FORM).step = *(uint16_t*)(response.data.data() + offset);
				offset += sizeof(uint16_t);
				break;
			case MtpDatatypeCode::Int32:
				std::get<MtpRangeFormV>(result.FORM).min = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				std::get<MtpRangeFormV>(result.FORM).max = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				std::get<MtpRangeFormV>(result.FORM).step = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				break;
			case MtpDatatypeCode::UInt32:
				std::get<MtpRangeFormV>(result.FORM).min = *(uint32_t*)(response.data.data() + offset);
				offset += sizeof(uint32_t);
				std::get<MtpRangeFormV>(result.FORM).max = *(uint32_t*)(response.data.data() + offset);
				offset += sizeof(uint32_t);
				std::get<MtpRangeFormV>(result.FORM).step = *(uint32_t*)(response.data.data() + offset);
				offset += sizeof(uint32_t);
				break;
			case MtpDatatypeCode::Int64:
				std::get<MtpRangeFormV>(result.FORM).min = *(int64_t*)(response.data.data() + offset);
				offset += sizeof(int64_t);
				std::get<MtpRangeFormV>(result.FORM).max = *(int64_t*)(response.data.data() + offset);
				offset += sizeof(int64_t);
				std::get<MtpRangeFormV>(result.FORM).step = *(int64_t*)(response.data.data() + offset);
				offset += sizeof(int64_t);
				break;
			case MtpDatatypeCode::UInt64:
				std::get<MtpRangeFormV>(result.FORM).min = *(uint64_t*)(response.data.data() + offset);
				offset += sizeof(uint64_t);
				std::get<MtpRangeFormV>(result.FORM).max = *(uint64_t*)(response.data.data() + offset);
				offset += sizeof(uint64_t);
				std::get<MtpRangeFormV>(result.FORM).step = *(uint64_t*)(response.data.data() + offset);
				offset += sizeof(uint64_t);
				break;
			case MtpDatatypeCode::Int128:
				throw std::runtime_error("int128 Not Implemented");
				break;
			case MtpDatatypeCode::UInt128:
				throw std::runtime_error("uint128 Not Implemented");
				break;
			case MtpDatatypeCode::ArrayInt8:
			{
				int32_t len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<int8_t>(len);
				std::memcpy(data.data(), (int8_t*)(response.data.data() + offset), sizeof(int8_t)* len);
				offset += sizeof(int8_t) * len;
				std::get<MtpRangeFormV>(result.FORM).min = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<int8_t>(len);
				std::memcpy(data.data(), (int8_t*)(response.data.data() + offset), sizeof(int8_t)* len);
				offset += sizeof(int8_t) * len;
				std::get<MtpRangeFormV>(result.FORM).max = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<int8_t>(len);
				std::memcpy(data.data(), (int8_t*)(response.data.data() + offset), sizeof(int8_t)* len);
				offset += sizeof(int8_t) * len;
				std::get<MtpRangeFormV>(result.FORM).step = std::move(data);
			}
			break;
			case MtpDatatypeCode::ArrayUInt8:
			{
				int32_t len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<uint8_t>(len);
				std::memcpy(data.data(), (uint8_t*)(response.data.data() + offset), sizeof(uint8_t)* len);
				offset += sizeof(uint8_t) * len;
				std::get<MtpRangeFormV>(result.FORM).min = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<uint8_t>(len);
				std::memcpy(data.data(), (uint8_t*)(response.data.data() + offset), sizeof(uint8_t)* len);
				offset += sizeof(uint8_t) * len;
				std::get<MtpRangeFormV>(result.FORM).max = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<uint8_t>(len);
				std::memcpy(data.data(), (uint8_t*)(response.data.data() + offset), sizeof(uint8_t)* len);
				offset += sizeof(uint8_t) * len;
				std::get<MtpRangeFormV>(result.FORM).step = std::move(data);
			}
			break;
			case MtpDatatypeCode::ArrayInt16:
			{
				int32_t len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<int16_t>(len);
				std::memcpy(data.data(), (int16_t*)(response.data.data() + offset), sizeof(int16_t) * len);
				offset += sizeof(int16_t) * len;
				std::get<MtpRangeFormV>(result.FORM).min = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<int16_t>(len);
				std::memcpy(data.data(), (int16_t*)(response.data.data() + offset), sizeof(int16_t) * len);
				offset += sizeof(int16_t) * len;
				std::get<MtpRangeFormV>(result.FORM).max = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<int16_t>(len);
				std::memcpy(data.data(), (int16_t*)(response.data.data() + offset), sizeof(int16_t) * len);
				offset += sizeof(int16_t) * len;
				std::get<MtpRangeFormV>(result.FORM).step = std::move(data);
			}
			break;
			case MtpDatatypeCode::ArrayUInt16:
			{
				int32_t len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<uint16_t>(len);
				std::memcpy(data.data(), (uint16_t*)(response.data.data() + offset), sizeof(uint16_t) * len);
				offset += sizeof(uint16_t) * len;
				std::get<MtpRangeFormV>(result.FORM).min = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<uint16_t>(len);
				std::memcpy(data.data(), (uint16_t*)(response.data.data() + offset), sizeof(uint16_t) * len);
				offset += sizeof(uint16_t) * len;
				std::get<MtpRangeFormV>(result.FORM).max = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<uint16_t>(len);
				std::memcpy(data.data(), (uint16_t*)(response.data.data() + offset), sizeof(uint16_t) * len);
				offset += sizeof(uint16_t) * len;
				std::get<MtpRangeFormV>(result.FORM).step = std::move(data);
			}
			break;
			case MtpDatatypeCode::ArrayInt32:
			{
				int32_t len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<int32_t>(len);
				std::memcpy(data.data(), (int32_t*)(response.data.data() + offset), sizeof(int32_t) * len);
				offset += sizeof(int32_t) * len;
				std::get<MtpRangeFormV>(result.FORM).min = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<int32_t>(len);
				std::memcpy(data.data(), (int32_t*)(response.data.data() + offset), sizeof(int32_t) * len);
				offset += sizeof(int32_t) * len;
				std::get<MtpRangeFormV>(result.FORM).max = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<int32_t>(len);
				std::memcpy(data.data(), (int32_t*)(response.data.data() + offset), sizeof(int32_t) * len);
				offset += sizeof(int32_t) * len;
				std::get<MtpRangeFormV>(result.FORM).step = std::move(data);
			}
			break;
			case MtpDatatypeCode::ArrayUInt32:
			{
				int32_t len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<uint32_t>(len);
				std::memcpy(data.data(), (uint32_t*)(response.data.data() + offset), sizeof(uint32_t)* len);
				offset += sizeof(uint32_t) * len;
				std::get<MtpRangeFormV>(result.FORM).min = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<uint32_t>(len);
				std::memcpy(data.data(), (uint32_t*)(response.data.data() + offset), sizeof(uint32_t)* len);
				offset += sizeof(uint32_t) * len;
				std::get<MtpRangeFormV>(result.FORM).max = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<uint32_t>(len);
				std::memcpy(data.data(), (uint32_t*)(response.data.data() + offset), sizeof(uint32_t)* len);
				offset += sizeof(uint32_t) * len;
				std::get<MtpRangeFormV>(result.FORM).step = std::move(data);
			}
			break;
			case MtpDatatypeCode::ArrayInt64:
			{
				int32_t len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<int64_t>(len);
				std::memcpy(data.data(), (int64_t*)(response.data.data() + offset), sizeof(int64_t)* len);
				offset += sizeof(int64_t) * len;
				std::get<MtpRangeFormV>(result.FORM).min = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<int64_t>(len);
				std::memcpy(data.data(), (int64_t*)(response.data.data() + offset), sizeof(int64_t)* len);
				offset += sizeof(int64_t) * len;
				std::get<MtpRangeFormV>(result.FORM).max = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<int64_t>(len);
				std::memcpy(data.data(), (int64_t*)(response.data.data() + offset), sizeof(int64_t)* len);
				offset += sizeof(int64_t) * len;
				std::get<MtpRangeFormV>(result.FORM).step = std::move(data);
			}
			break;
			case MtpDatatypeCode::ArrayUInt64:
			{
				int32_t len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<uint64_t>(len);
				std::memcpy(data.data(), (uint64_t*)(response.data.data() + offset), sizeof(uint64_t)* len);
				offset += sizeof(uint64_t) * len;
				std::get<MtpRangeFormV>(result.FORM).min = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<uint64_t>(len);
				std::memcpy(data.data(), (uint64_t*)(response.data.data() + offset), sizeof(uint64_t)* len);
				offset += sizeof(uint64_t) * len;
				std::get<MtpRangeFormV>(result.FORM).max = std::move(data);

				len = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				data = std::vector<uint64_t>(len);
				std::memcpy(data.data(), (uint64_t*)(response.data.data() + offset), sizeof(uint64_t)* len);
				offset += sizeof(uint64_t) * len;
				std::get<MtpRangeFormV>(result.FORM).step = std::move(data);
			}
			break;
			case MtpDatatypeCode::ArrayInt128:
				throw std::runtime_error("int128 Not Implemented");
				break;
			case MtpDatatypeCode::ArrayUInt128:
				throw std::runtime_error("uint128 Not Implemented");
				break;
			case MtpDatatypeCode::String:
			{
				auto len = *(uint8_t*)(response.data.data() + offset);
				offset += sizeof(uint8_t);
				auto str = std::vector<char16_t>(len);
				std::memcpy(str.data(), (char16_t*)(response.data.data() + offset), sizeof(char16_t)* len);
				offset += sizeof(char16_t) * len;
				std::get<MtpRangeFormV>(result.FORM).min = std::wstring(str.begin(), str.end());

				len = *(uint8_t*)(response.data.data() + offset);
				offset += sizeof(uint8_t);
				str = std::vector<char16_t>(len);
				std::memcpy(str.data(), (char16_t*)(response.data.data() + offset), sizeof(char16_t)* len);
				offset += sizeof(char16_t) * len;
				std::get<MtpRangeFormV>(result.FORM).max = std::wstring(str.begin(), str.end());

				len = *(uint8_t*)(response.data.data() + offset);
				offset += sizeof(uint8_t);
				str = std::vector<char16_t>(len);
				std::memcpy(str.data(), (char16_t*)(response.data.data() + offset), sizeof(char16_t)* len);
				offset += sizeof(char16_t) * len;
				std::get<MtpRangeFormV>(result.FORM).step = std::wstring(str.begin(), str.end());
			}
			break;
			default:
				throw std::runtime_error("Unknown Type");
				break;
		}
	}
	else if (result.FormFlag == MtpFormtypeCode::Enum) {
		uint16_t len = *(uint16_t*)(response.data.data() + offset);
		offset += sizeof(uint16_t);

		switch (result.DataType) {
		case MtpDatatypeCode::Int8:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				form[i] = MtpDatatypeVariant{ *(int8_t*)(response.data.data() + offset) };
				offset += sizeof(int8_t);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::UInt8:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				form[i] = MtpDatatypeVariant{ *(uint8_t*)(response.data.data() + offset) };
				offset += sizeof(uint8_t);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::Int16:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				form[i] = MtpDatatypeVariant{ *(int16_t*)(response.data.data() + offset) };
				offset += sizeof(int16_t);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::UInt16:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				form[i] = MtpDatatypeVariant{ *(uint16_t*)(response.data.data() + offset) };
				offset += sizeof(uint16_t);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::Int32:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				form[i] = MtpDatatypeVariant{ *(int32_t*)(response.data.data() + offset) };
				offset += sizeof(int32_t);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::UInt32:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				form[i] = MtpDatatypeVariant{ *(uint32_t*)(response.data.data() + offset) };
				offset += sizeof(uint32_t);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::Int64:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				form[i] = MtpDatatypeVariant{ *(int64_t*)(response.data.data() + offset) };
				offset += sizeof(int64_t);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::UInt64:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				form[i] = MtpDatatypeVariant{ *(uint64_t*)(response.data.data() + offset) };
				offset += sizeof(uint64_t);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::Int128:
			throw std::runtime_error("int128 Not Implemented");
			break;
		case MtpDatatypeCode::UInt128:
			throw std::runtime_error("uint128 Not Implemented");
			break;
		case MtpDatatypeCode::ArrayInt8:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				int32_t lenarr = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<int8_t>(lenarr);
				std::memcpy(data.data(), (int8_t*)(response.data.data() + offset), sizeof(int8_t) * lenarr);
				offset += sizeof(int8_t) * lenarr;
				form[i] = std::move(data);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::ArrayUInt8:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				int32_t lenarr = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<uint8_t>(lenarr);
				std::memcpy(data.data(), (uint8_t*)(response.data.data() + offset), sizeof(uint8_t) * lenarr);
				offset += sizeof(uint8_t) * lenarr;
				form[i] = std::move(data);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::ArrayInt16:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				int32_t lenarr = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<int16_t>(lenarr);
				std::memcpy(data.data(), (int16_t*)(response.data.data() + offset), sizeof(int16_t) * lenarr);
				offset += sizeof(int16_t) * lenarr;
				form[i] = std::move(data);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::ArrayUInt16:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				int32_t lenarr = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<uint16_t>(lenarr);
				std::memcpy(data.data(), (uint16_t*)(response.data.data() + offset), sizeof(uint16_t) * lenarr);
				offset += sizeof(uint16_t) * lenarr;
				form[i] = std::move(data);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::ArrayInt32:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				int32_t lenarr = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<int32_t>(lenarr);
				std::memcpy(data.data(), (int32_t*)(response.data.data() + offset), sizeof(int32_t) * lenarr);
				offset += sizeof(int32_t) * lenarr;
				form[i] = std::move(data);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::ArrayUInt32:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				int32_t lenarr = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<uint32_t>(lenarr);
				std::memcpy(data.data(), (uint32_t*)(response.data.data() + offset), sizeof(uint32_t) * lenarr);
				offset += sizeof(uint32_t) * lenarr;
				form[i] = std::move(data);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::ArrayInt64:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				int32_t lenarr = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<int64_t>(lenarr);
				std::memcpy(data.data(), (int64_t*)(response.data.data() + offset), sizeof(int64_t) * lenarr);
				offset += sizeof(int64_t) * lenarr;
				form[i] = std::move(data);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::ArrayUInt64:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				int32_t lenarr = *(int32_t*)(response.data.data() + offset);
				offset += sizeof(int32_t);
				auto data = std::vector<uint64_t>(lenarr);
				std::memcpy(data.data(), (uint64_t*)(response.data.data() + offset), sizeof(uint64_t) * lenarr);
				offset += sizeof(uint64_t) * lenarr;
				form[i] = std::move(data);
			}
			result.FORM = std::move(form);
		}
		break;
		case MtpDatatypeCode::ArrayInt128:
			throw std::runtime_error("int128 Not Implemented");
		break;
		case MtpDatatypeCode::ArrayUInt128:
			throw std::runtime_error("uint128 Not Implemented");
		break;
		case MtpDatatypeCode::String:
		{
			MtpEnumFormV form(len);
			for (size_t i = 0; i < len; i++) {
				auto lenstr = *(uint8_t*)(response.data.data() + offset);
				offset += sizeof(uint8_t);
				auto str = std::vector<char16_t>(lenstr);
				std::memcpy(str.data(), (char16_t*)(response.data.data() + offset), sizeof(char16_t) * lenstr);
				offset += sizeof(char16_t) * lenstr;
				form[i] = MtpDatatypeVariant{ std::wstring(str.begin(), str.end()) };
			}
			result.FORM = std::move(form);
		}
		break;
		default:
			throw std::runtime_error("Unknown Type");
			break;
		}
	}

	return result;
}

MtpDatatypeVariant MtpDevice::GetDevicePropValue(uint16_t devicePropCode) {
	mutexDeviceInfo_.lock();
	if (devicePropDataType_.find(devicePropCode) == devicePropDataType_.end()) {
		mutexDeviceInfo_.unlock();
		return GetDevicePropDesc(devicePropCode).CurrentValue; //Get the current value and cache the Type
	}
	uint16_t dataType = devicePropDataType_[devicePropCode];
	mutexDeviceInfo_.unlock();

	MtpParams params;
	params.addUint32(static_cast<uint32_t>(devicePropCode));
	MtpResponse response = SendCommandAndRead(MtpOperationCode::GetDevicePropValue, params);

	if (response.responseCode != MtpResponseCode::OK) {
		throw MtpException(MtpOperationCode::GetDevicePropValue, response.responseCode);
	}

	return GetDevicePropValue_(response, dataType);
}
MtpDatatypeVariant MtpDevice::GetDevicePropValue_(MtpResponse& response, uint16_t dataType) {
	MtpDatatypeVariant result;
	
	switch (dataType) {
		case MtpDatatypeCode::Int8:
			result = *(int8_t*)(response.data.data());
			break;
		case MtpDatatypeCode::UInt8:
			result = *(uint8_t*)(response.data.data());
			break;
		case MtpDatatypeCode::Int16:
			result = *(int16_t*)(response.data.data());
			break;
		case MtpDatatypeCode::UInt16:
			result = *(uint16_t*)(response.data.data());
			break;
		case MtpDatatypeCode::Int32:
			result = *(int32_t*)(response.data.data());
			break;
		case MtpDatatypeCode::UInt32:
			result = *(uint32_t*)(response.data.data());
			break;
		case MtpDatatypeCode::Int64:
			result = *(int64_t*)(response.data.data());
			break;
		case MtpDatatypeCode::UInt64:
			result = *(uint64_t*)(response.data.data());
			break;
		case MtpDatatypeCode::Int128:
			throw std::runtime_error("int128 Not Implemented");
			break;
		case MtpDatatypeCode::UInt128:
			throw std::runtime_error("uint128 Not Implemented");
			break;
		case MtpDatatypeCode::ArrayInt8:
		{
			int32_t len = *(int32_t*)(response.data.data());
			size_t offset = sizeof(int32_t);
			auto data = std::vector<int8_t>(len);
			std::memcpy(data.data(), (int8_t*)(response.data.data() + offset), sizeof(int8_t) * len);
			offset += sizeof(int8_t) * len;
			result = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayUInt8:
		{
			int32_t len = *(int32_t*)(response.data.data());
			size_t offset = sizeof(int32_t);
			auto data = std::vector<uint8_t>(len);
			std::memcpy(data.data(), (uint8_t*)(response.data.data() + offset), sizeof(uint8_t) * len);
			offset += sizeof(uint8_t) * len;
			result = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayInt16:
		{
			int32_t len = *(int32_t*)(response.data.data());
			size_t offset = sizeof(int32_t);
			auto data = std::vector<int16_t>(len);
			std::memcpy(data.data(), (int16_t*)(response.data.data() + offset), sizeof(int16_t) * len);
			offset += sizeof(int16_t) * len;
			result = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayUInt16:
		{
			int32_t len = *(int32_t*)(response.data.data());
			size_t offset = sizeof(int32_t);
			auto data = std::vector<uint16_t>(len);
			std::memcpy(data.data(), (uint16_t*)(response.data.data() + offset), sizeof(uint16_t) * len);
			offset += sizeof(uint16_t) * len;
			result = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayInt32:
		{
			int32_t len = *(int32_t*)(response.data.data());
			size_t offset = sizeof(int32_t);
			auto data = std::vector<int32_t>(len);
			std::memcpy(data.data(), (int32_t*)(response.data.data() + offset), sizeof(int32_t) * len);
			offset += sizeof(int32_t) * len;
			result = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayUInt32:
		{
			int32_t len = *(int32_t*)(response.data.data());
			size_t offset = sizeof(int32_t);
			auto data = std::vector<uint32_t>(len);
			std::memcpy(data.data(), (uint32_t*)(response.data.data() + offset), sizeof(uint32_t) * len);
			offset += sizeof(uint32_t) * len;
			result = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayInt64:
		{
			int32_t len = *(int32_t*)(response.data.data());
			size_t offset = sizeof(int32_t);
			auto data = std::vector<int64_t>(len);
			std::memcpy(data.data(), (int64_t*)(response.data.data() + offset), sizeof(int64_t) * len);
			offset += sizeof(int64_t) * len;
			result = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayUInt64:
		{
			int32_t len = *(int32_t*)(response.data.data());
			size_t offset = sizeof(int32_t);
			auto data = std::vector<uint64_t>(len);
			std::memcpy(data.data(), (uint64_t*)(response.data.data() + offset), sizeof(uint64_t) * len);
			offset += sizeof(uint64_t) * len;
			result = std::move(data);
		}
		break;
		case MtpDatatypeCode::ArrayInt128:
			throw std::runtime_error("int128 Not Implemented");
			break;
		case MtpDatatypeCode::ArrayUInt128:
			throw std::runtime_error("uint128 Not Implemented");
			break;
		case MtpDatatypeCode::String:
		{
			auto len = *(uint8_t*)(response.data.data());
			size_t offset = sizeof(uint8_t);
			auto str = std::vector<char16_t>(len);
			std::memcpy(str.data(), (char16_t*)(response.data.data() + offset), sizeof(char16_t) * len);
			offset += sizeof(char16_t) * len;
			result = std::wstring(str.begin(), str.end());
		}
		break;
		default:
			throw std::runtime_error("Unknown Type");
			break;
	}

	return result;
}

void MtpDevice::SetDevicePropValue(uint16_t devicePropCode, MtpDatatypeVariant data) {
	std::vector<uint8_t> rawdata = SetDevicePropValue_(data);

	MtpParams params;
	params.addUint32(static_cast<uint32_t>(devicePropCode));
	MtpResponse response = SendCommandAndWrite(MtpOperationCode::SetDevicePropValue, params, rawdata);

	if (response.responseCode != MtpResponseCode::OK) {
		throw MtpException(MtpOperationCode::SetDevicePropValue, response.responseCode);
	}

	return;
}
std::vector<uint8_t> MtpDevice::SetDevicePropValue_(MtpDatatypeVariant data) {
	auto rawdata = std::vector<uint8_t>();

	switch (data.index()) {
	case 1: //int8
		rawdata.resize(sizeof(int8_t));
		std::memcpy((int8_t*)rawdata.data(), &std::get<int8_t>(data), sizeof(int8_t));
		break;
	case 2: //uint8
		rawdata.resize(sizeof(uint8_t));
		std::memcpy((uint8_t*)rawdata.data(), &std::get<uint8_t>(data), sizeof(uint8_t));
		break;
	case 3: //int16
		rawdata.resize(sizeof(int16_t));
		std::memcpy((int16_t*)rawdata.data(), &std::get<int16_t>(data), sizeof(int16_t));
		break;
	case 4: //uint16
		rawdata.resize(sizeof(uint16_t));
		std::memcpy((uint16_t*)rawdata.data(), &std::get<uint16_t>(data), sizeof(uint16_t));
		break;
	case 5: //int32
		rawdata.resize(sizeof(int32_t));
		std::memcpy((int32_t*)rawdata.data(), &std::get<int32_t>(data), sizeof(int32_t));
		break;
	case 6: //uint32
		rawdata.resize(sizeof(uint32_t));
		std::memcpy((uint32_t*)rawdata.data(), &std::get<uint32_t>(data), sizeof(uint32_t));
		break;
	case 7: //int64
		rawdata.resize(sizeof(int64_t));
		std::memcpy((int64_t*)rawdata.data(), &std::get<int64_t>(data), sizeof(int32_t));
		break;
	case 8: //uint64
		rawdata.resize(sizeof(uint64_t));
		std::memcpy((uint64_t*)rawdata.data(), &std::get<uint64_t>(data), sizeof(uint32_t));
		break;
	case 9: //int128
		throw std::runtime_error("int128 Not Implemented");
		break;
	case 10: //uint128
		throw std::runtime_error("uint128 Not Implemented");
		break;
	case 11: //array int8
	{
		size_t len = std::get<std::vector<int8_t>>(data).size();
		size_t size = sizeof(int8_t) * len;
		rawdata.resize(size + sizeof(uint32_t));
		std::memcpy((uint32_t*)rawdata.data(), (uint32_t*)&len, sizeof(uint32_t));
		std::memcpy((int8_t*)(rawdata.data() + sizeof(uint32_t)), std::get<std::vector<int8_t>>(data).data(), size);
	}
	break;
	case 12: //array uint8
	{
		size_t len = std::get<std::vector<uint8_t>>(data).size();
		size_t size = sizeof(uint8_t) * len;
		rawdata.resize(size + sizeof(uint32_t));
		std::memcpy((uint32_t*)rawdata.data(), (uint32_t*)&len, sizeof(uint32_t));
		std::memcpy((uint8_t*)(rawdata.data() + sizeof(uint32_t)), std::get<std::vector<uint8_t>>(data).data(), size);
	}
	break;
	case 13: //array int16
	{
		size_t len = std::get<std::vector<int16_t>>(data).size();
		size_t size = sizeof(int16_t) * len;
		rawdata.resize(size + sizeof(uint32_t));
		std::memcpy((uint32_t*)rawdata.data(), (uint32_t*)&len, sizeof(uint32_t));
		std::memcpy((int16_t*)(rawdata.data() + sizeof(uint32_t)), std::get<std::vector<int16_t>>(data).data(), size);
	}
	break;
	case 14: //array uint16
	{
		size_t len = std::get<std::vector<uint16_t>>(data).size();
		size_t size = sizeof(uint16_t) * len;
		rawdata.resize(size + sizeof(uint32_t));
		std::memcpy((uint32_t*)rawdata.data(), (uint32_t*)&len, sizeof(uint32_t));
		std::memcpy((uint16_t*)(rawdata.data() + sizeof(uint32_t)), std::get<std::vector<uint16_t>>(data).data(), size);
	}
	break;
	case 15: //array int32
	{
		size_t len = std::get<std::vector<int32_t>>(data).size();
		size_t size = sizeof(int32_t) * len;
		rawdata.resize(size + sizeof(uint32_t));
		std::memcpy((uint32_t*)rawdata.data(), (uint32_t*)&len, sizeof(uint32_t));
		std::memcpy((int32_t*)(rawdata.data() + sizeof(uint32_t)), std::get<std::vector<int32_t>>(data).data(), size);
	}
	break;
	case 16: //array uint32
	{
		size_t len = std::get<std::vector<uint32_t>>(data).size();
		size_t size = sizeof(uint32_t) * len;
		rawdata.resize(size + sizeof(uint32_t));
		std::memcpy((uint32_t*)rawdata.data(), (uint32_t*)&len, sizeof(uint32_t));
		std::memcpy((uint32_t*)(rawdata.data() + sizeof(uint32_t)), std::get<std::vector<uint32_t>>(data).data(), size);
	}
	break;
	case 17: //array int64
	{
		size_t len = std::get<std::vector<int64_t>>(data).size();
		size_t size = sizeof(int64_t) * len;
		rawdata.resize(size + sizeof(uint32_t));
		std::memcpy((uint32_t*)rawdata.data(), (uint32_t*)&len, sizeof(uint32_t));
		std::memcpy((int64_t*)(rawdata.data() + sizeof(uint32_t)), std::get<std::vector<int64_t>>(data).data(), size);
	}
	break;
	case 18: //aray uint64
	{
		size_t len = std::get<std::vector<uint64_t>>(data).size();
		size_t size = sizeof(uint64_t) * len;
		rawdata.resize(size + sizeof(uint32_t));
		std::memcpy((uint32_t*)rawdata.data(), (uint32_t*)&len, sizeof(uint32_t));
		std::memcpy((uint64_t*)(rawdata.data() + sizeof(uint32_t)), std::get<std::vector<uint64_t>>(data).data(), size);
	}
	break;
	case 19: //array int128
		throw std::runtime_error("int128 Not Implemented");
		break;
	case 20: //array uint128
		throw std::runtime_error("uint128 Not Implemented");
		break;
	case 21: //string
	{
		size_t len = std::get<std::wstring>(data).length() + 1;
		size_t size = sizeof(char16_t) * len;
		rawdata.resize(size + sizeof(uint8_t));
		std::memcpy((uint8_t*)rawdata.data(), (uint8_t*)&len, sizeof(uint8_t));
		std::memcpy((char16_t*)(rawdata.data() + sizeof(uint8_t)), (char16_t*)std::get<std::wstring>(data).c_str(), size);
	}
	break;
	default:
		throw std::runtime_error("Unknown Type");
		break;
	}

	return rawdata;
}

void MtpDevice::SetDevicePropValueTypesafe(uint16_t devicePropCode, MtpDatatypeVariant data) {
	uint16_t dataType = MtpDatatypeCode::Undefined;
	mutexDeviceInfo_.lock();
	if (devicePropDataType_.find(devicePropCode) == devicePropDataType_.end()) {
		mutexDeviceInfo_.unlock();
		dataType = GetDevicePropDesc(devicePropCode).DataType;
	}
	else {
		dataType = devicePropDataType_[devicePropCode];
		mutexDeviceInfo_.unlock();
	}

	MtpDatatypeVariant newdata;
	if (SetDevicePropValueTypesafe_(dataType, data, newdata)) return SetDevicePropValue(devicePropCode, newdata);

	throw MtpException(MtpOperationCode::SetDevicePropValue, MtpResponseCode::Invalid_DeviceProp_Format);
}
bool MtpDevice::SetDevicePropValueTypesafe_(const uint16_t dataType, const MtpDatatypeVariant& data, MtpDatatypeVariant& newdata) {
	switch (dataType) {
	case MtpDatatypeCode::Int8:
		return TryGetAs<int8_t, int16_t, int32_t, int64_t>(data, newdata);
		break;
	case MtpDatatypeCode::Int16:
		return TryGetAs<int16_t, int8_t, int32_t, int64_t>(data, newdata);
		break;
	case MtpDatatypeCode::Int32:
		return TryGetAs<int32_t, int8_t, int16_t, int64_t>(data, newdata);
		break;
	case MtpDatatypeCode::Int64:
		return TryGetAs<int64_t, int8_t, int16_t, int32_t>(data, newdata);
		break;
	case MtpDatatypeCode::UInt8:
		return TryGetAs<uint8_t, uint16_t, uint32_t, uint64_t>(data, newdata);
		break;
	case MtpDatatypeCode::UInt16:
		return TryGetAs<uint16_t, uint8_t, uint32_t, uint64_t>(data, newdata);
		break;
	case MtpDatatypeCode::UInt32:
		return TryGetAs<uint32_t, uint8_t, uint16_t, uint64_t>(data, newdata);
		break;
	case MtpDatatypeCode::UInt64:
		return TryGetAs<uint64_t, uint8_t, uint16_t, uint32_t>(data, newdata);
		break;
	case MtpDatatypeCode::ArrayInt8:
		return TryGetAsArray<int8_t, int16_t, int32_t, int64_t>(data, newdata);
		break;
	case MtpDatatypeCode::ArrayInt16:
		return TryGetAsArray<int16_t, int8_t, int32_t, int64_t>(data, newdata);
		break;
	case MtpDatatypeCode::ArrayInt32:
		return TryGetAsArray<int32_t, int8_t, int16_t, int64_t>(data, newdata);
		break;
	case MtpDatatypeCode::ArrayInt64:
		return TryGetAsArray<int64_t, int8_t, int16_t, int32_t>(data, newdata);
		break;
	case MtpDatatypeCode::ArrayUInt8:
		return TryGetAsArray<uint8_t, uint16_t, uint32_t, uint64_t>(data, newdata);
		break;
	case MtpDatatypeCode::ArrayUInt16:
		return TryGetAsArray<uint16_t, uint8_t, uint32_t, uint64_t>(data, newdata);
		break;
	case MtpDatatypeCode::ArrayUInt32:
		return TryGetAsArray<uint32_t, uint8_t, uint16_t, uint64_t>(data, newdata);
		break;
	case MtpDatatypeCode::ArrayUInt64:
		return TryGetAsArray<uint64_t, uint8_t, uint16_t, uint32_t>(data, newdata);
		break;
	case MtpDatatypeCode::String:
		if (auto* d = std::get_if<std::wstring>(&data)) {
			newdata = *d;
			return true;
		}
		break;
	}

	return false;
}