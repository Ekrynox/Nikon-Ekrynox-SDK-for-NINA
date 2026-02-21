#pragma once
#include "nikon_struct.hpp"
#include "mtp/nek_mtp_struct.hpp"
#include "Nikon_Enum.h"


using namespace System::Runtime::InteropServices;



namespace NEKCS {

	public ref struct NikonDeviceInfoDS {
	private:
		System::UInt16 _standardVersion = 0;
		System::UInt32 _vendorExtensionID = 0;
		System::UInt16 _vendorExtensionVersion = 0;
		System::String^ _vendorExtensionDesc = "";
		System::UInt16 _functionalMode = 0;
		System::Collections::Generic::List<NikonMtpOperationCode>^ _operationsSupported;
		System::Collections::Generic::List<NikonMtpEventCode>^ _eventsSupported;
		System::Collections::Generic::List<NikonMtpDevicePropCode>^ _devicePropertiesSupported;
		System::Collections::Generic::List<System::UInt16>^ _captureFormats;
		System::Collections::Generic::List<System::UInt16>^ _imageFormats;
		System::String^ _manufacture = "";
		System::String^ _model = "";
		System::String^ _deviceVersion = "";
		System::String^ _serialNumber = "";

	public:
		property System::UInt16 StandardVersion{
			System::UInt16 get() { return _standardVersion; }
			void set(System::UInt16 value) { _standardVersion = value; }
		}
		property System::UInt32 VendorExtensionID{
			System::UInt32 get() { return _vendorExtensionID; }
			void set(System::UInt32 value) { _vendorExtensionID = value; }
		}
		property System::UInt16 VendorExtensionVersion{
			System::UInt16 get() { return _vendorExtensionVersion; }
			void set(System::UInt16 value) { _vendorExtensionVersion = value; }
		}
		property System::String^ VendorExtensionDesc{
			System::String ^ get() { return _vendorExtensionDesc; }
			void set(System::String ^ value) { _vendorExtensionDesc = value; }
		}
		property System::UInt16 FunctionalMode {
			System::UInt16 get() { return _functionalMode; }
			void set(System::UInt16 value) { _functionalMode = value; }
		}
		property System::Collections::Generic::List<NikonMtpOperationCode>^ OperationsSupported {
			System::Collections::Generic::List<NikonMtpOperationCode>^ get() { return _operationsSupported; }
			void set(System::Collections::Generic::List<NikonMtpOperationCode>^ value) { _operationsSupported = value; }
		}
		property System::Collections::Generic::List<NikonMtpEventCode>^ EventsSupported {
			System::Collections::Generic::List<NikonMtpEventCode>^ get() { return _eventsSupported; }
			void set(System::Collections::Generic::List<NikonMtpEventCode>^ value) { _eventsSupported = value; }
		}
		property System::Collections::Generic::List<NikonMtpDevicePropCode>^ DevicePropertiesSupported {
			System::Collections::Generic::List<NikonMtpDevicePropCode>^ get() { return _devicePropertiesSupported; }
			void set(System::Collections::Generic::List<NikonMtpDevicePropCode>^ value) { _devicePropertiesSupported = value; }
		}
		property System::Collections::Generic::List<System::UInt16>^ CaptureFormats {
			System::Collections::Generic::List<System::UInt16>^ get() { return _captureFormats; }
			void set(System::Collections::Generic::List<System::UInt16>^ value) { _captureFormats = value; }
		}
		property System::Collections::Generic::List<System::UInt16>^ ImageFormats {
			System::Collections::Generic::List<System::UInt16>^ get() { return _imageFormats; }
			void set(System::Collections::Generic::List<System::UInt16>^ value) { _imageFormats = value; }
		}
		property System::String^ Manufacture {
			System::String^ get() { return _manufacture; }
			void set(System::String^ value) { _manufacture = value; }
		}
		property System::String^ Model {
			System::String^ get() { return _model; }
			void set(System::String^ value) { _model = value; }
		}
		property System::String^ DeviceVersion {
			System::String^ get() { return _deviceVersion; }
			void set(System::String^ value) { _deviceVersion = value; }
		}
		property System::String^ SerialNumber {
			System::String^ get() { return _serialNumber; }
			void set(System::String^ value) { _serialNumber = value; }
		}

		NikonDeviceInfoDS();
		NikonDeviceInfoDS(const nek::NikonDeviceInfoDS& native);
	};


