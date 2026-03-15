#pragma once
#include "../nek.hpp"
#include "backend/nek_mtp_backend.hpp"
#include "nek_mtp_utils.hpp"
#include "nek_mtp_enum.hpp"
#include "nek_mtp_struct.hpp"
#include "nek_mtp_except.hpp"

#include <atomic>
#include <future>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>



namespace nek::mtp {

	class MtpDevice {
	public:
		NEK_API MtpDevice(std::unique_ptr<backend::IMtpTransport> backend, bool autoConnect = true);
		NEK_API MtpDevice(const backend::MtpConnectionInfo& connectionInfo, bool autoConnect = true);
		NEK_API MtpDevice(MtpDevice&& other) noexcept;
		NEK_API ~MtpDevice();

		NEK_API bool isConnected() const;
		NEK_API virtual void Connect();
		NEK_API virtual void Disconnect();

		NEK_API MtpResponse SendCommand(uint16_t operationCode, const std::vector<uint32_t>& params);
		NEK_API MtpResponse SendCommandAndRead(uint16_t operationCode, const std::vector<uint32_t>& params);
		NEK_API MtpResponse SendCommandAndWrite(uint16_t operationCode, const std::vector<uint32_t>& params, std::vector<uint8_t> data);

		using Callback = std::function<void(const MtpEvent&)>;
		NEK_API size_t RegisterCallback(Callback const& callback);
		NEK_API void UnregisterCallback(size_t id);


		NEK_API MtpDeviceInfoDS GetDeviceInfo();

		NEK_API MtpObjectInfoDS GetObjectInfo(uint32_t handle);

		NEK_API MtpDevicePropDescDSV GetDevicePropDesc(uint16_t devicePropCode);
		NEK_API MtpDatatypeVariant GetDevicePropValue(uint16_t devicePropCode);
		NEK_API void SetDevicePropValue(uint16_t devicePropCode, MtpDatatypeVariant data);
		NEK_API void SetDevicePropValueTypesafe(uint16_t devicePropCode, MtpDatatypeVariant data);


	protected:
		std::unique_ptr<backend::IMtpTransport> backend_;
		std::optional<size_t> backendCallbackId_;

		MtpDeviceInfoDS deviceInfo_;
		std::mutex mutexDeviceInfo_;

		std::map<uint32_t, uint16_t> devicePropDataType_;

		std::unordered_map<size_t, Callback> eventCallbacks_;
		size_t eventNextId_;
		std::mutex eventMutex_;
		
		void OnEvent(const MtpEvent& event);
		

		MtpDevicePropDescDSV GetDevicePropDesc_(MtpResponse& response);
		MtpDatatypeVariant GetDevicePropValue_(MtpResponse& response, uint16_t dataType);
		std::vector<uint8_t> SetDevicePropValue_(MtpDatatypeVariant data);
		bool SetDevicePropValueTypesafe_(const uint16_t dataType, const MtpDatatypeVariant& data, MtpDatatypeVariant& newdata);
	};



	class MtpManager {
	public:
		NEK_API MtpManager();
		NEK_API void registerBackend(std::unique_ptr<backend::IMtpBackendProvider> backend);

		NEK_API std::vector<std::unique_ptr<backend::IMtpTransport>> tryCreateTransport(const backend::MtpConnectionInfo& connectionInfo);
		NEK_API std::vector<std::pair<backend::MtpConnectionInfo, std::unique_ptr<backend::IMtpTransport>>> listAllDevices();
		NEK_API std::vector<std::pair<backend::MtpConnectionInfo, MtpDevice>> getAllDevices();
		NEK_API size_t countAllDevices();

	private:
		std::vector<std::unique_ptr<backend::IMtpBackendProvider>> backends_;
	};

}