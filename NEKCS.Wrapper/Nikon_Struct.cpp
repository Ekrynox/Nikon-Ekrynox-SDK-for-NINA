#include "pch.h"

#include "Nikon_Struct.h"

#include <assert.h>


using namespace System::Runtime::InteropServices;


using namespace NEKCS;


//NikonDeviceInfoDS
NikonDeviceInfoDS::NikonDeviceInfoDS() {
	_standardVersion = 0;
	_vendorExtensionID = 0;
	_vendorExtensionVersion = 0;
	_vendorExtensionDesc = gcnew System::String("");
	_functionalMode = 0;
	_operationsSupported = gcnew System::Collections::Generic::List<NikonMtpOperationCode>(0);
	_eventsSupported = gcnew System::Collections::Generic::List<NikonMtpEventCode>(0);
	_devicePropertiesSupported = gcnew System::Collections::Generic::List<NikonMtpDevicePropCode>(0);
	_captureFormats = gcnew System::Collections::Generic::List<System::UInt16>(0);
	_imageFormats = gcnew System::Collections::Generic::List<System::UInt16>(0);
	_manufacture = gcnew System::String("");
	_model = gcnew System::String("");
	_deviceVersion = gcnew System::String("");
	_serialNumber = gcnew System::String("");
};

NikonDeviceInfoDS::NikonDeviceInfoDS(const nek::NikonDeviceInfoDS& native) {
	_standardVersion = native.StandardVersion;
	_vendorExtensionID = native.VendorExtensionID;
	_vendorExtensionVersion = native.VendorExtensionVersion;
	_vendorExtensionDesc = gcnew System::String(native.VendorExtensionDesc.c_str());
	_functionalMode = native.FunctionalMode;

	_operationsSupported = gcnew System::Collections::Generic::List<NikonMtpOperationCode>(0);
	for (auto el : native.OperationsSupported) {
		_operationsSupported->Add((NikonMtpOperationCode)el);
	}
	_eventsSupported = gcnew System::Collections::Generic::List<NikonMtpEventCode>(0);
	for (auto el : native.EventsSupported) {
		_eventsSupported->Add((NikonMtpEventCode)el);
	}
	_devicePropertiesSupported = gcnew System::Collections::Generic::List<NikonMtpDevicePropCode>(0);
	for (auto el : native.DevicePropertiesSupported) {
		_devicePropertiesSupported->Add((NikonMtpDevicePropCode)el);
	}
	_captureFormats = gcnew System::Collections::Generic::List<System::UInt16>(0);
	for (auto el : native.CaptureFormats) {
		_captureFormats->Add(el);
	}
	_imageFormats = gcnew System::Collections::Generic::List<System::UInt16>(0);
	for (auto el : native.ImageFormats) {
		_imageFormats->Add(el);
	}

	_manufacture = gcnew System::String(native.Manufacture.c_str());
	_model = gcnew System::String(native.Model.c_str());
	_deviceVersion = gcnew System::String(native.DeviceVersion.c_str());
	_serialNumber = gcnew System::String(native.SerialNumber.c_str());
};



//NikonObjectInfoDS
NikonObjectInfoDS::NikonObjectInfoDS() {
	_storageID = 0;
	_objectFormat = (NikonMtpObjectFormatCode)0;
	_protectionStatus = 0;
	_objectCompressedSize = 0;
	_thumbFormat = (NikonMtpObjectFormatCode)0;
	_thumbCompressedSize = 0;
	_thumbPixWidth = 0;
	_thumbPixHeight = 0;
	_imagePixWidth = 0;
	_imagePixHeight = 0;
	_imageBitDepth = 0;
	_parentObject = 0;
	_associationType = 0;
	_associationDesc = 0;
	_sequenceNumber = 0;
	_filename = "";
	_captureDate = "";
	_modificationDate = "";
	_keywords = 0;
};

NikonObjectInfoDS::NikonObjectInfoDS(const nek::mtp::MtpObjectInfoDS& native) {
	_storageID = native.StorageID;
	_objectFormat = (NikonMtpObjectFormatCode)native.ObjectFormat;
	_protectionStatus = native.ProtectionStatus;
	_objectCompressedSize = native.ObjectCompressedSize;
	_thumbFormat = (NikonMtpObjectFormatCode)native.ThumbFormat;
	_thumbCompressedSize = native.ThumbCompressedSize;
	_thumbPixWidth = native.ThumbPixWidth;
	_thumbPixHeight = native.ThumbPixHeight;
	_imagePixWidth = native.ImagePixWidth;
	_imagePixHeight = native.ImagePixHeight;
	_imageBitDepth = native.ImageBitDepth;
	_parentObject = native.ParentObject;
	_associationType = native.AssociationType;
	_associationDesc = native.AssociationDesc;
	_sequenceNumber = native.SequenceNumber;
	_filename = gcnew System::String(native.Filename.c_str());
	_captureDate = gcnew System::String(native.CaptureDate.c_str());
	_modificationDate = gcnew System::String(native.ModificationDate.c_str());
	_keywords = native.Keywords;
};



