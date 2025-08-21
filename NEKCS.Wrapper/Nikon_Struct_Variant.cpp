#include "pch.h"

#include "Nikon_Struct.h"

#include <assert.h>


using namespace System::Runtime::InteropServices;


using namespace NEKCS;



template<typename T> struct NativeToManaged; // no default
template<> struct NativeToManaged<int8_t> { using type = System::SByte; };
template<> struct NativeToManaged<uint8_t> { using type = System::Byte; };
template<> struct NativeToManaged<int16_t> { using type = System::Int16; };
template<> struct NativeToManaged<uint16_t> { using type = System::UInt16; };
template<> struct NativeToManaged<int32_t> { using type = System::Int32; };
template<> struct NativeToManaged<uint32_t> { using type = System::UInt32; };
template<> struct NativeToManaged<int64_t> { using type = System::Int64; };
template<> struct NativeToManaged<uint64_t> { using type = System::UInt64; };




//MtpDatatypeVariant
NikonMtpDatatypeCode MtpDatatypeVariant::GetCodeFromType(System::Type^ t) {
	if (t == System::SByte::typeid) return NikonMtpDatatypeCode::Int8;
	if (t == System::Byte::typeid) return NikonMtpDatatypeCode::UInt8;
	if (t == System::Int16::typeid) return NikonMtpDatatypeCode::Int16;
	if (t == System::UInt16::typeid) return NikonMtpDatatypeCode::UInt16;
	if (t == System::Int32::typeid) return NikonMtpDatatypeCode::Int32;
	if (t == System::UInt32::typeid) return NikonMtpDatatypeCode::UInt32;
	if (t == System::Int64::typeid) return NikonMtpDatatypeCode::Int64;
	if (t == System::UInt64::typeid) return NikonMtpDatatypeCode::UInt64;
	if (t == System::Int128::typeid) return NikonMtpDatatypeCode::Int128;
	if (t == System::UInt128::typeid) return NikonMtpDatatypeCode::UInt128;

	if (t == array<System::SByte>::typeid) return NikonMtpDatatypeCode::ArrayInt8;
	if (t == array<System::Byte>::typeid) return NikonMtpDatatypeCode::ArrayUInt8;
	if (t == array<System::Int16>::typeid) return NikonMtpDatatypeCode::ArrayInt16;
	if (t == array<System::UInt16>::typeid) return NikonMtpDatatypeCode::ArrayUInt16;
	if (t == array<System::Int32>::typeid) return NikonMtpDatatypeCode::ArrayInt32;
	if (t == array<System::UInt32>::typeid) return NikonMtpDatatypeCode::ArrayUInt32;
	if (t == array<System::Int64>::typeid) return NikonMtpDatatypeCode::ArrayInt64;
	if (t == array<System::UInt64>::typeid) return NikonMtpDatatypeCode::ArrayUInt64;
	if (t == array<System::Int128>::typeid) return NikonMtpDatatypeCode::ArrayInt128;
	if (t == array<System::UInt128>::typeid) return NikonMtpDatatypeCode::ArrayUInt128;

	if (t == System::String::typeid) return NikonMtpDatatypeCode::String;

	throw gcnew System::NotSupportedException("This Datatype is not supported: " + t->FullName);
}

template<typename T> void MtpDatatypeVariant::SetManaged(T data) {
	m_value = data;
	m_type = GetCodeFromType(m_value->GetType());
}
template<typename T> void MtpDatatypeVariant::SetManaged(array<T>^ data) {
	auto tmp = gcnew array<T>(data->Length);
	System::Array::Copy(data, tmp, data->Length);
	m_value = tmp;
	m_type = GetCodeFromType(m_value->GetType());
}


template<typename T> void MtpDatatypeVariant::SetNative(const T& data) {
	using managed_t = typename NativeToManaged<T>::type;
	m_value = static_cast<managed_t>(data);
	m_type = GetCodeFromType(m_value->GetType());

}
template<typename T> void MtpDatatypeVariant::SetNative(const std::vector<T>& data) {
	assert(data.size() <= static_cast<size_t>(INT_MAX));
	using managed_t = typename NativeToManaged<T>::type;

	auto value = gcnew array<managed_t>(static_cast<size_t>(data.size()));
	if (value->Length > 0) {
		pin_ptr<managed_t> dataptr = &value[0];
		std::memcpy(dataptr, data.data(), sizeof(T) * value->Length);
	}
	m_value = value;
	m_type = GetCodeFromType(m_value->GetType());
}
template<> void MtpDatatypeVariant::SetNative(const std::wstring& data) {
	m_value = gcnew System::String(data.c_str());
	m_type = GetCodeFromType(m_value->GetType());
}


