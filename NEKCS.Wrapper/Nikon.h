#pragma once
#include "nikon.hpp"
#include "Nikon_Utils.h"
#include "Nikon_Enum.h"
#include "Nikon_Struct.h"
#include "Nikon_Except.h"

#include <vcclr.h>



namespace NEKCS {

	public ref class NikonCamera {
	private:
		nek::NikonCamera* m_nativeClass;
		System::Collections::Generic::Dictionary<MtpEventHandler^, size_t> _callbackIds;

	public:
		static System::Collections::Generic::Dictionary<System::String^, NikonDeviceInfoDS^>^ listNikonCameras(System::Boolean onlyOn);
		static System::Collections::Generic::Dictionary<System::String^, NikonDeviceInfoDS^>^ listNikonCameras();
		static size_t countNikonCameras(System::Boolean onlyOn);
		static size_t countNikonCameras();

		NikonCamera(System::String^ devicePath, System::Byte additionThreads);
		NikonCamera(System::String^ devicePath);
		~NikonCamera();
		!NikonCamera();

		System::Boolean isConnected();

		MtpResponse^ SendCommand(NikonMtpOperationCode operationCode, MtpParams^ params);
		MtpResponse^ SendCommandAndRead(NikonMtpOperationCode operationCode, MtpParams^ params);
		MtpResponse^ SendCommandAndWrite(NikonMtpOperationCode operationCode, MtpParams^ params, array<System::Byte>^ data);


		event MtpEventHandler^ OnMtpEvent {
			void add(MtpEventHandler^ handler) {
				if (_callbackIds.ContainsKey(handler)) {
					return;
				}
				try {
					auto dispatcher = new gcroot<MtpEventHandlerHelper^>(gcnew MtpEventHandlerHelper(this, handler));
					size_t callbackId = m_nativeClass->RegisterCallback(MtpEventHandlerHelper::createCallback(dispatcher));
					_callbackIds[handler] = callbackId;
				}
				catch (const nek::mtp::MtpDeviceException& e) {
					throw gcnew MtpDeviceException(e);
				}
			}
			void remove(MtpEventHandler^ handler) {
				try {
					if (_callbackIds.ContainsKey(handler)) {
						this->m_nativeClass->UnregisterCallback(_callbackIds[handler]);
						_callbackIds.Remove(handler);
					}
				}
				catch (const nek::mtp::MtpDeviceException& e) {
					throw gcnew MtpDeviceException(e);
				}
			}
		}


		NikonDeviceInfoDS^ GetDeviceInfo();

		NikonObjectInfoDS^ GetObjectInfo(System::UInt32 handle);

		NikonDevicePropDescDS_Variant^ GetDevicePropDesc(NikonMtpDevicePropCode devicePropCode);
		MtpDatatypeVariant^ GetDevicePropValue(NikonMtpDevicePropCode devicePropCode);
		void SetDevicePropValue(NikonMtpDevicePropCode devicePropCode, MtpDatatypeVariant^ data);

		NikonMtpResponseCode DeviceReady();
		NikonMtpResponseCode DeviceReadyWhile(NikonMtpResponseCode whileResponseCode, System::Threading::CancellationToken stopToken, System::UInt32 sleepTimems);
		NikonMtpResponseCode DeviceReadyWhile(NikonMtpResponseCode whileResponseCode, System::Threading::CancellationToken stopToken);
		NikonMtpResponseCode DeviceReadyWhile(NikonMtpResponseCode whileResponseCode, System::UInt32 sleepTimems);
		NikonMtpResponseCode DeviceReadyWhile(NikonMtpResponseCode whileResponseCode);
		NikonMtpResponseCode DeviceReadyWhile(System::Collections::Generic::List<NikonMtpResponseCode>^ whileResponseCodes, System::Threading::CancellationToken stopToken, System::UInt32 sleepTimems);
		NikonMtpResponseCode DeviceReadyWhile(System::Collections::Generic::List<NikonMtpResponseCode>^ whileResponseCodes, System::Threading::CancellationToken stopToken);
		NikonMtpResponseCode DeviceReadyWhile(System::Collections::Generic::List<NikonMtpResponseCode>^ whileResponseCodes, System::UInt32 sleepTimems);
		NikonMtpResponseCode DeviceReadyWhile(System::Collections::Generic::List<NikonMtpResponseCode>^ whileResponseCodes);
		NikonMtpResponseCode DeviceReadyWhileNot(NikonMtpResponseCode whileNotResponseCode, System::Threading::CancellationToken stopToken, System::UInt32 sleepTimems);
		NikonMtpResponseCode DeviceReadyWhileNot(NikonMtpResponseCode whileNotResponseCode, System::Threading::CancellationToken stopToken);
		NikonMtpResponseCode DeviceReadyWhileNot(NikonMtpResponseCode whileNotResponseCode, System::UInt32 sleepTimems);
		NikonMtpResponseCode DeviceReadyWhileNot(NikonMtpResponseCode whileNotResponseCode);
		NikonMtpResponseCode DeviceReadyWhileNot(System::Collections::Generic::List<NikonMtpResponseCode>^ whileNotResponseCodes, System::Threading::CancellationToken stopToken, System::UInt32 sleepTimems);
		NikonMtpResponseCode DeviceReadyWhileNot(System::Collections::Generic::List<NikonMtpResponseCode>^ whileNotResponseCodes, System::Threading::CancellationToken stopToken);
		NikonMtpResponseCode DeviceReadyWhileNot(System::Collections::Generic::List<NikonMtpResponseCode>^ whileNotResponseCodes, System::UInt32 sleepTimems);
		NikonMtpResponseCode DeviceReadyWhileNot(System::Collections::Generic::List<NikonMtpResponseCode>^ whileNotResponseCodes);

		NikonMtpResponseCode StartLiveView(System::Boolean wait, System::Threading::CancellationToken stopToken, System::UInt32 sleepTimems);
		NikonMtpResponseCode StartLiveView(System::Boolean wait, System::Threading::CancellationToken stopToken);
		NikonMtpResponseCode StartLiveView(System::Boolean wait, System::UInt32 sleepTimems);
		NikonMtpResponseCode StartLiveView(System::Boolean wait);
		NikonMtpResponseCode StartLiveView();
		void EndLiveView();
	};

}