	public ref struct NikonObjectInfoDS {
	private:
		System::UInt32 _storageID = 0;
		NikonMtpObjectFormatCode _objectFormat = (NikonMtpObjectFormatCode)0;
		System::UInt16 _protectionStatus = 0;
		System::UInt32 _objectCompressedSize = 0;
		NikonMtpObjectFormatCode _thumbFormat = (NikonMtpObjectFormatCode)0;
		System::UInt32 _thumbCompressedSize = 0;
		System::UInt32 _thumbPixWidth = 0;
		System::UInt32 _thumbPixHeight = 0;
		System::UInt32 _imagePixWidth = 0;
		System::UInt32 _imagePixHeight = 0;
		System::UInt32 _imageBitDepth = 0;
		System::UInt32 _parentObject = 0;
		System::UInt16 _associationType = 0;
		System::UInt32 _associationDesc = 0;
		System::UInt32 _sequenceNumber = 0;
		System::String^ _filename = "";
		System::String^ _captureDate = "";
		System::String^ _modificationDate = "";
		System::Byte _keywords = 0;

	public:
		property System::UInt32 StorageID {
			System::UInt32 get() { return _storageID; }
			void set(System::UInt32 value) { _storageID = value; }
		}
		property NikonMtpObjectFormatCode ObjectFormat {
			NikonMtpObjectFormatCode get() { return _objectFormat; }
			void set(NikonMtpObjectFormatCode value) { _objectFormat = value; }
		}
		property System::UInt16 ProtectionStatus {
			System::UInt16 get() { return _protectionStatus; }
			void set(System::UInt16 value) { _protectionStatus = value; }
		}
		property System::UInt32 ObjectCompressedSize {
			System::UInt32 get() { return _objectCompressedSize; }
			void set(System::UInt32 value) { _objectCompressedSize = value; }
		}
		property NikonMtpObjectFormatCode ThumbFormat {
			NikonMtpObjectFormatCode get() { return _thumbFormat; }
			void set(NikonMtpObjectFormatCode value) { _thumbFormat = value; }
		}
		property System::UInt32 ThumbCompressedSize {
			System::UInt32 get() { return _thumbCompressedSize; }
			void set(System::UInt32 value) { _thumbCompressedSize = value; }
		}
		property System::UInt32 ThumbPixWidth {
			System::UInt32 get() { return _thumbPixWidth; }
			void set(System::UInt32 value) { _thumbPixWidth = value; }
		}
		property System::UInt32 ThumbPixHeight {
			System::UInt32 get() { return _thumbPixHeight; }
			void set(System::UInt32 value) { _thumbPixHeight = value; }
		}
		property System::UInt32 ImagePixWidth {
			System::UInt32 get() { return _imagePixWidth; }
			void set(System::UInt32 value) { _imagePixWidth = value; }
		}
		property System::UInt32 ImagePixHeight {
			System::UInt32 get() { return _imagePixHeight; }
			void set(System::UInt32 value) { _imagePixHeight = value; }
		}
		property System::UInt32 ImageBitDepth {
			System::UInt32 get() { return _imageBitDepth; }
			void set(System::UInt32 value) { _imageBitDepth = value; }
		}
		property System::UInt32 ParentObject {
			System::UInt32 get() { return _parentObject; }
			void set(System::UInt32 value) { _parentObject = value; }
		}
		property System::UInt16 AssociationType {
			System::UInt16 get() { return _associationType; }
			void set(System::UInt16 value) { _associationType = value; }
		}
		property System::UInt32 AssociationDesc {
			System::UInt32 get() { return _associationDesc; }
			void set(System::UInt32 value) { _associationDesc = value; }
		}
		property System::UInt32 SequenceNumber {
			System::UInt32 get() { return _sequenceNumber; }
			void set(System::UInt32 value) { _sequenceNumber = value; }
		}
		property System::String^ Filename {
			System::String^ get() { return _filename; }
			void set(System::String^ value) { _filename = value; }
		}
		property System::String^ CaptureDate {
			System::String^ get() { return _captureDate; }
			void set(System::String^ value) { _captureDate = value; }
		}
		property System::String^ ModificationDate {
			System::String^ get() { return _modificationDate; }
			void set(System::String^ value) { _modificationDate = value; }
		}
		property System::Byte Keywords {
			System::Byte get() { return _keywords; }
			void set(System::Byte value) { _keywords = value; }
		}

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

		NikonMtpDatatypeCode GetType() { return m_type; };

		System::Boolean TryGetInteger([Out] System::Int64% data);
		System::Boolean TryGetUInteger([Out] System::UInt64% data);
		System::Boolean TryGetArrayInteger([Out] array<System::Int64>^% data);
		System::Boolean TryGetArrayUInteger([Out] array<System::UInt64>^% data);

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
	internal:
		T _min;
		T _max;
		T _step;

	public:
		property T Min {
			T get() { return _min; }
			void set(T value) { _min = value; }
		}
		property T Max {
			T get() { return _max; }
			void set(T value) { _max = value; }
		}
		property T Step {
			T get() { return _step; }
			void set(T value) { _step = value; }
		}
		NikonMtpRangeForm() {};
	};

