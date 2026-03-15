#pragma once
#include <mtp/backend/nek_mtp_backend.hpp>



namespace NEKCS {

	public ref struct MtpConnectionInfo {
		property System::String^ UsbPath {
			System::String^ get() { return _usbPath; }
			void set(System::String^ value) { _usbPath = value; }
		}

		MtpConnectionInfo() {};

	private:
		System::String^ _usbPath;

	internal:
		MtpConnectionInfo(const nek::mtp::backend::MtpConnectionInfo& connectionInfo);
		nek::mtp::backend::MtpConnectionInfo getNative();
	};

}