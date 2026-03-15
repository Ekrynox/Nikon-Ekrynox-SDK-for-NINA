#pragma once
#include "nek.hpp"
#include "mtp/nek_mtp.hpp"
#include "nikon_enum.hpp"
#include "nikon_struct.hpp"

#include <string>
#include <vector>



namespace nek {

	class NikonCamera : public nek::mtp::MtpDevice {
	public:
		NEK_API static std::vector<std::pair<mtp::backend::MtpConnectionInfo, mtp::MtpDeviceInfoDS>> listNikonCameras(bool onlyOn = true);
		NEK_API static std::vector<std::pair<NikonCamera, mtp::MtpDeviceInfoDS>>getNikonCameras(bool onlyOn = true);
		NEK_API static size_t countNikonCameras(bool onlyOn = true);

		NEK_API NikonCamera(std::unique_ptr<mtp::backend::IMtpTransport> backend, bool autoConnect = true);
		NEK_API NikonCamera(mtp::backend::MtpConnectionInfo connectionInfo, bool autoConnect = true);

		NEK_API void Connect() override;
		NEK_API void Disconnect() override;


		NEK_API NikonDeviceInfoDS GetDeviceInfo();
		NEK_API std::vector<mtp::MtpEvent> GetEvent();
		NEK_API std::vector<mtp::MtpEvent> GetEventEx();



		NEK_API mtp::MtpDevicePropDescDSV GetDevicePropDesc(uint32_t devicePropCode);
		NEK_API mtp::MtpDatatypeVariant GetDevicePropValue(uint32_t devicePropCode);
		NEK_API void SetDevicePropValue(uint32_t devicePropCode, mtp::MtpDatatypeVariant data);
		NEK_API void SetDevicePropValueTypesafe(uint32_t devicePropCode, mtp::MtpDatatypeVariant data);


		NEK_API uint32_t DeviceReady();
		NEK_API uint32_t DeviceReadyWhile(uint32_t whileResponseCode, std::stop_token stopToken = std::stop_token(), size_t sleepTimems = 500);
		NEK_API uint32_t DeviceReadyWhile(std::vector<uint32_t> whileResponseCodes, std::stop_token stopToken = std::stop_token(), size_t sleepTimems = 500);
		NEK_API uint32_t DeviceReadyWhileNot(uint32_t whileNotResponseCode, std::stop_token stopToken = std::stop_token(), size_t sleepTimems = 500);
		NEK_API uint32_t DeviceReadyWhileNot(std::vector<uint32_t> whileNotResponseCodes, std::stop_token stopToken = std::stop_token(), size_t sleepTimems = 500);


		NEK_API uint32_t StartLiveView(bool wait = true, std::stop_token stopToken = std::stop_token(), size_t sleepTimems = 500);
		NEK_API void EndLiveView();


	private:
		std::jthread eventPolling;

		NikonDeviceInfoDS deviceInfo_;
	};

}