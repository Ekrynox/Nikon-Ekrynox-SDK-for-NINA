﻿#include "pch.h"

#include "Nikon.h"



using namespace NEKCS;



//NikonCamera
System::Collections::Generic::Dictionary<System::String^, NikonDeviceInfoDS^>^ NikonCamera::listNikonCameras(System::Boolean onlyOn) {
	try {
		System::Collections::Generic::Dictionary<System::String^, NikonDeviceInfoDS^>^ result = gcnew System::Collections::Generic::Dictionary<System::String^, NikonDeviceInfoDS^>();
		for (auto& camera : nek::NikonCamera::listNikonCameras(onlyOn)) {
			result->Add(gcnew System::String(camera.first.c_str()), gcnew NikonDeviceInfoDS(camera.second));
		}
		return result;
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
}
System::Collections::Generic::Dictionary<System::String^, NikonDeviceInfoDS^>^ NikonCamera::listNikonCameras() { return listNikonCameras(true); }

size_t NikonCamera::countNikonCameras(System::Boolean onlyOn) { return nek::NikonCamera::countNikonCameras(onlyOn); }
size_t NikonCamera::countNikonCameras() { return countNikonCameras(true); }



NikonCamera::NikonCamera(System::String^ devicePath, System::Byte additionThreads) {
	std::wstring str;
	for each (auto c in devicePath) {
		str += c;
	}
	try {
		m_nativeClass = new nek::NikonCamera(str, additionThreads);
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
};
NikonCamera::NikonCamera(System::String^ devicePath) : NikonCamera(devicePath, 0) {};
NikonCamera::~NikonCamera() { this->!NikonCamera(); };
NikonCamera::!NikonCamera() {
	if (m_nativeClass != nullptr) {
		delete m_nativeClass;
		m_nativeClass = nullptr;
	}
};


System::Boolean NikonCamera::isConnected() {
	return m_nativeClass->isConnected();
}



MtpResponse^ NikonCamera::SendCommand(NikonMtpOperationCode operationCode, MtpParams^ params) { 
	try {
		return gcnew MtpResponse(m_nativeClass->SendCommand((System::UInt16)operationCode, *params->m_nativeClass));
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
}
MtpResponse^ NikonCamera::SendCommandAndRead(NikonMtpOperationCode operationCode, MtpParams^ params) {
	try {
		return gcnew MtpResponse(m_nativeClass->SendCommandAndRead((System::UInt16)operationCode, *params->m_nativeClass));
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
}
MtpResponse^ NikonCamera::SendCommandAndWrite(NikonMtpOperationCode operationCode, MtpParams^ params, array<System::Byte>^ data) {
	std::vector<BYTE> datac = std::vector<BYTE>();
	datac.resize(data->Length);
	if (data->Length > 0) {
		pin_ptr<System::Byte> dataptr = &data[0];
		std::memcpy(datac.data(), dataptr, data->Length);
	}
	try {
		return gcnew MtpResponse(m_nativeClass->SendCommandAndWrite((System::UInt16)operationCode, *params->m_nativeClass, datac));
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
}



NikonDeviceInfoDS^ NikonCamera::GetDeviceInfo() {
	try {
		return gcnew NikonDeviceInfoDS(m_nativeClass->GetDeviceInfo());
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
	catch (const nek::mtp::MtpException& e) {
		throw gcnew MtpException(e);
	}
}

NikonObjectInfoDS^ NikonCamera::GetObjectInfo(System::UInt32 handle) {
	try {
		return gcnew NikonObjectInfoDS(m_nativeClass->GetObjectInfo(handle));
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
	catch (const nek::mtp::MtpException& e) {
		throw gcnew MtpException(e);
	}
}

NikonDevicePropDescDS_Variant^ NikonCamera::GetDevicePropDesc(NikonMtpDevicePropCode devicePropCode) {
	try {
		return gcnew NikonDevicePropDescDS_Variant(m_nativeClass->GetDevicePropDesc((System::UInt32)devicePropCode));
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
	catch (const nek::mtp::MtpException& e) {
		throw gcnew MtpException(e);
	}
}
MtpDatatypeVariant^ NikonCamera::GetDevicePropValue(NikonMtpDevicePropCode devicePropCode) {
	try {
		return gcnew MtpDatatypeVariant(m_nativeClass->GetDevicePropValue((System::UInt32)devicePropCode));
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
	catch (const nek::mtp::MtpException& e) {
		throw gcnew MtpException(e);
	}
}
void NikonCamera::SetDevicePropValue(NikonMtpDevicePropCode devicePropCode, MtpDatatypeVariant^ data) {
	try {
		return m_nativeClass->SetDevicePropValue((System::UInt32)devicePropCode, data->getVariant());
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
	catch (const nek::mtp::MtpException& e) {
		throw gcnew MtpException(e);
	}
}


NikonMtpResponseCode NikonCamera::DeviceReady() {
	try {
		return (NikonMtpResponseCode)m_nativeClass->DeviceReady();
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
}
NikonMtpResponseCode NikonCamera::DeviceReady(NikonMtpResponseCode whileResponseCode, System::UInt32 sleepTimems) {
	try {
		return (NikonMtpResponseCode)m_nativeClass->DeviceReady((System::UInt32)whileResponseCode, sleepTimems);
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
}
NikonMtpResponseCode NikonCamera::DeviceReady(NikonMtpResponseCode whileResponseCode) {
	try {
		return (NikonMtpResponseCode)m_nativeClass->DeviceReady((System::UInt32)whileResponseCode);
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
}


NikonMtpResponseCode NikonCamera::StartLiveView(System::Boolean wait, System::UInt32 sleepTimems) {
	try {
		return (NikonMtpResponseCode)m_nativeClass->StartLiveView(wait, sleepTimems);
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
	catch (const nek::mtp::MtpException& e) {
		throw gcnew MtpException(e);
	}
}
NikonMtpResponseCode NikonCamera::StartLiveView(System::Boolean wait) {
	try {
		return (NikonMtpResponseCode)m_nativeClass->StartLiveView(wait);
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
	catch (const nek::mtp::MtpException& e) {
		throw gcnew MtpException(e);
	}
}
NikonMtpResponseCode NikonCamera::StartLiveView() {
	try {
		return (NikonMtpResponseCode)m_nativeClass->StartLiveView();
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
	catch (const nek::mtp::MtpException& e) {
		throw gcnew MtpException(e);
	}
}
void NikonCamera::EndLiveView() {
	try {
		return m_nativeClass->EndLiveView();
	}
	catch (const nek::mtp::MtpDeviceException& e) {
		throw gcnew MtpDeviceException(e);
	}
	catch (const nek::mtp::MtpException& e) {
		throw gcnew MtpException(e);
	}
}