#include "nek_wpd.hpp"

#include "../nek_mtp_enum.hpp"
#include "../nek_mtp_except.hpp"

#include <algorithm>
#include <future>



namespace nek::mtp::backend::wpd {

#pragma region WpdMtpTransport

	WpdMtpTransport::WpdMtpTransport(const std::wstring& devicePath) {
		this->devicePath_ = devicePath;
		running_ = false;
		command_ = nullptr;
		eventCookie_ = nullptr;
		eventManager_ = new WpdMtpEventManager();
		std::lock_guard lock(eventManager_->eventMutex_);
		eventManager_->eventCallbacks_.clear();
		eventManager_->eventNextId_ = 0;
	}

	WpdMtpTransport::~WpdMtpTransport() {
		unsubscribe();
		disconnect();
		eventManager_.Release();
	}

	std::unique_ptr<IMtpTransport> WpdMtpTransport::clone() const {
		return std::make_unique<WpdMtpTransport>(this->devicePath_);
	}



	void WpdMtpTransport::connect() {
		if (running_) return; //TODO: Throw an error like device already connected

		running_ = true;
		commandThread_ = std::thread(&WpdMtpTransport::commandLoop, this);

		std::unique_lock lk(commandMutex_);
		commandCV_.wait(lk, [this] { return this->isConnected(); });
		lk.unlock();

		std::lock_guard lock(eventManager_->eventMutex_);
		if (eventManager_->eventCallbacks_.size() > 0) startEventCapture();
	}

	void WpdMtpTransport::disconnect() {
		if (!running_) return; //TODO: Throw an error like device already disconnected

		std::lock_guard lock(eventManager_->eventMutex_);
		if (eventManager_->eventCallbacks_.size() == 0) stopEventCapture();

		running_ = false;
		commandCV_.notify_all();
		commandThread_.join();
	}

	bool WpdMtpTransport::isConnected() const {
		if (!running_) return false;
		if (device_ == nullptr) return false;
		return true;
	}



	MtpResponse WpdMtpTransport::sendCommand(uint16_t operationCode, const std::vector<uint32_t>& params) {
		std::unique_lock lk(commandMutex_);
		commandCV_.wait(lk, [this] { return !this->running_ || this->command_ == nullptr; });

		auto p = std::promise<MtpResponse>();
		auto f = p.get_future();
		command_ = new std::function([this, &p, operationCode, params] {
			try {
				p.set_value(this->sendCommand_(operationCode, params));
			}
			catch (...) {
				p.set_exception(std::current_exception());
			}
			});

		lk.unlock();
		commandCV_.notify_all();

		return f.get();
	}

	MtpResponse WpdMtpTransport::sendCommandAndRead(uint16_t operationCode, const std::vector<uint32_t>& params) {
		std::unique_lock lk(commandMutex_);
		commandCV_.wait(lk, [this] { return !this->running_ || this->command_ == nullptr; });

		auto p = std::promise<MtpResponse>();
		auto f = p.get_future();
		command_ = new std::function([this, &p, operationCode, params] {
			try {
				p.set_value(this->sendCommandAndRead_(operationCode, params));
			}
			catch (...) {
				p.set_exception(std::current_exception());
			}
			});

		lk.unlock();
		commandCV_.notify_all();

		return f.get();
	}

	MtpResponse WpdMtpTransport::sendCommandAndWrite(uint16_t operationCode, const std::vector<uint32_t>& params, const std::vector<uint8_t>& data) {
		std::unique_lock lk(commandMutex_);
		commandCV_.wait(lk, [this] { return !this->running_ || this->command_ == nullptr; });

		auto p = std::promise<MtpResponse>();
		auto f = p.get_future();
		command_ = new std::function([this, &p, operationCode, params, data] {
			try {
				p.set_value(this->sendCommandAndWrite_(operationCode, params, data));
			}
			catch (...) {
				p.set_exception(std::current_exception());
			}
			});

		lk.unlock();
		commandCV_.notify_all();

		return f.get();
	}