//NikonDevicePropDescDS_Variant
NikonDevicePropDescDS_Variant::NikonDevicePropDescDS_Variant(const nek::mtp::MtpDevicePropDescDSV& desc) {
	_devicePropertyCode = (NikonMtpDevicePropCode)desc.DevicePropertyCode;
	_dataType = (NikonMtpDatatypeCode)desc.DataType;
	_getSet = desc.GetSet;

	_factoryDefaultValue = gcnew MtpDatatypeVariant(desc.FactoryDefaultValue);
	_currentValue = gcnew MtpDatatypeVariant(desc.CurrentValue);
	_formFlag = NikonMtpFormtypeCode(desc.FormFlag);

	if (_formFlag == NikonMtpFormtypeCode::Range) {
		auto form = std::get<nek::mtp::MtpRangeFormV>(desc.FORM);
		_rangeFORM = gcnew NikonMtpRangeForm<MtpDatatypeVariant^>();
		_rangeFORM->_min = gcnew MtpDatatypeVariant(form.min);
		_rangeFORM->_max = gcnew MtpDatatypeVariant(form.max);
		_rangeFORM->_step = gcnew MtpDatatypeVariant(form.step);
	}
	else if (_formFlag == NikonMtpFormtypeCode::Enum) {
		auto form = std::get<nek::mtp::MtpEnumFormV>(desc.FORM);
		assert(form.size() <= static_cast<int>(INT_MAX));
		_enumFORM = gcnew array<MtpDatatypeVariant^>(static_cast<int>(form.size()));
		for (int i = 0; i < form.size(); i++) {
			_enumFORM[i] = gcnew MtpDatatypeVariant(form[i]);
		}
	}
}

template<typename T> System::Boolean NikonDevicePropDescDS_Variant::TryGet([Out] NikonDevicePropDescDS<T>^% desc) {
	desc = gcnew NikonDevicePropDescDS<T>();
	if (_dataType != MtpDatatypeVariant::GetCodeFromType(T::typeid)) {
		return false;
	}
	System::Boolean res = true;

	desc->_devicePropertyCode = _devicePropertyCode;
	desc->_dataType = _dataType;
	desc->_getSet = _getSet;

	res &= _factoryDefaultValue->TryGet(desc->_factoryDefaultValue);
	res &= _currentValue->TryGet(desc->_currentValue);
	desc->_formFlag = _formFlag;

	if (_formFlag == NikonMtpFormtypeCode::Range) {
		desc->_rangeFORM = gcnew NikonMtpRangeForm<T>();
		res &= _rangeFORM->_min->TryGet(desc->_rangeFORM->_min);
		res &= _rangeFORM->_max->TryGet(desc->_rangeFORM->_max);
		res &= _rangeFORM->_step->TryGet(desc->_rangeFORM->_step);
	}
	else if (_formFlag == NikonMtpFormtypeCode::Enum) {
		desc->_enumFORM = gcnew array<T>(_enumFORM->Length);
		for (int i = 0; i < _enumFORM->Length; i++) {
			res &= _enumFORM[i]->TryGet(desc->_enumFORM[i]);
		}
	}

	return res;
}