	generic<typename T>
	public ref struct NikonDevicePropDescDS {
	internal:
		NikonMtpDevicePropCode _devicePropertyCode = (NikonMtpDevicePropCode)0;
		NikonMtpDatatypeCode _dataType = NikonMtpDatatypeCode::Undefined;
		System::Byte _getSet = 0;

		T _factoryDefaultValue;
		T _currentValue;
		NikonMtpFormtypeCode _formFlag = NikonMtpFormtypeCode::Empty;

		NikonMtpRangeForm<T>^ _rangeFORM = nullptr;
		array<T>^ _enumFORM = nullptr;

	public:
		property NikonMtpDevicePropCode DevicePropertyCode {
			NikonMtpDevicePropCode get() { return _devicePropertyCode; }
			void set(NikonMtpDevicePropCode value) { _devicePropertyCode = value; }
		}
		property NikonMtpDatatypeCode DataType {
			NikonMtpDatatypeCode get() { return _dataType; }
			void set(NikonMtpDatatypeCode value) { _dataType = value; }
		}
		property System::Byte GetSet {
			System::Byte get() { return _getSet; }
			void set(System::Byte value) { _getSet = value; }
		}

		property T FactoryDefaultValue {
			T get() { return _factoryDefaultValue; }
			void set(T value) { _factoryDefaultValue = value; }
		}
		property T CurrentValue {
			T get() { return _currentValue; }
			void set(T value) { _currentValue = value; }
		}
		property NikonMtpFormtypeCode FormFlag {
			NikonMtpFormtypeCode get() { return _formFlag; }
			void set(NikonMtpFormtypeCode value) { _formFlag = value; }
		}

		property NikonMtpRangeForm<T>^ RangeFORM {
			NikonMtpRangeForm<T>^ get() { return _rangeFORM; }
			void set(NikonMtpRangeForm<T>^ value) { _rangeFORM = value; }
		}
		property array<T>^ EnumFORM {
			array<T>^ get() { return _enumFORM; }
			void set(array<T>^ value) { _enumFORM = value; }
		}

		NikonDevicePropDescDS() {};
	};
	public ref struct NikonDevicePropDescDS_Variant : NikonDevicePropDescDS<MtpDatatypeVariant^> {
		NikonDevicePropDescDS_Variant(const nek::mtp::MtpDevicePropDescDSV& desc);

		template<typename T> System::Boolean TryGet([Out] NikonDevicePropDescDS<T>^% desc);

		System::Boolean TryGetInteger([Out] NikonDevicePropDescDS<System::Int64>^% desc);
		System::Boolean TryGetUInteger([Out] NikonDevicePropDescDS<System::UInt64>^% desc);
		System::Boolean TryGetArrayInteger([Out] NikonDevicePropDescDS<array<System::Int64>^>^% desc);
		System::Boolean TryGetArrayUInteger([Out] NikonDevicePropDescDS<array<System::UInt64>^>^% desc);

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