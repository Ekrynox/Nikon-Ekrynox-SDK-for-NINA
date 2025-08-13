#include "pch.h"

#include "Nikon_Struct.h"

#include <assert.h>


using namespace NEKCS;


//NikonDeviceInfoDS
NikonDeviceInfoDS::NikonDeviceInfoDS() {
	StandardVersion = 0;
	VendorExtensionID = 0;
	VendorExtensionVersion = 0;
	VendorExtensionDesc = gcnew System::String("");
	FunctionalMode = 0;
	OperationsSupported = gcnew System::Collections::Generic::List<NikonMtpOperationCode>();
	EventsSupported = gcnew System::Collections::Generic::List<NikonMtpEventCode>();
	DevicePropertiesSupported = gcnew System::Collections::Generic::List<NikonMtpDevicePropCode>();
	CaptureFormats = gcnew System::Collections::Generic::List<System::UInt16>();
	ImageFormats = gcnew System::Collections::Generic::List<System::UInt16>();
	Manufacture = gcnew System::String("");
	Model = gcnew System::String("");
	DeviceVersion = gcnew System::String("");
	SerialNumber = gcnew System::String("");
};

NikonDeviceInfoDS::NikonDeviceInfoDS(const nek::mtp::MtpDeviceInfoDS& native) {
	StandardVersion = native.StandardVersion;
	VendorExtensionID = native.VendorExtensionID;
	VendorExtensionVersion = native.VendorExtensionVersion;
	VendorExtensionDesc = gcnew System::String(native.VendorExtensionDesc.c_str());
	FunctionalMode = native.FunctionalMode;

	OperationsSupported = gcnew System::Collections::Generic::List<NikonMtpOperationCode>();
	for (auto el : native.OperationsSupported) {
		OperationsSupported->Add((NikonMtpOperationCode)el);
	}
	EventsSupported = gcnew System::Collections::Generic::List<NikonMtpEventCode>();
	for (auto el : native.EventsSupported) {
		EventsSupported->Add((NikonMtpEventCode)el);
	}
	DevicePropertiesSupported = gcnew System::Collections::Generic::List<NikonMtpDevicePropCode>();
	for (auto el : native.DevicePropertiesSupported) {
		DevicePropertiesSupported->Add((NikonMtpDevicePropCode)el);
	}
	CaptureFormats = gcnew System::Collections::Generic::List<System::UInt16>();
	for (auto el : native.CaptureFormats) {
		CaptureFormats->Add(el);
	}
	ImageFormats = gcnew System::Collections::Generic::List<System::UInt16>();
	for (auto el : native.ImageFormats) {
		ImageFormats->Add(el);
	}

	Manufacture = gcnew System::String(native.Manufacture.c_str());
	Model = gcnew System::String(native.Model.c_str());
	DeviceVersion = gcnew System::String(native.DeviceVersion.c_str());
	SerialNumber = gcnew System::String(native.SerialNumber.c_str());
};



//MtpDatatypeVariant
MtpDatatypeVariant::MtpDatatypeVariant(System::SByte data) {
	m_type = NikonMtpDatatypeCode::Int8;
	m_value = data;
}
MtpDatatypeVariant::MtpDatatypeVariant(System::Byte data) {
	m_type = NikonMtpDatatypeCode::UInt8;
	m_value = data;
}
MtpDatatypeVariant::MtpDatatypeVariant(System::Int16 data) {
	m_type = NikonMtpDatatypeCode::Int16;
	m_value = data;
}
MtpDatatypeVariant::MtpDatatypeVariant(System::UInt16 data) {
	m_type = NikonMtpDatatypeCode::UInt16;
	m_value = data;
}
MtpDatatypeVariant::MtpDatatypeVariant(System::Int32 data) {
	m_type = NikonMtpDatatypeCode::Int32;
	m_value = data;
}
MtpDatatypeVariant::MtpDatatypeVariant(System::UInt32 data) {
	m_type = NikonMtpDatatypeCode::UInt32;
	m_value = data;
}
MtpDatatypeVariant::MtpDatatypeVariant(System::Int64 data) {
	m_type = NikonMtpDatatypeCode::Int64;
	m_value = data;
}
MtpDatatypeVariant::MtpDatatypeVariant(System::UInt64 data) {
	m_type = NikonMtpDatatypeCode::UInt64;
	m_value = data;
}