template<typename T> System::Boolean MtpDatatypeVariant::TryGet([Out] T% data) {
	if (m_type == GetCodeFromType(T::typeid)) {
		data = static_cast<T>(m_value);
		return true;
	}

	data = 0;
	return false;
}
template<typename T> System::Boolean MtpDatatypeVariant::TryGet([Out] array<T>^% data) {
	if (m_type == GetCodeFromType(array<T>::typeid)) {
		data = static_cast<array<T>^>(m_value);
		return true;
	}

	data = gcnew array<T>(0);
	return false;
}
template<> System::Boolean MtpDatatypeVariant::TryGet([Out] System::String^% data) {
	if (m_type == NikonMtpDatatypeCode::String) {
		data = static_cast<System::String^>(m_value);
		return true;
	}

	data = "";
	return false;
}


System::Boolean MtpDatatypeVariant::TryGetInteger([Out] System::Int64% data) {
	if (m_type == GetCodeFromType(System::SByte::typeid)) {
		data = static_cast<System::SByte>(m_value);
		return true;
	} 
	else if (m_type == GetCodeFromType(System::Int16::typeid)) {
		data = static_cast<System::Int16>(m_value);
		return true;
	}
	else if (m_type == GetCodeFromType(System::Int32::typeid)) {
		data = static_cast<System::Int32>(m_value);
		return true;
	}
	else if (m_type == GetCodeFromType(System::Int64::typeid)) {
		data = static_cast<System::Int64>(m_value);
		return true;
	}

	data = 0;
	return false;
}
System::Boolean MtpDatatypeVariant::TryGetUInteger([Out] System::UInt64% data) {
	if (m_type == GetCodeFromType(System::Byte::typeid)) {
		data = static_cast<System::Byte>(m_value);
		return true;
	}
	else if (m_type == GetCodeFromType(System::UInt16::typeid)) {
		data = static_cast<System::UInt16>(m_value);
		return true;
	}
	else if (m_type == GetCodeFromType(System::UInt32::typeid)) {
		data = static_cast<System::UInt32>(m_value);
		return true;
	}
	else if (m_type == GetCodeFromType(System::UInt64::typeid)) {
		data = static_cast<System::UInt64>(m_value);
		return true;
	}

	data = 0;
	return false;
}
System::Boolean MtpDatatypeVariant::TryGetArrayInteger([Out] array<System::Int64>^% data) {
	if (m_type == GetCodeFromType(array<System::SByte>::typeid)) {
		auto v = static_cast<array<System::SByte>^>(m_value);
		data = gcnew array<System::Int64>(v->Length);
		for (int i = 0; i < v->Length; ++i) {
			data[i] = static_cast<System::Int64>(v[i]);
		}
		return true;
	} else if (m_type == GetCodeFromType(array<System::Int16>::typeid)) {
		auto v = static_cast<array<System::Int16>^>(m_value);
		data = gcnew array<System::Int64>(v->Length);
		for (int i = 0; i < v->Length; ++i) {
			data[i] = static_cast<System::Int64>(v[i]);
		}
		return true;
	} else if (m_type == GetCodeFromType(array<System::Int32>::typeid)) {
		auto v = static_cast<array<System::Int32>^>(m_value);
		data = gcnew array<System::Int64>(v->Length);
		for (int i = 0; i < v->Length; ++i) {
			data[i] = static_cast<System::Int64>(v[i]);
		}
		return true;
	} else if (m_type == GetCodeFromType(array<System::Int64>::typeid)) {
		data = static_cast<array<System::Int64>^>(m_value);
		return true;
	}

	data = gcnew array<System::Int64>(0);
	return false;
}
System::Boolean MtpDatatypeVariant::TryGetArrayUInteger([Out] array<System::UInt64>^% data) {
	if (m_type == GetCodeFromType(array<System::Byte>::typeid)) {
		auto v = static_cast<array<System::Byte>^>(m_value);
		data = gcnew array<System::UInt64>(v->Length);
		for (int i = 0; i < v->Length; ++i) {
			data[i] = static_cast<System::UInt64>(v[i]);
		}
		return true;
	}
	else if (m_type == GetCodeFromType(array<System::UInt16>::typeid)) {
		auto v = static_cast<array<System::UInt16>^>(m_value);
		data = gcnew array<System::UInt64>(v->Length);
		for (int i = 0; i < v->Length; ++i) {
			data[i] = static_cast<System::UInt64>(v[i]);
		}
		return true;
	}
	else if (m_type == GetCodeFromType(array<System::UInt32>::typeid)) {
		auto v = static_cast<array<System::UInt32>^>(m_value);
		data = gcnew array<System::UInt64>(v->Length);
		for (int i = 0; i < v->Length; ++i) {
			data[i] = static_cast<System::UInt64>(v[i]);
		}
		return true;
	}
	else if (m_type == GetCodeFromType(array<System::UInt64>::typeid)) {
		data = static_cast<array<System::UInt64>^>(m_value);
		return true;
	}

	data = gcnew array<System::UInt64>(0);
	return false;
}


