#pragma once
#include "../nek.hpp"
#include "../utils/nek_threading.hpp"
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

#include <atlbase.h>
#include <PortableDeviceApi.h>
#include <PortableDevice.h>
#include <WpdMtpExtensions.h>



#define CLIENT_NAME			L"Nikon Ekrynox SDK"
#define CLIENT_MAJOR_VER	1
#define CLIENT_MINOR_VER	0
#define CLIENT_REVISION		0



namespace nek::mtp {

	class MtpManager : protected nek::utils::ThreadedClass {
	public:
		NEK_API static MtpManager& Instance();

		NEK_API std::map<std::wstring, MtpDeviceInfoDS> listMtpDevices();
		NEK_API size_t countMtpDevices();
		NEK_API bool isDeviceConnected(std::wstring devicePath);

	private:
		MtpManager& operator= (const MtpManager&) = delete;
		MtpManager(const MtpManager&) = delete;
		MtpManager();
		~MtpManager() { nek::utils::ThreadedClass::~ThreadedClass(); };
		void threadTask();

		CComPtr<IPortableDeviceManager> deviceManager_;
		std::mutex mutexDevice_;
	};



	class MtpDevice : protected nek::utils::MultiThreadedClass {
	public:
		NEK_API MtpDevice(std::wstring devicePath, uint8_t additionalThreadsNb = 0);
		NEK_API ~MtpDevice();

		NEK_API bool isConnected() const;
		NEK_API void Disconnect();

		NEK_API MtpResponse SendCommand(WORD operationCode, MtpParams params);
		NEK_API MtpResponse SendCommandAndRead(WORD operationCode, MtpParams params);
		NEK_API MtpResponse SendCommandAndWrite(WORD operationCode, MtpParams params, std::vector<uint8_t> data);

		NEK_API size_t RegisterCallback(std::function<void(MtpEvent)> callback);
		NEK_API void UnregisterCallback(size_t id);


		NEK_API MtpDeviceInfoDS GetDeviceInfo();

		NEK_API MtpObjectInfoDS GetObjectInfo(uint32_t handle);

		NEK_API MtpDevicePropDescDSV GetDevicePropDesc(uint16_t devicePropCode);
		NEK_API MtpDatatypeVariant GetDevicePropValue(uint16_t devicePropCode);
		NEK_API void SetDevicePropValue(uint16_t devicePropCode, MtpDatatypeVariant data);
		NEK_API void SetDevicePropValueTypesafe(uint16_t devicePropCode, MtpDatatypeVariant data);


	protected:
		MtpDevice();
		virtual void mainThreadTask();
		virtual void additionalThreadsTask();

		static MtpResponse SendCommand_(CComPtr<IPortableDevice> device, WORD operationCode, MtpParams params);
		static MtpResponse SendCommandAndRead_(CComPtr<IPortableDevice> device, WORD operationCode, MtpParams params);
		static MtpResponse SendCommandAndWrite_(CComPtr<IPortableDevice> device, WORD operationCode, MtpParams params, std::vector<uint8_t> data);

		void initCom();
		void initDevice();
		virtual void startThreads();

		std::wstring devicePath_;
		CComPtr<IPortableDeviceValues> deviceClient_;
		CComPtr<IPortableDevice> device_;
		std::atomic<bool> connected_;

		MtpDeviceInfoDS deviceInfo_;
		std::map<uint32_t, uint16_t> devicePropDataType_;

		CComPtr<MtpEventCallback> eventCallback_;
		PWSTR eventCookie_;
		std::mutex mutexDevice_;
		std::mutex mutexDeviceInfo_;

		uint8_t additionalThreadsNb_;


		MtpDevicePropDescDSV GetDevicePropDesc_(MtpResponse& response);
		MtpDatatypeVariant GetDevicePropValue_(MtpResponse& response, uint16_t dataType);
		std::vector<uint8_t> SetDevicePropValue_(MtpDatatypeVariant data);
		bool SetDevicePropValueTypesafe_(const uint16_t dataType, const MtpDatatypeVariant& data, MtpDatatypeVariant& newdata);
	};

}