	void WpdMtpTransport::initCom() {
		//Com context
		HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (hr == RPC_E_CHANGED_MODE) {
			throw WpdMtpDeviceException(MtpExPhase::COM_INIT, hr);
		}

		//Device Client
		if (deviceClient_ != nullptr) return; //Already initialized
		hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceClient_));
		if (FAILED(hr)) {
			throw WpdMtpDeviceException(MtpExPhase::DEVICECLIENT_INIT, hr);
		}
		deviceClient_->SetStringValue(WPD_CLIENT_NAME, CLIENT_NAME);
		deviceClient_->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, CLIENT_MAJOR_VER);
		deviceClient_->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, CLIENT_MINOR_VER);
		deviceClient_->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, CLIENT_REVISION);
		deviceClient_->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ | GENERIC_WRITE);
	}

	void WpdMtpTransport::initDevice() {
		if (device_ != nullptr) return; //Already connected

		HRESULT hr = CoCreateInstance(CLSID_PortableDeviceFTM, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&device_));
		if (FAILED(hr)) {
			throw WpdMtpDeviceException(MtpExPhase::DEVICE_INIT, hr);
		}

		hr = device_->Open(devicePath_.c_str(), deviceClient_);
		if (FAILED(hr)) {
			device_.Release();
			throw WpdMtpDeviceException(MtpExPhase::DEVICE_INIT, hr);
		}
	}

	void WpdMtpTransport::commandLoop() {
		initCom();
		initDevice();
		commandCV_.notify_all();

		while (running_) {
			commandMutex_.lock();
			if (command_ != nullptr) (*command_)();
			delete command_;
			command_ = nullptr;
			commandMutex_.unlock();

			std::unique_lock lk(commandMutex_);
			commandCV_.wait(lk, [this] { return !this->running_ || this->command_ != nullptr; });
			lk.unlock();
		}

		device_.Release();
		deviceClient_.Release();
		CoUninitialize();
	}


	void WpdMtpTransport::startEventCapture() {
		if (eventCookie_ == nullptr) {
			std::unique_lock lk(commandMutex_);
			commandCV_.wait(lk, [this] { return !this->running_ || this->command_ == nullptr; });

			auto p = std::promise<void>();
			auto f = p.get_future();
			command_ = new std::function([this, &p] {
				try {
					this->device_->Advise(0, this->eventManager_, nullptr, &this->eventCookie_);
					p.set_value();
				}
				catch (...) {
					p.set_exception(std::current_exception());
				}
				});

			lk.unlock();
			commandCV_.notify_all();

			f.get();
		}
	}

	void WpdMtpTransport::stopEventCapture() {
		if (eventCookie_ != nullptr) {
			if (running_) {
				std::unique_lock lk(commandMutex_);
				commandCV_.wait(lk, [this] { return !this->running_ || this->command_ == nullptr; });

				auto p = std::promise<void>();
				auto f = p.get_future();
				command_ = new std::function([this, &p] {
					try {
						this->device_->Unadvise(this->eventCookie_);
						this->eventCookie_ = nullptr;
						p.set_value();
					}
					catch (...) {
						p.set_exception(std::current_exception());
					}
					});

				lk.unlock();
				commandCV_.notify_all();

				return f.get();
			}
			else {
				this->eventCookie_ = nullptr;
			}
		}
	}


	MtpResponse WpdMtpTransport::sendCommand_(uint16_t operationCode, const std::vector<uint32_t>& params) {
		MtpResponse result;
		HRESULT hr;

		CComPtr<IPortableDeviceValues> command;
		hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&command));
		if (FAILED(hr)) {
			throw WpdMtpDeviceException(MtpExPhase::OPERATION_INIT, hr);
		}

		// Set command category and ID
		command->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.fmtid);
		command->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE.pid);

		// Set operation code and parameters
		command->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, operationCode);

		CComPtr<IPortableDevicePropVariantCollection> parametersCollection;
		PROPVARIANT pv;
		hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&parametersCollection));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create Prop Variant Collection: " + hr);
		}
		for (auto param : params) {
			hr = InitPropVariantFromUInt32(param, &pv);
			if (FAILED(hr)) {
				throw std::runtime_error("Failed to create Prop Variant: " + hr);
			}
			parametersCollection->Add(&pv);
			PropVariantClear(&pv);
		}
		command->SetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, parametersCollection);
		parametersCollection.Release();

		// Send command
		CComPtr<IPortableDeviceValues> commandResult;
		hr = device_->SendCommand(0, command, &commandResult);
		if (FAILED(hr)) {
			command.Release();
			throw WpdMtpDeviceException(MtpExPhase::OPERATION_SEND, hr);
		}

		// Extract response code
		ULONG tempCode;
		hr = commandResult->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &tempCode);
		result.responseCode = static_cast<uint16_t>(tempCode);
		if (FAILED(hr)) {
			command.Release();
			commandResult.Release();
			throw WpdMtpDeviceException(MtpExPhase::OPERATION_RESPONSE, hr);
		}

		// Extract response parameters
		hr = commandResult->GetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_RESPONSE_PARAMS, &parametersCollection);
		if (FAILED(hr)) {
			command.Release();
			commandResult.Release();
			throw WpdMtpDeviceException(MtpExPhase::OPERATION_RESPONSE, hr);
		}

		DWORD nbParams = 0;
		PropVariantInit(&pv);
		parametersCollection->GetCount(&nbParams);
		for (DWORD i = 0; i < nbParams; i++) {
			parametersCollection->GetAt(i, &pv);
			result.parameters.push_back(pv.uintVal);
		}
		PropVariantClear(&pv);

		parametersCollection.Release();
		command.Release();
		commandResult.Release();

		return result;
	}

	MtpResponse WpdMtpTransport::sendCommandAndRead_(uint16_t operationCode, const std::vector<uint32_t>& params) {
		MtpResponse result;
		HRESULT hr;

		CComPtr<IPortableDeviceValues> command;
		hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&command));
		if (FAILED(hr)) {
			throw WpdMtpDeviceException(MtpExPhase::OPERATIONREAD_INIT, hr);
		}

		// Set command category and ID
		command->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_READ.fmtid);
		command->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_READ.pid);

		// Set operation code and parameters
		command->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, operationCode);

		CComPtr<IPortableDevicePropVariantCollection> parametersCollection;
		PROPVARIANT pv;
		hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&parametersCollection));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create Prop Variant Collection: " + hr);
		}
		for (auto param : params) {
			hr = InitPropVariantFromUInt32(param, &pv);
			if (FAILED(hr)) {
				throw std::runtime_error("Failed to create Prop Variant: " + hr);
			}
			parametersCollection->Add(&pv);
			PropVariantClear(&pv);
		}
		command->SetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, parametersCollection);
		parametersCollection.Release();

		// Send command
		CComPtr<IPortableDeviceValues> commandResult;
		hr = device_->SendCommand(0, command, &commandResult);
		if (FAILED(hr)) {
			command.Release();
			throw WpdMtpDeviceException(MtpExPhase::OPERATIONREAD_SEND, hr);
		}

		LPWSTR context;
		hr = commandResult->GetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, &context);
		if (FAILED(hr)) {
			command.Release();
			commandResult.Release();
			throw WpdMtpDeviceException(MtpExPhase::OPERATIONREAD_RESPONSE, hr);
		}

		ULONG totalSize;
		hr = commandResult->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_TOTAL_DATA_SIZE, &totalSize);
		if (FAILED(hr)) {
			command.Release();
			commandResult.Release();
			throw WpdMtpDeviceException(MtpExPhase::OPERATIONREAD_RESPONSE, hr);
		}

		ULONG optimalSize;
		hr = commandResult->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPTIMAL_TRANSFER_BUFFER_SIZE, &optimalSize);
		if (FAILED(hr)) {
			command.Release();
			commandResult.Release();
			throw WpdMtpDeviceException(MtpExPhase::OPERATIONREAD_RESPONSE, hr);
		}

		command.Release();
		commandResult.Release();


		// Start Data Transfert
		hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&command));
		if (FAILED(hr)) {
			CoTaskMemFree(context);
			throw WpdMtpDeviceException(MtpExPhase::DATAREAD_INIT, hr);
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
			hr = device_->SendCommand(0, command, &commandResult);
			if (FAILED(hr)) {
				CoTaskMemFree(context);
				command.Release();
				commandResult.Release();
				throw WpdMtpDeviceException(MtpExPhase::DATAREAD_SEND, hr);
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
			throw WpdMtpDeviceException(MtpExPhase::ENDREAD_INIT, hr);
		}
		command->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.fmtid);
		command->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.pid);
		command->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, context);
		hr = device_->SendCommand(0, command, &commandResult);
		if (FAILED(hr)) {
			CoTaskMemFree(context);
			command.Release();
			throw WpdMtpDeviceException(MtpExPhase::ENDREAD_SEND, hr);
		}

		// Extract response code
		ULONG tempCode;
		hr = commandResult->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &tempCode);
		result.responseCode = static_cast<uint16_t>(tempCode);
		if (FAILED(hr)) {
			CoTaskMemFree(context);
			command.Release();
			commandResult.Release();
			throw WpdMtpDeviceException(MtpExPhase::ENDREAD_RESPONSE, hr);
		}

		// Extract response parameters
		hr = commandResult->GetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_RESPONSE_PARAMS, &parametersCollection);
		if (FAILED(hr)) {
			CoTaskMemFree(context);
			command.Release();
			commandResult.Release();
			throw WpdMtpDeviceException(MtpExPhase::ENDREAD_RESPONSE, hr);
		}

		DWORD nbParams = 0;
		PropVariantInit(&pv);
		parametersCollection->GetCount(&nbParams);
		for (DWORD i = 0; i < nbParams; i++) {
			parametersCollection->GetAt(i, &pv);
			result.parameters.push_back(pv.uintVal);
		}
		PropVariantClear(&pv);

		parametersCollection.Release();
		CoTaskMemFree(context);
		command.Release();
		commandResult.Release();
		return result;
	}

	MtpResponse WpdMtpTransport::sendCommandAndWrite_(uint16_t operationCode, const std::vector<uint32_t>& params, const std::vector<uint8_t>& data) {
		MtpResponse result;
		HRESULT hr;

		CComPtr<IPortableDeviceValues> command;
		hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&command));
		if (FAILED(hr)) {
			throw WpdMtpDeviceException(MtpExPhase::OPERATIONWRITE_INIT, hr);
		}

		// Set command category and ID
		command->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_WRITE.fmtid);
		command->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_WRITE.pid);

		// Set operation code and parameters
		command->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE, operationCode);

		CComPtr<IPortableDevicePropVariantCollection> parametersCollection;
		PROPVARIANT pv;
		hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&parametersCollection));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create Prop Variant Collection: " + hr);
		}
		for (auto param : params) {
			hr = InitPropVariantFromUInt32(param, &pv);
			if (FAILED(hr)) {
				throw std::runtime_error("Failed to create Prop Variant: " + hr);
			}
			parametersCollection->Add(&pv);
			PropVariantClear(&pv);
		}
		command->SetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, parametersCollection);
		parametersCollection.Release();

		command->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_TOTAL_DATA_SIZE, (ULONG)data.size());

		// Send command
		CComPtr<IPortableDeviceValues> commandResult;
		hr = device_->SendCommand(0, command, &commandResult);
		if (FAILED(hr)) {
			command.Release();
			commandResult.Release();
			throw WpdMtpDeviceException(MtpExPhase::OPERATIONWRITE_SEND, hr);
		}

		LPWSTR context;
		hr = commandResult->GetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, &context);
		if (FAILED(hr)) {
			command.Release();
			commandResult.Release();
			throw WpdMtpDeviceException(MtpExPhase::OPERATIONWRITE_RESPONSE, hr);
		}

		ULONG optimalSize;
		hr = commandResult->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPTIMAL_TRANSFER_BUFFER_SIZE, &optimalSize);
		if (FAILED(hr)) {
			command.Release();
			commandResult.Release();
			CoTaskMemFree(context);
			throw WpdMtpDeviceException(MtpExPhase::OPERATIONWRITE_RESPONSE, hr);
		}

		command.Release();
		commandResult.Release();


		// Start Data Transfert
		DWORD offset = 0;
		while (offset < data.size()) {
			hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&command));
			if (FAILED(hr)) {
				CoTaskMemFree(context);
				throw WpdMtpDeviceException(MtpExPhase::DATAWRITE_INIT, hr);
			}

			command->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_WRITE_DATA.fmtid);
			command->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_WRITE_DATA.pid);
			command->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, context);

			optimalSize = std::min(optimalSize, (ULONG)(data.size() - offset));
			command->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_NUM_BYTES_TO_WRITE, optimalSize);
			command->SetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, (BYTE *)data.data() + offset, optimalSize);

			hr = device_->SendCommand(0, command, &commandResult);
			if (FAILED(hr)) {
				command.Release();
				commandResult.Release();
				CoTaskMemFree(context);
				throw WpdMtpDeviceException(MtpExPhase::DATAWRITE_SEND, hr);
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
			throw WpdMtpDeviceException(MtpExPhase::ENDWRITE_INIT, hr);
		}

		command->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.fmtid);
		command->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.pid);
		command->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, context);
		hr = device_->SendCommand(0, command, &commandResult);
		if (FAILED(hr)) {
			command.Release();
			CoTaskMemFree(context);
			throw WpdMtpDeviceException(MtpExPhase::ENDWRITE_SEND, hr);
		}

		// Extract response code
		ULONG tempCode;
		hr = commandResult->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &tempCode);
		result.responseCode = static_cast<uint16_t>(tempCode);
		if (FAILED(hr)) {
			command.Release();
			commandResult.Release();
			CoTaskMemFree(context);
			throw WpdMtpDeviceException(MtpExPhase::ENDWRITE_RESPONSE, hr);
		}

		// Extract response parameters
		hr = commandResult->GetIPortableDevicePropVariantCollectionValue(WPD_PROPERTY_MTP_EXT_RESPONSE_PARAMS, &parametersCollection);
		if (FAILED(hr)) {
			command.Release();
			commandResult.Release();
			CoTaskMemFree(context);
			throw WpdMtpDeviceException(MtpExPhase::ENDWRITE_RESPONSE, hr);
		}
		
		DWORD nbParams = 0;
		PropVariantInit(&pv);
		parametersCollection->GetCount(&nbParams);
		for (DWORD i = 0; i < nbParams; i++) {
			parametersCollection->GetAt(i, &pv);
			result.parameters.push_back(pv.uintVal);
		}
		PropVariantClear(&pv);

		parametersCollection.Release();
		command.Release();
		commandResult.Release();
		CoTaskMemFree(context);
		return result;
	}



	size_t WpdMtpTransport::subscribe(Handler const& eventCallback) {
		if (!running_) throw MtpDeviceException(DEVICE_NOT_CONNECTED, DEVICE_DISCONNECTED); //TODO: Throw an error like device not connected
		std::lock_guard lock(eventManager_->eventMutex_);
		eventManager_->eventCallbacks_[eventManager_->eventNextId_] = eventCallback;

		startEventCapture();

		return eventManager_->eventNextId_++;
	}

	void WpdMtpTransport::unsubscribe(size_t id) {
		std::lock_guard lock(eventManager_->eventMutex_);
		if (eventManager_->eventCallbacks_.contains(id)) eventManager_->eventCallbacks_.erase(id);

		if (eventManager_->eventCallbacks_.size() == 0) stopEventCapture();
	}

	void WpdMtpTransport::unsubscribe() {
		std::lock_guard lock(eventManager_->eventMutex_);
		eventManager_->eventCallbacks_.clear();

		stopEventCapture();
	}



	HRESULT __stdcall WpdMtpTransport::WpdMtpEventManager::QueryInterface(REFIID riid, void** ppvObject) {
		static const QITAB qitab[] = { QITABENT(WpdMtpEventManager, IPortableDeviceEventCallback), { }, };
		return QISearch(this, qitab, riid, ppvObject);
	}

	ULONG __stdcall WpdMtpTransport::WpdMtpEventManager::AddRef(void) {
		return InterlockedIncrement(&ref_);
	}

	ULONG __stdcall WpdMtpTransport::WpdMtpEventManager::Release(void) {
		ULONG ref = _InterlockedDecrement(&ref_);
		if (ref == 0) {
			delete this;
		}

		return ref;
	}

	HRESULT __stdcall WpdMtpTransport::WpdMtpEventManager::OnEvent(IPortableDeviceValues* pEventParameters) {
		MtpEvent event;
		event.eventCode = MtpEventCode::Undefined;
		event.parameters = std::vector<uint32_t>(0);

		for (auto& [id, callback] : eventCallbacks_) {
			if (callback) callback(event);
		}

		return S_OK; //TODO: implement a way to fix the mapping of WPD events
	}

