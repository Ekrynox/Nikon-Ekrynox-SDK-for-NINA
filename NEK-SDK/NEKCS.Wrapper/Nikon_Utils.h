#pragma once
#include "mtp/nek_mtp_utils.hpp"
#include "Nikon_Enum.h"

#include <functional>
#include <vcclr.h>



namespace NEKCS {

	public ref struct MtpResponse {
		property NikonMtpResponseCode ResponseCode {
			NikonMtpResponseCode get() { return _responseCode; }
		}
		property array<System::UInt32>^ Parameters {
			array<System::UInt32>^ get() { return _parameters; }
		}
		property array<System::Byte>^ Data {
			array<System::Byte>^ get() { return _data; }
		}

	private:
		NikonMtpResponseCode _responseCode;
		array<System::UInt32>^ _parameters;
		array<System::Byte>^ _data;

	internal:
		MtpResponse();
		MtpResponse(nek::mtp::MtpResponse response);
	};


	public ref struct MtpEvent {
		property NikonMtpEventCode EventCode {
			NikonMtpEventCode get() { return _eventCode; }
		}
		property array<System::UInt32>^ Parameters {
			array<System::UInt32>^ get() { return _parameters; }
		}

	private:
		NikonMtpEventCode _eventCode;
		array<System::UInt32>^ _parameters;

	internal:
		MtpEvent(nek::mtp::MtpEvent event);
	};


	ref class NikonCamera;
	public delegate void MtpEventHandler(NikonCamera^ camera, MtpEvent^ e);
	private ref class MtpEventHandlerHelper {
	private:
		MtpEventHandler^ handler;
		NikonCamera^ camera;
	public:
		MtpEventHandlerHelper(NikonCamera^ cam, MtpEventHandler^ del) { handler = del; camera = cam; };
		void Dispatch(nek::mtp::MtpEvent nativeEvent) { handler(camera, gcnew MtpEvent(nativeEvent)); };
		static std::function<void(nek::mtp::MtpEvent)> createCallback(gcroot<MtpEventHandlerHelper^>* dispatcher) { return [dispatcher](nek::mtp::MtpEvent evt) { (*dispatcher)->Dispatch(evt); }; };
	};

}