MtpDatatypeVariant::MtpDatatypeVariant(array<System::SByte>^ data) {
	m_type = NikonMtpDatatypeCode::ArrayInt8;
	m_value = gcnew array<System::SByte>(data->Length);
	System::Buffer::BlockCopy(data, 0, (array<System::SByte>^)m_value, 0, data->Length);
}
MtpDatatypeVariant::MtpDatatypeVariant(array<System::Byte>^ data) {
	m_type = NikonMtpDatatypeCode::ArrayUInt8;
	m_value = gcnew array<System::Byte>(data->Length);
	System::Buffer::BlockCopy(data, 0, (array<System::Byte>^)m_value, 0, data->Length);
}
MtpDatatypeVariant::MtpDatatypeVariant(array<System::Int16>^ data) {
	m_type = NikonMtpDatatypeCode::ArrayInt16;
	m_value = gcnew array<System::Int16>(data->Length);
	System::Buffer::BlockCopy(data, 0, (array<System::Int16>^)m_value, 0, data->Length);
}
MtpDatatypeVariant::MtpDatatypeVariant(array<System::UInt16>^ data) {
	m_type = NikonMtpDatatypeCode::ArrayUInt16;
	m_value = gcnew array<System::UInt16>(data->Length);
	System::Buffer::BlockCopy(data, 0, (array<System::UInt16>^)m_value, 0, data->Length);
}
MtpDatatypeVariant::MtpDatatypeVariant(array<System::Int32>^ data) {
	m_type = NikonMtpDatatypeCode::ArrayInt32;
	m_value = gcnew array<System::Int32>(data->Length);
	System::Buffer::BlockCopy(data, 0, (array<System::Int32>^)m_value, 0, data->Length);
}
MtpDatatypeVariant::MtpDatatypeVariant(array<System::UInt32>^ data) {
	m_type = NikonMtpDatatypeCode::ArrayUInt32;
	m_value = gcnew array<System::UInt32>(data->Length);
	System::Buffer::BlockCopy(data, 0, (array<System::UInt32>^)m_value, 0, data->Length);
}
MtpDatatypeVariant::MtpDatatypeVariant(array<System::Int64>^ data) {
	m_type = NikonMtpDatatypeCode::ArrayInt64;
	m_value = gcnew array<System::Int64>(data->Length);
	System::Buffer::BlockCopy(data, 0, (array<System::Int64>^)m_value, 0, data->Length);
}
MtpDatatypeVariant::MtpDatatypeVariant(array<System::UInt64>^ data) {
	m_type = NikonMtpDatatypeCode::ArrayUInt64;
	m_value = gcnew array<System::UInt64>(data->Length);
	System::Buffer::BlockCopy(data, 0, (array<System::UInt64>^)m_value, 0, data->Length);
}

MtpDatatypeVariant::MtpDatatypeVariant(System::String^ data) {
	m_type = NikonMtpDatatypeCode::String;
	m_value = data->Clone();
}

