#pragma once
#include "nek.hpp"
#include "mtp/nek_mtp.hpp"
#include "nikon_enum.hpp"

#include <string>
#include <vector>



namespace nek {

	class NikonCamera : public nek::mtp::MtpDevice {
	public:
		NEK_API static std::map<std::wstring, mtp::MtpDeviceInfoDS> listNikonCameras(bool onlyOn = true);
		NEK_API static size_t countNikonCameras(bool onlyOn = true);

		NEK_API NikonCamera(std::wstring devicePath, uint8_t additionalThread = 0);


		NEK_API mtp::MtpDeviceInfoDS GetDeviceInfo();


		NEK_API mtp::MtpDevicePropDescDS GetDevicePropDesc(uint32_t devicePropCode);
		NEK_API mtp::MtpDatatypeVariant GetDevicePropValue(uint32_t devicePropCode);
		NEK_API void SetDevicePropValue(uint32_t devicePropCode, mtp::MtpDatatypeVariant data);


	private:
		virtual void mainThreadTask() override;
		virtual void threadTask() override;
		void eventThreadTask();

		virtual void startThreads() override;

		std::deque<std::function<void()>> tasksEvent_;
	};

}