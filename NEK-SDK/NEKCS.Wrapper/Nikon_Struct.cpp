#include "pch.h"

#include "Nikon_Struct.h"

#include <assert.h>


using namespace System::Runtime::InteropServices;


using namespace NEKCS;


//NikonDeviceInfoDS
NikonDeviceInfoDS::NikonDeviceInfoDS() {
	StandardVersion = 0;
	VendorExtensionID = 0;
	VendorExtensionVersion = 0;
	VendorExtensionDesc = gcnew System::String("");
	FunctionalMode = 0;
	OperationsSupported = gcnew System::Collections::Generic::List<NikonMtpOperationCode>(0);
	EventsSupported = gcnew System::Collections::Generic::List<NikonMtpEventCode>(0);
	DevicePropertiesSupported = gcnew System::Collections::Generic::List<NikonMtpDevicePropCode>(0);
	CaptureFormats = gcnew System::Collections::Generic::List<System::UInt16>(0);
	ImageFormats = gcnew System::Collections::Generic::List<System::UInt16>(0);
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

	OperationsSupported = gcnew System::Collections::Generic::List<NikonMtpOperationCode>(0);
	for (auto el : native.OperationsSupported) {
		OperationsSupported->Add((NikonMtpOperationCode)el);
	}
	EventsSupported = gcnew System::Collections::Generic::List<NikonMtpEventCode>(0);
	for (auto el : native.EventsSupported) {
		EventsSupported->Add((NikonMtpEventCode)el);
	}
	DevicePropertiesSupported = gcnew System::Collections::Generic::List<NikonMtpDevicePropCode>(0);
	for (auto el : native.DevicePropertiesSupported) {
		DevicePropertiesSupported->Add((NikonMtpDevicePropCode)el);
	}
	CaptureFormats = gcnew System::Collections::Generic::List<System::UInt16>(0);
	for (auto el : native.CaptureFormats) {
		CaptureFormats->Add(el);
	}
	ImageFormats = gcnew System::Collections::Generic::List<System::UInt16>(0);
	for (auto el : native.ImageFormats) {
		ImageFormats->Add(el);
	}

	Manufacture = gcnew System::String(native.Manufacture.c_str());
	Model = gcnew System::String(native.Model.c_str());
	DeviceVersion = gcnew System::String(native.DeviceVersion.c_str());
	SerialNumber = gcnew System::String(native.SerialNumber.c_str());
};



//NikonObjectInfoDS
NikonObjectInfoDS::NikonObjectInfoDS() {
	StorageID = 0;
	ObjectFormat = (NikonMtpObjectFormatCode)0;
	ProtectionStatus = 0;
	ObjectCompressedSize = 0;
	ThumbFormat = (NikonMtpObjectFormatCode)0;
	ThumbCompressedSize = 0;
	ThumbPixWidth = 0;
	ThumbPixHeight = 0;
	ImagePixWidth = 0;
	ImagePixHeight = 0;
	ImageBitDepth = 0;
	ParentObject = 0;
	AssociationType = 0;
	AssociationDesc = 0;
	SequenceNumber = 0;
	Filename = "";
	CaptureDate = "";
	ModificationDate = "";
	Keywords = 0;
};

NikonObjectInfoDS::NikonObjectInfoDS(const nek::mtp::MtpObjectInfoDS& native) {
	StorageID = native.StorageID;
	ObjectFormat = (NikonMtpObjectFormatCode)native.ObjectFormat;
	ProtectionStatus = native.ProtectionStatus;
	ObjectCompressedSize = native.ObjectCompressedSize;
	ThumbFormat = (NikonMtpObjectFormatCode)native.ThumbFormat;
	ThumbCompressedSize = native.ThumbCompressedSize;
	ThumbPixWidth = native.ThumbPixWidth;
	ThumbPixHeight = native.ThumbPixHeight;
	ImagePixWidth = native.ImagePixWidth;
	ImagePixHeight = native.ImagePixHeight;
	ImageBitDepth = native.ImageBitDepth;
	ParentObject = native.ParentObject;
	AssociationType = native.AssociationType;
	AssociationDesc = native.AssociationDesc;
	SequenceNumber = native.SequenceNumber;
	Filename = gcnew System::String(native.Filename.c_str());
	CaptureDate = gcnew System::String(native.CaptureDate.c_str());
	ModificationDate = gcnew System::String(native.ModificationDate.c_str());
	Keywords = native.Keywords;
};



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

template<typename T> System::Boolean NikonDevicePropDescDS_Variant::TryGet([Out] NikonDevicePropDescDS<T>^% desc) {
	if (DataType != MtpDatatypeVariant::GetCodeFromType(T::typeid)) {
		desc = gcnew NikonDevicePropDescDS<T>();
		return false;
	}
	System::Boolean res = true;

	desc->DevicePropertyCode = DevicePropertyCode;
	desc->DataType = DataType;
	desc->GetSet = GetSet;

	res &= FactoryDefaultValue->TryGet(desc->FactoryDefaultValue);
	res &= CurrentValue->TryGet(desc->CurrentValue);
	desc->FormFlag = FormFlag;

	if (FormFlag == NikonMtpFormtypeCode::Range) {
		desc->RangeFORM = gcnew NikonMtpRangeForm<T>();
		res &= RangeFORM->min->TryGet(desc->RangeFORM->min);
		res &= RangeFORM->max->TryGet(desc->RangeFORM->max);
		res &= RangeFORM->step->TryGet(desc->RangeFORM->step);
	}
	else if (FormFlag == NikonMtpFormtypeCode::Enum) {
		desc->EnumFORM = gcnew array<T>(EnumFORM->Length);
		for (int i = 0; i < EnumFORM->Length; i++) {
			res &= EnumFORM[i]->TryGet(desc->EnumFORM[i]);
		}
	}

	return res;
}