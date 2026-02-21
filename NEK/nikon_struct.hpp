#pragma once
#include "nek.hpp"
#include "mtp/nek_mtp_struct.hpp"



namespace nek {

	struct NikonDeviceInfoDS_ : mtp::MtpDeviceInfoDS_ {
		std::vector<uint32_t> OperationsSupported;
		std::vector<uint32_t> DevicePropertiesSupported;

		NikonDeviceInfoDS_() : mtp::MtpDeviceInfoDS_() {
			OperationsSupported = std::vector<uint32_t>();
			DevicePropertiesSupported = std::vector<uint32_t>();
		}

		NikonDeviceInfoDS_(const mtp::MtpDeviceInfoDS_& base) : mtp::MtpDeviceInfoDS_(base) {
			OperationsSupported.reserve(base.OperationsSupported.size());
			for (auto v : base.OperationsSupported) OperationsSupported.push_back(static_cast<uint32_t>(v));

			DevicePropertiesSupported.reserve(base.DevicePropertiesSupported.size());
			for (auto v : base.DevicePropertiesSupported) DevicePropertiesSupported.push_back(static_cast<uint32_t>(v));
		}

		NikonDeviceInfoDS_& operator=(const mtp::MtpDeviceInfoDS_& base) {
			mtp::MtpDeviceInfoDS_::operator=(base);

			OperationsSupported.reserve(base.OperationsSupported.size());
			for (auto v : base.OperationsSupported) OperationsSupported.push_back(static_cast<uint32_t>(v));

			DevicePropertiesSupported.reserve(base.DevicePropertiesSupported.size());
			for (auto v : base.DevicePropertiesSupported) DevicePropertiesSupported.push_back(static_cast<uint32_t>(v));
		}
	};
	typedef struct NikonDeviceInfoDS_ NikonDeviceInfoDS;

}