System::Boolean NikonDevicePropDescDS_Variant::TryGetInteger([Out] NikonDevicePropDescDS<System::Int64>^% desc) {
	desc = gcnew NikonDevicePropDescDS<System::Int64>();
	if ((_dataType != NikonMtpDatatypeCode::Int8) && (_dataType != NikonMtpDatatypeCode::Int16) && (_dataType != NikonMtpDatatypeCode::Int32) && (_dataType != NikonMtpDatatypeCode::Int64)) {
		return false;
	}
	System::Boolean res = true;

	desc->_devicePropertyCode = _devicePropertyCode;
	desc->_dataType = _dataType;
	desc->_getSet = _getSet;

	res &= _factoryDefaultValue->TryGetInteger(desc->_factoryDefaultValue);
	res &= _currentValue->TryGetInteger(desc->_currentValue);
	desc->_formFlag = _formFlag;

	if (_formFlag == NikonMtpFormtypeCode::Range) {
		desc->_rangeFORM = gcnew NikonMtpRangeForm<System::Int64>();
		res &= _rangeFORM->_min->TryGetInteger(desc->_rangeFORM->_min);
		res &= _rangeFORM->_max->TryGetInteger(desc->_rangeFORM->_max);
		res &= _rangeFORM->_step->TryGetInteger(desc->_rangeFORM->_step);
	}
	else if (_formFlag == NikonMtpFormtypeCode::Enum) {
		desc->_enumFORM = gcnew array<System::Int64>(_enumFORM->Length);
		for (int i = 0; i < _enumFORM->Length; i++) {
			res &= _enumFORM[i]->TryGetInteger(desc->_enumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetUInteger([Out] NikonDevicePropDescDS<System::UInt64>^% desc) {
	desc = gcnew NikonDevicePropDescDS<System::UInt64>();
	if ((_dataType != NikonMtpDatatypeCode::UInt8) && (_dataType != NikonMtpDatatypeCode::UInt16) && (_dataType != NikonMtpDatatypeCode::UInt32) && (_dataType != NikonMtpDatatypeCode::UInt64)) {
		return false;
	}
	System::Boolean res = true;

	desc->_devicePropertyCode = _devicePropertyCode;
	desc->_dataType = _dataType;
	desc->_getSet = _getSet;

	res &= _factoryDefaultValue->TryGetUInteger(desc->_factoryDefaultValue);
	res &= _currentValue->TryGetUInteger(desc->_currentValue);
	desc->_formFlag = _formFlag;

	if (_formFlag == NikonMtpFormtypeCode::Range) {
		desc->_rangeFORM = gcnew NikonMtpRangeForm<System::UInt64>();
		res &= _rangeFORM->_min->TryGetUInteger(desc->_rangeFORM->_min);
		res &= _rangeFORM->_max->TryGetUInteger(desc->_rangeFORM->_max);
		res &= _rangeFORM->_step->TryGetUInteger(desc->_rangeFORM->_step);
	}
	else if (_formFlag == NikonMtpFormtypeCode::Enum) {
		desc->_enumFORM = gcnew array<System::UInt64>(_enumFORM->Length);
		for (int i = 0; i < _enumFORM->Length; i++) {
			res &= _enumFORM[i]->TryGetUInteger(desc->_enumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetArrayInteger([Out] NikonDevicePropDescDS<array<System::Int64>^>^% desc) {
	desc = gcnew NikonDevicePropDescDS<array<System::Int64>^>();
	if ((_dataType != NikonMtpDatatypeCode::ArrayInt8) && (_dataType != NikonMtpDatatypeCode::ArrayInt16) && (_dataType != NikonMtpDatatypeCode::ArrayInt32) && (_dataType != NikonMtpDatatypeCode::ArrayInt64)) {
		return false;
	}
	System::Boolean res = true;

	desc->_devicePropertyCode = _devicePropertyCode;
	desc->_dataType = _dataType;
	desc->_getSet = _getSet;

	res &= _factoryDefaultValue->TryGetArrayInteger(desc->_factoryDefaultValue);
	res &= _currentValue->TryGetArrayInteger(desc->_currentValue);
	desc->FormFlag = _formFlag;

	if (_formFlag == NikonMtpFormtypeCode::Range) {
		desc->_rangeFORM = gcnew NikonMtpRangeForm<array<System::Int64>^>();
		res &= _rangeFORM->_min->TryGetArrayInteger(desc->_rangeFORM->_min);
		res &= _rangeFORM->_max->TryGetArrayInteger(desc->_rangeFORM->_max);
		res &= _rangeFORM->_step->TryGetArrayInteger(desc->_rangeFORM->_step);
	}
	else if (_formFlag == NikonMtpFormtypeCode::Enum) {
		desc->_enumFORM = gcnew array<array<System::Int64>^>(_enumFORM->Length);
		for (int i = 0; i < _enumFORM->Length; i++) {
			res &= _enumFORM[i]->TryGetArrayInteger(desc->_enumFORM[i]);
		}
	}

	return res;
}
System::Boolean NikonDevicePropDescDS_Variant::TryGetArrayUInteger([Out] NikonDevicePropDescDS<array<System::UInt64>^>^% desc) {
	desc = gcnew NikonDevicePropDescDS<array<System::UInt64>^>();
	if ((_dataType != NikonMtpDatatypeCode::ArrayUInt8) && (_dataType != NikonMtpDatatypeCode::ArrayUInt16) && (_dataType != NikonMtpDatatypeCode::ArrayUInt32) && (_dataType != NikonMtpDatatypeCode::ArrayUInt64)) {
		return false;
	}
	System::Boolean res = true;

	desc->_devicePropertyCode = _devicePropertyCode;
	desc->_dataType = _dataType;
	desc->_getSet = _getSet;

	res &= _factoryDefaultValue->TryGetArrayUInteger(desc->_factoryDefaultValue);
	res &= _currentValue->TryGetArrayUInteger(desc->_currentValue);
	desc->FormFlag = _formFlag;

	if (_formFlag == NikonMtpFormtypeCode::Range) {
		desc->_rangeFORM = gcnew NikonMtpRangeForm<array<System::UInt64>^>();
		res &= _rangeFORM->_min->TryGetArrayUInteger(desc->_rangeFORM->_min);
		res &= _rangeFORM->_max->TryGetArrayUInteger(desc->_rangeFORM->_max);
		res &= _rangeFORM->_step->TryGetArrayUInteger(desc->_rangeFORM->_step);
	}
	else if (_formFlag == NikonMtpFormtypeCode::Enum) {
		desc->_enumFORM = gcnew array<array<System::UInt64>^>(_enumFORM->Length);
		for (int i = 0; i < _enumFORM->Length; i++) {
			res &= _enumFORM[i]->TryGetArrayUInteger(desc->_enumFORM[i]);
		}
	}

	return res;
}