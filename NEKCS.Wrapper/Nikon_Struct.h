#pragma once
#include "mtp/nek_mtp_struct.hpp"
#include "Nikon_Enum.h"


using namespace System::Runtime::InteropServices;



namespace NEKCS {

	public ref struct NikonDeviceInfoDS {
		System::UInt16 StandardVersion = 0;
		System::UInt32 VendorExtensionID = 0;
		System::UInt16 VendorExtensionVersion = 0;
		System::String^ VendorExtensionDesc = "";
		System::UInt16 FunctionalMode = 0;
		System::Collections::Generic::List<NikonMtpOperationCode>^ OperationsSupported;
		System::Collections::Generic::List<NikonMtpEventCode>^ EventsSupported;
		System::Collections::Generic::List<NikonMtpDevicePropCode>^ DevicePropertiesSupported;
		System::Collections::Generic::List<System::UInt16>^ CaptureFormats;
		System::Collections::Generic::List<System::UInt16>^ ImageFormats;
		System::String^ Manufacture = "";
		System::String^ Model = "";
		System::String^ DeviceVersion = "";
		System::String^ SerialNumber = "";

		NikonDeviceInfoDS();
		NikonDeviceInfoDS(const nek::mtp::MtpDeviceInfoDS& native);
	};


	public ref struct NikonObjectInfoDS {
		System::UInt32 StorageID = 0;
		NikonMtpObjectFormatCode ObjectFormat = (NikonMtpObjectFormatCode)0;
		System::UInt16 ProtectionStatus = 0;
		System::UInt32 ObjectCompressedSize = 0;
		NikonMtpObjectFormatCode ThumbFormat = (NikonMtpObjectFormatCode)0;
		System::UInt32 ThumbCompressedSize = 0;
		System::UInt32 ThumbPixWidth = 0;
		System::UInt32 ThumbPixHeight = 0;
		System::UInt32 ImagePixWidth = 0;
		System::UInt32 ImagePixHeight = 0;
		System::UInt32 ImageBitDepth = 0;
		System::UInt32 ParentObject = 0;
		System::UInt16 AssociationType = 0;
		System::UInt32 AssociationDesc = 0;
		System::UInt32 SequenceNumber = 0;
		System::String^ Filename = "";
		System::String^ CaptureDate = "";
		System::String^ ModificationDate = "";
		System::Byte Keywords = 0;

		NikonObjectInfoDS();
		NikonObjectInfoDS(const nek::mtp::MtpObjectInfoDS& native);
	};



	public ref class MtpDatatypeVariant {
	private:
		NikonMtpDatatypeCode m_type;
		Object^ m_value;

	public:
		MtpDatatypeVariant(System::SByte data) { SetManaged(data); };
		MtpDatatypeVariant(System::Byte data) { SetManaged(data); };
		MtpDatatypeVariant(System::Int16 data) { SetManaged(data); };
		MtpDatatypeVariant(System::UInt16 data) { SetManaged(data); };
		MtpDatatypeVariant(System::Int32 data) { SetManaged(data); };
		MtpDatatypeVariant(System::UInt32 data) { SetManaged(data); };
		MtpDatatypeVariant(System::Int64 data) { SetManaged(data); };
		MtpDatatypeVariant(System::UInt64 data) { SetManaged(data); };
		MtpDatatypeVariant(array<System::SByte>^ data) { SetManaged(data); };
		MtpDatatypeVariant(array<System::Byte>^ data) { SetManaged(data); };
		MtpDatatypeVariant(array<System::Int16>^ data) { SetManaged(data); };
		MtpDatatypeVariant(array<System::UInt16>^ data) { SetManaged(data); };
		MtpDatatypeVariant(array<System::Int32>^ data) { SetManaged(data); };
		MtpDatatypeVariant(array<System::UInt32>^ data) { SetManaged(data); };
		MtpDatatypeVariant(array<System::Int64>^ data) { SetManaged(data); };
		MtpDatatypeVariant(array<System::UInt64>^ data) { SetManaged(data); };
		MtpDatatypeVariant(System::String^ data) { SetManaged(data); };

		MtpDatatypeVariant(const nek::mtp::MtpDatatypeVariant& data);

		System::Boolean TryGetInt8([Out] System::SByte% data) { return TryGet(data); };
		System::Boolean TryGetUInt8([Out] System::Byte% data) { return TryGet(data); };
		System::Boolean TryGetInt16([Out] System::Int16% data) { return TryGet(data); };
		System::Boolean TryGetUInt16([Out] System::UInt16% data) { return TryGet(data); };
		System::Boolean TryGetInt32([Out] System::Int32% data) { return TryGet(data); };
		System::Boolean TryGetUInt32([Out] System::UInt32% data) { return TryGet(data); };
		System::Boolean TryGetInt64([Out] System::Int64% data) { return TryGet(data); };
		System::Boolean TryGetUInt64([Out] System::UInt64% data) { return TryGet(data); };
		System::Boolean TryGetArrayInt8([Out] array<System::SByte>^% data) { return TryGet(data); };
		System::Boolean TryGetArrayUInt8([Out] array<System::Byte>^% data) { return TryGet(data); };
		System::Boolean TryGetArrayInt16([Out] array<System::Int16>^% data) { return TryGet(data); };
		System::Boolean TryGetArrayUInt16([Out] array<System::UInt16>^% data) { return TryGet(data); };
		System::Boolean TryGetArrayInt32([Out] array<System::Int32>^% data) { return TryGet(data); };
		System::Boolean TryGetArrayUInt32([Out] array<System::UInt32>^% data) { return TryGet(data); };
		System::Boolean TryGetArrayInt64([Out] array<System::Int64>^% data) { return TryGet(data); };
		System::Boolean TryGetArrayUInt64([Out] array<System::UInt64>^% data) { return TryGet(data); };
		System::Boolean TryGetString([Out] System::String^% data) { return TryGet(data); };

	internal:
		nek::mtp::MtpDatatypeVariant getVariant();
		static NikonMtpDatatypeCode GetCodeFromType(System::Type^ t);

		template<typename T> void SetManaged(T data);
		template<typename T> void SetManaged(array<T>^ data);
		template<typename T> void SetNative(const T& data);
		template<typename T> void SetNative(const std::vector<T>& data);
		template<> void SetNative(const std::wstring& data);

		template<typename T> System::Boolean TryGet([Out] T% data);
		template<typename T> System::Boolean TryGet([Out] array<T>^% data);
		template<> System::Boolean TryGet([Out] System::String^% data);
	};


