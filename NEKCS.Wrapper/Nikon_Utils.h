#pragma once
#include "mtp/nek_mtp_utils.hpp"
#include "Nikon_Enum.h"

#include <vcclr.h>



namespace NEKCS {

	public ref class MtpReponseParams {
	internal:
		nek::mtp::MtpReponseParams* m_nativeClass;
		MtpReponseParams(nek::mtp::MtpReponseParams responseParams);
		MtpReponseParams();

	public:
		~MtpReponseParams();
		!MtpReponseParams();
	};

	
	public ref class MtpParams {
	internal:
		nek::mtp::MtpParams* m_nativeClass;
		MtpParams(nek::mtp::MtpParams params);

	public:
		MtpParams();
		~MtpParams();
		!MtpParams();

		void addUint32(System::UInt32 param);
		void addUint16(System::UInt16 param);
		void addInt32(System::Int32 param);
		void addInt16(System::Int16 param);
	};


	public ref class MtpResponse {
	internal:
		MtpResponse();
		MtpResponse(nek::mtp::MtpResponse response);

	public:
		~MtpResponse();
		!MtpResponse();

		MtpReponseParams^ GetParams();

		NikonMtpResponseCode responseCode;
		array<System::Byte>^ data;
	internal:
		MtpReponseParams^ responseParams_;
	};


	public ref class MtpEvent {
	internal:
		MtpEvent(nek::mtp::MtpEvent event);

	public:
		NikonMtpEventCode eventCode;
		array<System::UInt32>^ eventParams;
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