MtpDatatypeVariant::MtpDatatypeVariant(const nek::mtp::MtpDatatypeVariant& data) {
	m_type = NikonMtpDatatypeCode::Undefined;
	m_value = nullptr;

	if (const auto* pval = std::get_if<int8_t>(&data)) SetNative(*pval);
	if (const auto* pval = std::get_if<uint8_t>(&data)) SetNative(*pval);
	if (const auto* pval = std::get_if<int16_t>(&data)) SetNative(*pval);
	if (const auto* pval = std::get_if<uint16_t>(&data)) SetNative(*pval);
	if (const auto* pval = std::get_if<int32_t>(&data)) SetNative(*pval);
	if (const auto* pval = std::get_if<uint32_t>(&data)) SetNative(*pval);
	if (const auto* pval = std::get_if<int64_t>(&data)) SetNative(*pval);
	if (const auto* pval = std::get_if<uint64_t>(&data)) SetNative(*pval);

	if (const auto* pval = std::get_if<std::vector<int8_t>>(&data)) SetNative(*pval);
	if (const auto* pval = std::get_if<std::vector<uint8_t>>(&data)) SetNative(*pval);
	if (const auto* pval = std::get_if<std::vector<int16_t>>(&data)) SetNative(*pval);
	if (const auto* pval = std::get_if<std::vector<uint16_t>>(&data)) SetNative(*pval);
	if (const auto* pval = std::get_if<std::vector<int32_t>>(&data)) SetNative(*pval);
	if (const auto* pval = std::get_if<std::vector<uint32_t>>(&data)) SetNative(*pval);
	if (const auto* pval = std::get_if<std::vector<int64_t>>(&data)) SetNative(*pval);
	if (const auto* pval = std::get_if<std::vector<uint64_t>>(&data)) SetNative(*pval);

	if (const auto* pval = std::get_if<std::wstring>(&data)) SetNative(*pval);
}

nek::mtp::MtpDatatypeVariant MtpDatatypeVariant::getVariant() {
	if (m_type == NikonMtpDatatypeCode::Int8) {
		int8_t native = static_cast<int8_t>(m_value);
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::UInt8) {
		uint8_t native = static_cast<uint8_t>(m_value);
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::Int16) {
		int16_t native = static_cast<int16_t>(m_value);
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::UInt16) {
		uint16_t native = static_cast<uint16_t>(m_value);
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::Int32) {
		int32_t native = static_cast<int32_t>(m_value);
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::UInt32) {
		uint32_t native = static_cast<uint32_t>(m_value);
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::Int64) {
		int64_t native = static_cast<int64_t>(m_value);
		nek::mtp::MtpDatatypeVariant v = native;
		return v;
	}
	else if (m_type == NikonMtpDatatypeCode::UInt64) {
		uint64_t native = static_cast<uint64_t>(m_value);
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