MtpDatatypeVariant::MtpDatatypeVariant(const nek::mtp::MtpDatatypeVariant& data) {
	switch (data.index()) {
	case 1:
		SetInt8(std::get<int8_t>(data));
		break;
	case 2:
		SetUInt8(std::get<uint8_t>(data));
		break;
	case 3:
		SetInt16(std::get<int16_t>(data));
		break;
	case 4:
		SetUInt16(std::get<uint16_t>(data));
		break;
	case 5:
		SetInt32(std::get<int32_t>(data));
		break;
	case 6:
		SetUInt32(std::get<uint32_t>(data));
		break;
	case 7:
		SetInt64(std::get<int64_t>(data));
		break;
	case 8:
		SetUInt64(std::get<uint64_t>(data));
		break;
	case 9:
		m_type = NikonMtpDatatypeCode::Int128;
		m_value = nullptr;
		throw gcnew System::NotImplementedException();
		break;
	case 10:
		m_type = NikonMtpDatatypeCode::UInt128;
		m_value = nullptr;
		throw gcnew System::NotImplementedException();
		break;
	case 11:
		SetArrayInt8(std::get<std::vector<int8_t>>(data));
		break;
	case 12:
		SetArrayUInt8(std::get<std::vector<uint8_t>>(data));
		break;
	case 13:
		SetArrayInt16(std::get<std::vector<int16_t>>(data));
		break;
	case 14:
		SetArrayUInt16(std::get<std::vector<uint16_t>>(data));
		break;
	case 15:
		SetArrayInt32(std::get<std::vector<int32_t>>(data));
		break;
	case 16:
		SetArrayUInt32(std::get<std::vector<uint32_t>>(data));
		break;
	case 17:
		SetArrayInt64(std::get<std::vector<int64_t>>(data));
		break;
	case 18:
		SetArrayUInt64(std::get<std::vector<uint64_t>>(data));
		break;
	case 19:
		m_type = NikonMtpDatatypeCode::ArrayInt128;
		m_value = nullptr;
		throw gcnew System::NotImplementedException();
		break;
	case 20:
		m_type = NikonMtpDatatypeCode::ArrayUInt128;
		m_value = nullptr;
		throw gcnew System::NotImplementedException();
		break;
	case 21:
		SetString(std::get<std::wstring>(data));
		break;
	default:
		m_type = NikonMtpDatatypeCode::Undefined;
		m_value = nullptr;
		break;
	}
}

void MtpDatatypeVariant::SetInt8(int8_t data) {
	m_type = NikonMtpDatatypeCode::Int8;
	m_value = data;
}
void MtpDatatypeVariant::SetUInt8(uint8_t data) {
	m_type = NikonMtpDatatypeCode::UInt8;
	m_value = data;
}
void MtpDatatypeVariant::SetInt16(int16_t data) {
	m_type = NikonMtpDatatypeCode::Int16;
	m_value = data;
}
void MtpDatatypeVariant::SetUInt16(uint16_t data) {
	m_type = NikonMtpDatatypeCode::UInt16;
	m_value = data;
}
void MtpDatatypeVariant::SetInt32(int32_t data) {
	m_type = NikonMtpDatatypeCode::Int32;
	m_value = data;
}
void MtpDatatypeVariant::SetUInt32(uint32_t data) {
	m_type = NikonMtpDatatypeCode::UInt32;
	m_value = data;
}
void MtpDatatypeVariant::SetInt64(int64_t data) {
	m_type = NikonMtpDatatypeCode::Int64;
	m_value = data;
}
void MtpDatatypeVariant::SetUInt64(uint64_t data) {
	m_type = NikonMtpDatatypeCode::UInt64;
	m_value = data;
}