	generic<typename T>
	public ref struct NikonMtpRangeForm {
		T min;
		T max;
		T step;
	};

	generic<typename T>
	public ref struct NikonDevicePropDescDS {
		NikonMtpDevicePropCode DevicePropertyCode = (NikonMtpDevicePropCode)0;
		NikonMtpDatatypeCode DataType = NikonMtpDatatypeCode::Undefined;
		System::Byte GetSet = 0;

		T FactoryDefaultValue;
		T CurrentValue;
		NikonMtpFormtypeCode FormFlag = NikonMtpFormtypeCode::Empty;

		NikonMtpRangeForm<T>^ RangeFORM = nullptr;
		array<T>^ EnumFORM = nullptr;

		NikonDevicePropDescDS() {};
	};
	public ref struct NikonDevicePropDescDS_Variant : NikonDevicePropDescDS<MtpDatatypeVariant^> {
		NikonDevicePropDescDS_Variant(const nek::mtp::MtpDevicePropDescDS& desc);

		template<typename T> System::Boolean TryGet([Out] NikonDevicePropDescDS<T>^% desc);

		System::Boolean TryGetInt8([Out] NikonDevicePropDescDS<System::SByte>^% desc) { return TryGet(desc); };
		System::Boolean TryGetUInt8([Out] NikonDevicePropDescDS<System::Byte>^% desc) { return TryGet(desc); };
		System::Boolean TryGetInt16([Out] NikonDevicePropDescDS<System::Int16>^% desc) { return TryGet(desc); };
		System::Boolean TryGetUInt16([Out] NikonDevicePropDescDS<System::UInt16>^% desc) { return TryGet(desc); };
		System::Boolean TryGetInt32([Out] NikonDevicePropDescDS<System::Int32>^% desc) { return TryGet(desc); };
		System::Boolean TryGetUInt32([Out] NikonDevicePropDescDS<System::UInt32>^% desc) { return TryGet(desc); };
		System::Boolean TryGetInt64([Out] NikonDevicePropDescDS<System::Int64>^% desc) { return TryGet(desc); };
		System::Boolean TryGetUInt64([Out] NikonDevicePropDescDS<System::UInt64>^% desc) { return TryGet(desc); };

		System::Boolean TryGetArrayInt8([Out] NikonDevicePropDescDS<array<System::SByte>^>^% desc) { return TryGet(desc); };
		System::Boolean TryGetArrayUInt8([Out] NikonDevicePropDescDS<array<System::Byte>^>^% desc) { return TryGet(desc); };
		System::Boolean TryGetArrayInt16([Out] NikonDevicePropDescDS<array<System::Int16>^>^% desc) { return TryGet(desc); };
		System::Boolean TryGetArrayUInt16([Out] NikonDevicePropDescDS<array<System::UInt16>^>^% desc) { return TryGet(desc); };
		System::Boolean TryGetArrayInt32([Out] NikonDevicePropDescDS<array<System::Int32>^>^% desc) { return TryGet(desc); };
		System::Boolean TryGetArrayUInt32([Out] NikonDevicePropDescDS<array<System::UInt32>^>^% desc) { return TryGet(desc); };
		System::Boolean TryGetArrayInt64([Out] NikonDevicePropDescDS<array<System::Int64>^>^% desc) { return TryGet(desc); };
		System::Boolean TryGetArrayUInt64([Out] NikonDevicePropDescDS<array<System::UInt64>^>^% desc) { return TryGet(desc); };

		System::Boolean TryGetString([Out] NikonDevicePropDescDS<System::String^>^% desc) { return TryGet(desc); };
	};
}