#pragma endregion



#pragma region WpdMtpBackendProvider

	std::unique_ptr<IMtpTransport> nek::mtp::backend::wpd::WpdMtpBackendProvider::tryCreateTransport(const MtpConnectionInfo& connectionInfo) {
		if (!connectionInfo.usbPath.has_value()) return nullptr;
		auto devices = listDevices();
		auto device = std::find_if(devices.begin(), devices.end(), [&connectionInfo](const std::pair<MtpConnectionInfo, std::unique_ptr<IMtpTransport>>& d) { return d.first.usbPath.value() == connectionInfo.usbPath.value(); });
		if (device == devices.end()) return nullptr;

		return std::move(device->second);
	}

	std::vector<std::pair<MtpConnectionInfo, std::unique_ptr<IMtpTransport>>> WpdMtpBackendProvider::listDevices() {
		auto result = std::vector<std::pair<MtpConnectionInfo, std::unique_ptr<IMtpTransport>>>();

		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		bool weInitialized = (hr == S_OK);

		//Device Manager
		CComPtr<IPortableDeviceManager> deviceManager;
		hr = CoCreateInstance(CLSID_PortableDeviceManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceManager));
		if (SUCCEEDED(hr)) {
			DWORD devicesNb = 0;
			PWSTR* devices = nullptr;
			HRESULT hr;

			//Update WPD devices list
			deviceManager->RefreshDeviceList();

			//Get the number of WPD devices
			hr = deviceManager->GetDevices(NULL, &devicesNb);
			if (FAILED(hr)) {
				throw WpdMtpDeviceException(MtpExPhase::MANAGER_DEVICELIST, hr);
			}

			//At least one device
			if (devicesNb > 0) {
				devices = new PWSTR[devicesNb]();
				HRESULT hr = deviceManager->GetDevices(devices, &devicesNb);
				if (FAILED(hr)) {
					delete[] devices;
					throw WpdMtpDeviceException(MtpExPhase::MANAGER_DEVICELIST, hr);
				}

				for (DWORD i = 0; i < devicesNb; i++) {
					std::pair<MtpConnectionInfo, std::unique_ptr<IMtpTransport>> info;
					info.first.usbPath = devices[i];
					info.second = std::make_unique<WpdMtpTransport>(devices[i]);
					result.push_back(std::move(info));

					CoTaskMemFree(devices[i]);
				}

				delete[] devices;
			}

			deviceManager.Release();
		}

		if (weInitialized) CoUninitialize();

		return result;
	}

	size_t WpdMtpBackendProvider::countDevices() {
		size_t result = 0;

		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		bool weInitialized = (hr == S_OK);

		//Device Manager
		CComPtr<IPortableDeviceManager> deviceManager;
		hr = CoCreateInstance(CLSID_PortableDeviceManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceManager));
		if (SUCCEEDED(hr)) {
			DWORD devicesNb = 0;
			PWSTR* devices = nullptr;
			HRESULT hr;

			//Update WPD devices list
			deviceManager->RefreshDeviceList();

			//Get the number of WPD devices
			hr = deviceManager->GetDevices(NULL, &devicesNb);
			if (FAILED(hr)) {
				throw WpdMtpDeviceException(MtpExPhase::MANAGER_DEVICELIST, hr);
			}

			result = devicesNb;

			deviceManager.Release();
		}

		if (weInitialized) CoUninitialize();

		return result;
	}

#pragma endregion



#pragma region WpdMtpDeviceException

	WpdMtpDeviceException::WpdMtpDeviceException(MtpExPhase phase, HRESULT hr) : MtpDeviceException(phase, computeCode(phase, hr)) {}

	MtpExCode WpdMtpDeviceException::computeCode(MtpExPhase phase, HRESULT hr) {
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

#pragma endregion

}