void MtpDatatypeVariant::SetArrayInt8(const std::vector<int8_t>& data) {
	assert(data.size() <= static_cast<size_t>(INT_MAX));
	m_type = NikonMtpDatatypeCode::ArrayInt8;
	auto value = gcnew array<System::SByte>(static_cast<int>(data.size()));
	if (value->Length > 0) {
		pin_ptr<System::SByte> dataptr = &value[0];
		std::memcpy(dataptr, data.data(), sizeof(int8_t) * value->Length);
	}
	m_value = value;
}
void MtpDatatypeVariant::SetArrayUInt8(const std::vector<uint8_t>& data) {
	assert(data.size() <= static_cast<size_t>(INT_MAX));
	m_type = NikonMtpDatatypeCode::ArrayUInt8;
	auto value = gcnew array<System::Byte>(static_cast<int>(data.size()));
	if (value->Length > 0) {
		pin_ptr<System::Byte> dataptr = &value[0];
		std::memcpy(dataptr, data.data(), sizeof(uint8_t) * value->Length);
	}
	m_value = value;
}
void MtpDatatypeVariant::SetArrayInt16(const std::vector<int16_t>& data) {
	assert(data.size() <= static_cast<size_t>(INT_MAX));
	m_type = NikonMtpDatatypeCode::ArrayInt16;
	auto value = gcnew array<System::Int16>(static_cast<int>(data.size()));
	if (value->Length > 0) {
		pin_ptr<System::Int16> dataptr = &value[0];
		std::memcpy(dataptr, data.data(), sizeof(int16_t) * value->Length);
	}
	m_value = value;
}
void MtpDatatypeVariant::SetArrayUInt16(const std::vector<uint16_t>& data) {
	assert(data.size() <= static_cast<size_t>(INT_MAX));
	m_type = NikonMtpDatatypeCode::ArrayUInt16;
	auto value = gcnew array<System::UInt16>(static_cast<int>(data.size()));
	if (value->Length > 0) {
		pin_ptr<System::UInt16> dataptr = &value[0];
		std::memcpy(dataptr, data.data(), sizeof(uint16_t) * value->Length);
	}
	m_value = value;
}
void MtpDatatypeVariant::SetArrayInt32(const std::vector<int32_t>& data) {
	assert(data.size() <= static_cast<size_t>(INT_MAX));
	m_type = NikonMtpDatatypeCode::ArrayInt32;
	auto value = gcnew array<System::Int32>(static_cast<int>(data.size()));
	if (value->Length > 0) {
		pin_ptr<System::Int32> dataptr = &value[0];
		std::memcpy(dataptr, data.data(), sizeof(int32_t) * value->Length);
	}
	m_value = value;
}
void MtpDatatypeVariant::SetArrayUInt32(const std::vector<uint32_t>& data) {
	assert(data.size() <= static_cast<size_t>(INT_MAX));
	m_type = NikonMtpDatatypeCode::ArrayUInt32;
	auto value = gcnew array<System::UInt32>(static_cast<int>(data.size()));
	if (value->Length > 0) {
		pin_ptr<System::UInt32> dataptr = &value[0];
		std::memcpy(dataptr, data.data(), sizeof(uint32_t) * value->Length);
	}
	m_value = value;
}
void MtpDatatypeVariant::SetArrayInt64(const std::vector<int64_t>& data) {
	assert(data.size() <= static_cast<size_t>(INT_MAX));
	m_type = NikonMtpDatatypeCode::ArrayInt64;
	auto value = gcnew array<System::Int64>(static_cast<int>(data.size()));
	if (value->Length > 0) {
		pin_ptr<System::Int64> dataptr = &value[0];
		std::memcpy(dataptr, data.data(), sizeof(int64_t) * value->Length);
	}
	m_value = value;
}
void MtpDatatypeVariant::SetArrayUInt64(const std::vector<uint64_t>& data) {
	assert(data.size() <= static_cast<size_t>(INT_MAX));
	m_type = NikonMtpDatatypeCode::ArrayUInt64;
	auto value = gcnew array<System::UInt64>(static_cast<int>(data.size()));
	if (value->Length > 0) {
		pin_ptr<System::UInt64> dataptr = &value[0];
		std::memcpy(dataptr, data.data(), sizeof(uint64_t) * value->Length);
	}
	m_value = value;
}

void MtpDatatypeVariant::SetString(const std::wstring& data) {
	m_type = NikonMtpDatatypeCode::String;
	m_value = gcnew System::String(data.c_str());
}

