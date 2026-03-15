#include "pch.h"

#include "Nikon_Backend.h"


using namespace NEKCS;



MtpConnectionInfo::MtpConnectionInfo(const nek::mtp::backend::MtpConnectionInfo& connectionInfo) {
	if (connectionInfo.usbPath.has_value()) _usbPath = gcnew System::String(connectionInfo.usbPath.value().c_str());
	else _usbPath = nullptr;
}

nek::mtp::backend::MtpConnectionInfo MtpConnectionInfo::getNative() {
	nek::mtp::backend::MtpConnectionInfo info;

	if (_usbPath != nullptr) {
		std::wstring str;
		for each (auto c in _usbPath) {
			str += c;
		}
		info.usbPath = str;
	}
	else info.usbPath = std::nullopt;

	return info;
}