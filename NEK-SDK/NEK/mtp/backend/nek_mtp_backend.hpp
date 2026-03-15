#pragma once

#include "../nek_mtp_utils.hpp"
#include "../nek_mtp_struct.hpp"

#include <functional>
#include <map>
#include <memory>
#include <optional>



namespace nek::mtp::backend {

	class IMtpTransport {
	public:
		virtual ~IMtpTransport() = default;
		virtual std::unique_ptr<IMtpTransport> clone() const = 0;

		virtual void connect() = 0;
		virtual void disconnect() = 0;
		virtual bool isConnected() const = 0;

		virtual MtpResponse sendCommand(uint16_t operationCode, const std::vector<uint32_t>& params) = 0;
		virtual MtpResponse sendCommandAndRead(uint16_t operationCode, const std::vector<uint32_t>& params) = 0;
		virtual MtpResponse sendCommandAndWrite(uint16_t operationCode, const std::vector<uint32_t>& params, const std::vector<uint8_t>& data) = 0;

		using Handler = std::function<void(const MtpEvent&)>;
		virtual size_t subscribe(Handler const& eventCallback) = 0;
		virtual void unsubscribe(size_t id) = 0;
		virtual void unsubscribe() = 0;
	};


	struct MtpConnectionInfo_ {
		std::optional<std::wstring> usbPath = std::nullopt;
	};
	typedef struct MtpConnectionInfo_ MtpConnectionInfo;


	class IMtpBackendProvider {
	public:
		virtual ~IMtpBackendProvider() = default;

		virtual std::unique_ptr<IMtpTransport> tryCreateTransport(const MtpConnectionInfo& connectionInfo) = 0;
		virtual std::vector<std::pair<MtpConnectionInfo, std::unique_ptr<IMtpTransport>>> listDevices() = 0;
		virtual size_t countDevices() = 0;
	};

}