nek::mtp::MtpDatatypeVariant MtpDatatypeVariant::getVariant() {
	if(m_type == NikonMtpDatatypeCode::Int8) {
		auto native = static_cast<int8_t>(safe_cast<System::SByte>(m_value));
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::UInt8) {
		auto native = static_cast<uint8_t>(safe_cast<System::Byte>(m_value));
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::Int16) {
		auto native = static_cast<int16_t>(safe_cast<System::Int16>(m_value));
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::UInt16) {
		auto native = static_cast<uint16_t>(safe_cast<System::UInt16>(m_value));
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::Int32) {
		auto native = static_cast<int32_t>(safe_cast<System::Int32>(m_value));
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::UInt32) {
		auto native = static_cast<uint32_t>(safe_cast<System::UInt32>(m_value));
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::Int64) {
		auto native = static_cast<int64_t>(safe_cast<System::Int64>(m_value));
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::UInt64) {
		auto native = static_cast<uint64_t>(safe_cast<System::UInt64>(m_value));
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::Int128) {
		throw gcnew System::NotImplementedException();
	}
	else if (m_type == NikonMtpDatatypeCode::UInt128) {
		throw gcnew System::NotImplementedException();
	}
	else if (m_type == NikonMtpDatatypeCode::ArrayInt8) {
		auto native = std::vector<int8_t>();
		auto arr = static_cast<array<System::SByte>^>(m_value);
		if (arr->Length > 0) {
			native.resize(arr->Length);
			pin_ptr<System::SByte> dataptr = &arr[0];
			std::memcpy(native.data(), dataptr, sizeof(int8_t) * arr->Length);
		}
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::ArrayUInt8) {
		auto native = std::vector<uint8_t>();
		auto arr = static_cast<array<System::Byte>^>(m_value);
		if (arr->Length > 0) {
			native.resize(arr->Length);
			pin_ptr<System::Byte> dataptr = &arr[0];
			std::memcpy(native.data(), dataptr, sizeof(uint8_t) * arr->Length);
		}
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::ArrayInt16) {
		auto native = std::vector<int16_t>();
		auto arr = static_cast<array<System::Int16>^>(m_value);
		if (arr->Length > 0) {
			native.resize(arr->Length);
			pin_ptr<System::Int16> dataptr = &arr[0];
			std::memcpy(native.data(), dataptr, sizeof(int16_t) * arr->Length);
		}
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::ArrayUInt16) {
		auto native = std::vector<uint16_t>();
		auto arr = static_cast<array<System::UInt16>^>(m_value);
		if (arr->Length > 0) {
			native.resize(arr->Length);
			pin_ptr<System::UInt16> dataptr = &arr[0];
			std::memcpy(native.data(), dataptr, sizeof(uint16_t) * arr->Length);
		}
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::ArrayInt32) {
		auto native = std::vector<int32_t>();
		auto arr = static_cast<array<System::Int32>^>(m_value);
		if (arr->Length > 0) {
			native.resize(arr->Length);
			pin_ptr<System::Int32> dataptr = &arr[0];
			std::memcpy(native.data(), dataptr, sizeof(int32_t) * arr->Length);
		}
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::ArrayUInt32) {
		auto native = std::vector<uint32_t>();
		auto arr = static_cast<array<System::UInt32>^>(m_value);
		if (arr->Length > 0) {
			native.resize(arr->Length);
			pin_ptr<System::UInt32> dataptr = &arr[0];
			std::memcpy(native.data(), dataptr, sizeof(uint32_t) * arr->Length);
		}
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::ArrayInt64) {
		auto native = std::vector<int64_t>();
		auto arr = static_cast<array<System::Int64>^>(m_value);
		if (arr->Length > 0) {
			native.resize(arr->Length);
			pin_ptr<System::Int64> dataptr = &arr[0];
			std::memcpy(native.data(), dataptr, sizeof(int64_t) * arr->Length);
		}
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::ArrayUInt64) {
		auto native = std::vector<uint64_t>();
		auto arr = static_cast<array<System::UInt64>^>(m_value);
		if (arr->Length > 0) {
			native.resize(arr->Length);
			pin_ptr<System::UInt64> dataptr = &arr[0];
			std::memcpy(native.data(), dataptr, sizeof(uint64_t) * arr->Length);
		}
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::String) {
		auto str = static_cast<System::String^>(m_value)->ToCharArray();
		pin_ptr<wchar_t> prstr = &str[0];
		auto native = std::wstring(prstr);
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	
	return nek::mtp::MtpDatatypeVariant();
}


System::Boolean MtpDatatypeVariant::TryGetInt8(System::SByte% data) {
	if (m_type == NikonMtpDatatypeCode::Int8) {
		data = safe_cast<System::SByte>(m_value);
		return true;
	}

	return false;
}
System::Boolean MtpDatatypeVariant::TryGetUInt8(System::Byte% data) {
	if (m_type == NikonMtpDatatypeCode::UInt8) {
		data = safe_cast<System::Byte>(m_value);
		return true;
	}

	return false;
}
System::Boolean MtpDatatypeVariant::TryGetInt16(System::Int16% data) {
	if (m_type == NikonMtpDatatypeCode::Int16) {
		data = safe_cast<System::Int16>(m_value);
		return true;
	}

	return false;
}
System::Boolean MtpDatatypeVariant::TryGetUInt16(System::UInt16% data) {
	if (m_type == NikonMtpDatatypeCode::UInt16) {
		data = safe_cast<System::UInt16>(m_value);
		return true;
	}

	return false;
}
System::Boolean MtpDatatypeVariant::TryGetInt32(System::Int32% data) {
	if (m_type == NikonMtpDatatypeCode::Int32) {
		data = safe_cast<System::Int32>(m_value);
		return true;
	}

	return false;
}
System::Boolean MtpDatatypeVariant::TryGetUInt32(System::UInt32% data) {
	if (m_type == NikonMtpDatatypeCode::UInt32) {
		data = safe_cast<System::UInt32>(m_value);
		return true;
	}

	return false;
}
System::Boolean MtpDatatypeVariant::TryGetInt64(System::Int64% data) {
	if (m_type == NikonMtpDatatypeCode::Int64) {
		data = safe_cast<System::Int64>(m_value);
		return true;
	}

	return false;
}
System::Boolean MtpDatatypeVariant::TryGetUInt64(System::UInt64% data) {
	if (m_type == NikonMtpDatatypeCode::UInt64) {
		data = safe_cast<System::UInt64>(m_value);
		return true;
	}

	return false;
}

System::Boolean MtpDatatypeVariant::TryGetArrayInt8(array<System::SByte>^% data) {
	if (m_type == NikonMtpDatatypeCode::ArrayInt8) {
		data = static_cast<array<System::SByte>^>(m_value);
		return true;
	}

	return false;
}
System::Boolean MtpDatatypeVariant::TryGetArrayUInt8(array<System::Byte>^% data) {
	if (m_type == NikonMtpDatatypeCode::ArrayUInt8) {
		data = static_cast<array<System::Byte>^>(m_value);
		return true;
	}

	return false;
}
System::Boolean MtpDatatypeVariant::TryGetArrayInt16(array<System::Int16>^% data) {
	if (m_type == NikonMtpDatatypeCode::ArrayInt16) {
		data = static_cast<array<System::Int16>^>(m_value);
		return true;
	}

	return false;
}
System::Boolean MtpDatatypeVariant::TryGetArrayUInt16(array<System::UInt16>^% data) {
	if (m_type == NikonMtpDatatypeCode::ArrayUInt16) {
		data = static_cast<array<System::UInt16>^>(m_value);
		return true;
	}

	return false;
}
System::Boolean MtpDatatypeVariant::TryGetArrayInt32(array<System::Int32>^% data) {
	if (m_type == NikonMtpDatatypeCode::ArrayInt32) {
		data = static_cast<array<System::Int32>^>(m_value);
		return true;
	}

	return false;
}
System::Boolean MtpDatatypeVariant::TryGetArrayUInt32(array<System::UInt32>^% data) {
	if (m_type == NikonMtpDatatypeCode::ArrayUInt32) {
		data = static_cast<array<System::UInt32>^>(m_value);
		return true;
	}

	return false;
}
System::Boolean MtpDatatypeVariant::TryGetArrayInt64(array<System::Int64>^% data) {
	if (m_type == NikonMtpDatatypeCode::ArrayInt64) {
		data = static_cast<array<System::Int64>^>(m_value);
		return true;
	}

	return false;
}
System::Boolean MtpDatatypeVariant::TryGetArrayUInt64(array<System::UInt64>^% data) {
	if (m_type == NikonMtpDatatypeCode::ArrayUInt64) {
		data = static_cast<array<System::UInt64>^>(m_value);
		return true;
	}

	return false;
}

System::Boolean MtpDatatypeVariant::TryGetString(System::String^% data) {
	if (m_type == NikonMtpDatatypeCode::String) {
		data = static_cast<System::String^>(m_value);
		return true;
	}

	return false;
}


//NikonDevicePropDescDS_Variant
NikonDevicePropDescDS_Variant::NikonDevicePropDescDS_Variant(const nek::mtp::MtpDevicePropDescDS& desc) {
	DevicePropertyCode = (NikonMtpDevicePropCode)desc.DevicePropertyCode;
	DataType = (NikonMtpDatatypeCode)desc.DataType;
	GetSet = desc.GetSet;

	FactoryDefaultValue = gcnew MtpDatatypeVariant(desc.FactoryDefaultValue);
	CurrentValue = gcnew MtpDatatypeVariant(desc.CurrentValue);
	FormFlag = NikonMtpFormtypeCode(desc.FormFlag);

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		auto form = std::get<nek::mtp::MtpRangeForm>(desc.FORM);
		RangeFORM = gcnew NikonMtpRangeForm<MtpDatatypeVariant^>();
		RangeFORM->min = gcnew MtpDatatypeVariant(form.min);
		RangeFORM->max = gcnew MtpDatatypeVariant(form.max);
		RangeFORM->step = gcnew MtpDatatypeVariant(form.step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		auto form = std::get<nek::mtp::MtpEnumForm>(desc.FORM);
		assert(form.size() <= static_cast<int>(INT_MAX));
		EnumFORM = gcnew array<MtpDatatypeVariant^>(static_cast<int>(form.size()));
		for (int i = 0; i < form.size(); i++) {
			EnumFORM[i] = gcnew MtpDatatypeVariant(form[i]);
		}
	}
}


System::Boolean NikonDevicePropDescDS_Variant::TryGetInt8(NikonDevicePropDescDS<System::SByte>^% desc) {
	if (DataType != NikonMtpDatatypeCode::Int8) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetInt8(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetInt8(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<System::SByte>();
		res &= RangeFORM->min->TryGetInt8(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetInt8(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetInt8(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<System::SByte>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetInt8(desc->EnumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetUInt8(NikonDevicePropDescDS<System::Byte>^% desc) {
	if (DataType != NikonMtpDatatypeCode::UInt8) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetUInt8(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetUInt8(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<System::Byte>();
		res &= RangeFORM->min->TryGetUInt8(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetUInt8(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetUInt8(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<System::Byte>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetUInt8(desc->EnumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetInt16(NikonDevicePropDescDS<System::Int16>^% desc) {
	if (DataType != NikonMtpDatatypeCode::Int16) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetInt16(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetInt16(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<System::Int16>();
		res &= RangeFORM->min->TryGetInt16(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetInt16(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetInt16(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<System::Int16>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetInt16(desc->EnumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetUInt16(NikonDevicePropDescDS<System::UInt16>^% desc) {
	if (DataType != NikonMtpDatatypeCode::UInt16) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetUInt16(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetUInt16(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<System::UInt16>();
		res &= RangeFORM->min->TryGetUInt16(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetUInt16(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetUInt16(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<System::UInt16>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetUInt16(desc->EnumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetInt32(NikonDevicePropDescDS<System::Int32>^% desc) {
	if (DataType != NikonMtpDatatypeCode::Int32) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetInt32(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetInt32(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<System::Int32>();
		res &= RangeFORM->min->TryGetInt32(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetInt32(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetInt32(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<System::Int32>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetInt32(desc->EnumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetUInt32(NikonDevicePropDescDS<System::UInt32>^% desc) {
	if (DataType != NikonMtpDatatypeCode::UInt32) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetUInt32(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetUInt32(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<System::UInt32>();
		res &= RangeFORM->min->TryGetUInt32(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetUInt32(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetUInt32(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<System::UInt32>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetUInt32(desc->EnumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetInt64(NikonDevicePropDescDS<System::Int64>^% desc) {
	if (DataType != NikonMtpDatatypeCode::Int64) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetInt64(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetInt64(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<System::Int64>();
		res &= RangeFORM->min->TryGetInt64(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetInt64(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetInt64(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<System::Int64>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetInt64(desc->EnumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetUInt64(NikonDevicePropDescDS<System::UInt64>^% desc) {
	if (DataType != NikonMtpDatatypeCode::UInt64) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetUInt64(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetUInt64(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<System::UInt64>();
		res &= RangeFORM->min->TryGetUInt64(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetUInt64(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetUInt64(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<System::UInt64>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetUInt64(desc->EnumFORM[i]);
		}
	}

	return res;
}

System::Boolean NikonDevicePropDescDS_Variant::TryGetArrayInt8(NikonDevicePropDescDS<array<System::SByte>^>^% desc) {
	if (DataType != NikonMtpDatatypeCode::ArrayInt8) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetArrayInt8(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetArrayInt8(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<array<System::SByte>^>();
		res &= RangeFORM->min->TryGetArrayInt8(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetArrayInt8(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetArrayInt8(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<array<System::SByte>^>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetArrayInt8(desc->EnumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetArrayUInt8(NikonDevicePropDescDS<array<System::Byte>^>^% desc) {
	if (DataType != NikonMtpDatatypeCode::ArrayUInt8) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetArrayUInt8(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetArrayUInt8(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<array<System::Byte>^>();
		res &= RangeFORM->min->TryGetArrayUInt8(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetArrayUInt8(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetArrayUInt8(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<array<System::Byte>^>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetArrayUInt8(desc->EnumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetArrayInt16(NikonDevicePropDescDS<array<System::Int16>^>^% desc) {
	if (DataType != NikonMtpDatatypeCode::ArrayInt16) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetArrayInt16(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetArrayInt16(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<array<System::Int16>^>();
		res &= RangeFORM->min->TryGetArrayInt16(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetArrayInt16(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetArrayInt16(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<array<System::Int16>^>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetArrayInt16(desc->EnumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetArrayUInt16(NikonDevicePropDescDS<array<System::UInt16>^>^% desc) {
	if (DataType != NikonMtpDatatypeCode::ArrayUInt16) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetArrayUInt16(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetArrayUInt16(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<array<System::UInt16>^>();
		res &= RangeFORM->min->TryGetArrayUInt16(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetArrayUInt16(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetArrayUInt16(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<array<System::UInt16>^>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetArrayUInt16(desc->EnumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetArrayInt32(NikonDevicePropDescDS<array<System::Int32>^>^% desc) {
	if (DataType != NikonMtpDatatypeCode::ArrayInt32) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetArrayInt32(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetArrayInt32(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<array<System::Int32>^>();
		res &= RangeFORM->min->TryGetArrayInt32(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetArrayInt32(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetArrayInt32(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<array<System::Int32>^>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetArrayInt32(desc->EnumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetArrayUInt32(NikonDevicePropDescDS<array<System::UInt32>^>^% desc) {
	if (DataType != NikonMtpDatatypeCode::ArrayUInt32) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetArrayUInt32(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetArrayUInt32(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<array<System::UInt32>^>();
		res &= RangeFORM->min->TryGetArrayUInt32(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetArrayUInt32(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetArrayUInt32(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<array<System::UInt32>^>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetArrayUInt32(desc->EnumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetArrayInt64(NikonDevicePropDescDS<array<System::Int64>^>^% desc) {
	if (DataType != NikonMtpDatatypeCode::ArrayInt64) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetArrayInt64(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetArrayInt64(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<array<System::Int64>^>();
		res &= RangeFORM->min->TryGetArrayInt64(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetArrayInt64(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetArrayInt64(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<array<System::Int64>^>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetArrayInt64(desc->EnumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetArrayUInt64(NikonDevicePropDescDS<array<System::UInt64>^>^% desc) {
	if (DataType != NikonMtpDatatypeCode::ArrayUInt64) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetArrayUInt64(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetArrayUInt64(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<array<System::UInt64>^>();
		res &= RangeFORM->min->TryGetArrayUInt64(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetArrayUInt64(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetArrayUInt64(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<array<System::UInt64>^>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetArrayUInt64(desc->EnumFORM[i]);
		}
	}

	return res;
}

System::Boolean NikonDevicePropDescDS_Variant::TryGetString(NikonDevicePropDescDS<System::String^>^% desc) {
	if (DataType != NikonMtpDatatypeCode::String) {
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGetString(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGetString(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<System::String^>();
		res &= RangeFORM->min->TryGetString(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGetString(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGetString(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<System::String^>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGetString(desc->EnumFORM[i]);
		}
	}

	return res;
}