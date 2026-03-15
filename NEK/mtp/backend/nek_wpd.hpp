#pragma once
#include "../../nek.hpp"
#include "nek_mtp_backend.hpp"
#include "../nek_mtp_except.hpp"

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>


#define NOMINMAX
#include <atlbase.h>
#include <PortableDeviceApi.h>
#include <PortableDevice.h>
#include <propvarutil.h>
#include <WpdMtpExtensions.h>

constexpr auto CLIENT_NAME = L"Nikon Ekrynox SDK";
constexpr auto CLIENT_MAJOR_VER = 1;
constexpr auto CLIENT_MINOR_VER = 0;
constexpr auto CLIENT_REVISION = 0;



namespace nek::mtp::backend::wpd {

	class WpdMtpTransport : public IMtpTransport {
	public:
		NEK_API WpdMtpTransport(const std::wstring& devicePath);
		NEK_API ~WpdMtpTransport();
		NEK_API std::unique_ptr<IMtpTransport> clone() const override;

		NEK_API void connect() override;
		NEK_API void disconnect() override;
		NEK_API bool isConnected() const override;

		NEK_API MtpResponse sendCommand(uint16_t operationCode, const std::vector<uint32_t>& params) override;
		NEK_API MtpResponse sendCommandAndRead(uint16_t operationCode, const std::vector<uint32_t>& params) override;
		NEK_API MtpResponse sendCommandAndWrite(uint16_t operationCode, const std::vector<uint32_t>& params, const std::vector<uint8_t>& data) override;

		NEK_API size_t subscribe(Handler const& eventCallback) override;
		NEK_API void unsubscribe(size_t id) override;
		NEK_API void unsubscribe() override;

	private:
		void initCom();
		void initDevice();
		void commandLoop();

		void startEventCapture();
		void stopEventCapture();

		MtpResponse sendCommand_(uint16_t operationCode, const std::vector<uint32_t>& params);
		MtpResponse sendCommandAndRead_(uint16_t operationCode, const std::vector<uint32_t>& params);
		MtpResponse sendCommandAndWrite_(uint16_t operationCode, const std::vector<uint32_t>& params, const std::vector<uint8_t>& data);

		std::atomic_bool running_;
		std::mutex commandMutex_;
		std::condition_variable commandCV_;
		std::thread commandThread_;
		std::function<void()>* command_;

		std::wstring devicePath_;
		CComPtr<IPortableDeviceValues> deviceClient_;
		CComPtr<IPortableDevice> device_;

		class WpdMtpEventManager : public IPortableDeviceEventCallback {
		public:
			HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
			ULONG __stdcall AddRef(void) override;
			ULONG __stdcall Release(void) override;
			HRESULT __stdcall OnEvent(IPortableDeviceValues* pEventParameters) override;

			std::unordered_map<size_t, Handler> eventCallbacks_;
			size_t eventNextId_;
			std::mutex eventMutex_;

		private:
			ULONG ref_ = 0;
		};

		PWSTR eventCookie_;
		CComPtr<WpdMtpEventManager> eventManager_;
	};



	class WpdMtpBackendProvider : public IMtpBackendProvider {
	public:
		NEK_API WpdMtpBackendProvider() {};
		NEK_API ~WpdMtpBackendProvider() {};

		NEK_API std::unique_ptr<IMtpTransport> tryCreateTransport(const MtpConnectionInfo& connectionInfo) override;
		NEK_API std::vector<std::pair<MtpConnectionInfo, std::unique_ptr<IMtpTransport>>> listDevices() override;
		NEK_API size_t countDevices() override;
	};



	class WpdMtpDeviceException : public MtpDeviceException {
	public:
		NEK_API WpdMtpDeviceException(MtpExPhase phase, HRESULT hr);

	private:
		static MtpExCode computeCode(MtpExPhase phase, HRESULT hr);
	};

}