#include "nek_mtp.hpp"
#include "backend/nek_wpd.hpp"

#include <algorithm>
#include <stdexcept>



using namespace nek::mtp;



#pragma region MtpDevice

MtpDevice::MtpDevice(std::unique_ptr<backend::IMtpTransport> backend, bool autoConnect) : backend_(std::move(backend)), backendCallbackId_(std::nullopt) {
	std::lock_guard lock(eventMutex_);
	eventCallbacks_.clear();
	eventNextId_ = 0;

	if (autoConnect) Connect();
}

MtpDevice::MtpDevice(const backend::MtpConnectionInfo& connectionInfo, bool autoConnect) {
	auto backends = MtpManager().tryCreateTransport(connectionInfo);
	if (backends.size() == 0) throw MtpDeviceException(MtpExPhase::DEVICE_NOT_CONNECTED, MtpExCode::DEVICE_DISCONNECTED); //TODO chnage to a more explicit error
	backend_ = std::move(backends[0]);
	backendCallbackId_ = std::nullopt;

	std::lock_guard lock(eventMutex_);
	eventCallbacks_.clear();
	eventNextId_ = 0;

	if (autoConnect) Connect();
}

nek::mtp::MtpDevice::MtpDevice(MtpDevice&& other) noexcept {
	std::lock_guard lock(eventMutex_);
	backend_ = std::move(other.backend_);
	backendCallbackId_ = std::nullopt;

	eventCallbacks_ = std::move(other.eventCallbacks_);
	eventNextId_ = other.eventNextId_;

	if (isConnected()) Connect();
}

MtpDevice::~MtpDevice() {
	Disconnect();
};



bool MtpDevice::isConnected() const {
	if (this == nullptr) return false;
	if (!backend_) return false;
	return backend_->isConnected();
}

void MtpDevice::Connect() {
	if (!backend_->isConnected()) backend_->connect();
	if (!backendCallbackId_.has_value()) backendCallbackId_ = backend_->subscribe([this](const MtpEvent& event) { return this->OnEvent(event); });
}

void MtpDevice::Disconnect() {
	if (backend_ == nullptr || !backend_->isConnected()) return; //Already disconnected

	if (backendCallbackId_.has_value()) backend_->unsubscribe(backendCallbackId_.value());
	backendCallbackId_ = std::nullopt;
	backend_->disconnect();
	
	OnEvent(nek::mtp::MtpEvent(MtpEventCode::DeviceInfoChanged)); // Notify Disconnection: DeviceInfoChanged
}


MtpResponse MtpDevice::SendCommand(uint16_t operationCode, const std::vector<uint32_t>& params) {
	if (!isConnected()) {
		throw MtpDeviceException(MtpExPhase::DEVICE_NOT_CONNECTED, MtpExCode::DEVICE_DISCONNECTED);
	}
	return backend_->sendCommand(operationCode, params);
}
MtpResponse MtpDevice::SendCommandAndRead(uint16_t operationCode, const std::vector<uint32_t>& params) {
	if (!isConnected()) {
		throw MtpDeviceException(MtpExPhase::DEVICE_NOT_CONNECTED, MtpExCode::DEVICE_DISCONNECTED);
	}
	return backend_->sendCommandAndRead(operationCode, params);
}
MtpResponse MtpDevice::SendCommandAndWrite(uint16_t operationCode, const std::vector<uint32_t>& params, std::vector<uint8_t> data) {
	if (!isConnected()) {
		throw MtpDeviceException(MtpExPhase::DEVICE_NOT_CONNECTED, MtpExCode::DEVICE_DISCONNECTED);
	}
	return backend_->sendCommandAndWrite(operationCode, params, data);
}


size_t MtpDevice::RegisterCallback(Callback const& callback) {
	std::lock_guard lock(eventMutex_);
	eventCallbacks_[eventNextId_] = std::move(callback);
	return eventNextId_++;
}

void MtpDevice::UnregisterCallback(size_t id) {
	std::lock_guard lock(eventMutex_);
	if (eventCallbacks_.contains(id)) eventCallbacks_.erase(id);
}

void MtpDevice::OnEvent(const MtpEvent& event) {
	MtpEvent eventCopy = event;
	std::async(std::launch::async, [this, eventCopy] {
		std::lock_guard lock(eventMutex_);
		for (auto& [id, callBack] : eventCallbacks_) {
			if (callBack) callBack(eventCopy);
		}
	});
}



MtpDeviceInfoDS MtpDevice::GetDeviceInfo() {
	std::vector<uint32_t> params;
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
	std::vector<uint32_t> params = { handle };
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
	std::vector<uint32_t> params = { static_cast<uint32_t>(devicePropCode) };
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

	std::vector<uint32_t> params = { static_cast<uint32_t>(devicePropCode) };
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

	std::vector<uint32_t> params = { static_cast<uint32_t>(devicePropCode) };
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

#pragma endregion



#pragma region MtpManager

MtpManager::MtpManager() : backends_(0) {
	MtpManager::registerBackend(std::make_unique<backend::wpd::WpdMtpBackendProvider>());
}

void MtpManager::registerBackend(std::unique_ptr<backend::IMtpBackendProvider> backend) {
	backends_.push_back(std::move(backend));
}

std::vector<std::unique_ptr<backend::IMtpTransport>> MtpManager::tryCreateTransport(const backend::MtpConnectionInfo& connectionInfo) {
	std::vector<std::unique_ptr<backend::IMtpTransport>> result;

	for (const auto& b : backends_) {
		auto t = b->tryCreateTransport(connectionInfo);
		if (t != nullptr) result.push_back(std::move(t));
	}

	return result;
}

std::vector<std::pair<backend::MtpConnectionInfo, std::unique_ptr<backend::IMtpTransport>>> MtpManager::listAllDevices() {
	auto result = std::vector<std::pair<backend::MtpConnectionInfo, std::unique_ptr<backend::IMtpTransport>>>();
	for (auto& b : backends_) {
		for (auto& d : b->listDevices()) {
			result.push_back(std::move(d));
		}
	}
	return result;
}

std::vector<std::pair<backend::MtpConnectionInfo, MtpDevice>> MtpManager::getAllDevices() {
	auto result = std::vector<std::pair<backend::MtpConnectionInfo, MtpDevice>>();
	for (auto& b : backends_) {
		for (auto& d : b->listDevices()) {
			result.push_back(std::make_pair(d.first, std::move(MtpDevice(std::move(d.second), false))));
		}
	}
	return result;
}

size_t MtpManager::countAllDevices() {
	size_t result = 0;
	for (auto& b : backends_) {
		result += b->countDevices();
	}
	return result;
}

#pragma endregion