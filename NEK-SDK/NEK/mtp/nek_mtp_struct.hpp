#pragma once
#include "../utils/nek_int128.hpp"

#include <stdint.h>
#include <string>
#include <variant>
#include <vector>



namespace nek::mtp {

	struct MtpDeviceInfoDS_ {
		uint16_t StandardVersion = 0;
		uint32_t VendorExtensionID = 0;
		uint16_t VendorExtensionVersion = 0;

		//uint8_t VendorExtensionDescLength;
		std::wstring VendorExtensionDesc; //char16_t

		uint16_t FunctionalMode = 0;

		//uint32_t OperationsSupportedLength;
		std::vector<uint16_t> OperationsSupported;

		//uint32_t EventsSupportedLength;
		std::vector<uint16_t> EventsSupported;

		//uint32_t DevicePropertiesSupportedLength;
		std::vector<uint16_t> DevicePropertiesSupported;

		//uint32_t CaptureFormatsLength;
		std::vector<uint16_t> CaptureFormats;

		//uint32_t ImageFormatsLength;
		std::vector<uint16_t> ImageFormats;

		//uint8_t ManufactureLength;
		std::wstring Manufacture; //char16_t

		//uint8_t ModelLength;
		std::wstring Model; //char16_t

		//uint8_t DeviceVersionLength;
		std::wstring DeviceVersion; //char16_t

		//uint8_t SerialNumberLength;
		std::wstring SerialNumber; //char16_t
	};
	typedef struct MtpDeviceInfoDS_ MtpDeviceInfoDS;


	struct MtpObjectInfoDS_ {
		uint32_t StorageID = 0;
		uint16_t ObjectFormat = 0;
		uint16_t ProtectionStatus = 0;
		uint32_t ObjectCompressedSize = 0;
		uint16_t ThumbFormat = 0;
		uint32_t ThumbCompressedSize = 0;
		uint32_t ThumbPixWidth = 0;
		uint32_t ThumbPixHeight = 0;
		uint32_t ImagePixWidth = 0;
		uint32_t ImagePixHeight = 0;
		uint32_t ImageBitDepth = 0;
		uint32_t ParentObject = 0;
		uint16_t AssociationType = 0;
		uint32_t AssociationDesc = 0;
		uint32_t SequenceNumber = 0;
		std::wstring Filename;
		std::wstring CaptureDate;
		std::wstring ModificationDate;
		uint8_t Keywords = 0;
	};
	typedef struct MtpObjectInfoDS_ MtpObjectInfoDS;




	using MtpDatatypeVariant = std::variant<std::monostate, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, int128, uint128, std::vector<int8_t>, std::vector<uint8_t>, std::vector<int16_t>, std::vector<uint16_t>, std::vector<int32_t>, std::vector<uint32_t>, std::vector<int64_t>, std::vector<uint64_t>, std::vector<int128>, std::vector<uint128>, std::wstring>;
	
	struct MtpRangeForm_ {
		MtpDatatypeVariant min;
		MtpDatatypeVariant max;
		MtpDatatypeVariant step;
	};
	typedef struct MtpRangeForm_ MtpRangeForm;
	typedef std::vector<MtpDatatypeVariant> MtpEnumForm;

	struct MtpDevicePropDescDS_ {
		uint32_t DevicePropertyCode = 0; //uint16_t for the GetDevicePropDesc operation //uint32_t for the Nikon GetDevicePropDescEx operation
		uint16_t DataType = 0;
		uint8_t GetSet = 0; //0x00 read-only, 0x01 read-write
		MtpDatatypeVariant FactoryDefaultValue; //See DataType for the type of data stored here
		MtpDatatypeVariant CurrentValue; //See DataType for the type of data stored here
		uint8_t FormFlag = 0; //0x00 normal, 0x01 range, 0x02 enum

		std::variant<
			std::monostate, //FormFlag is 0x00
			MtpRangeForm, //FormFlag is 0x01 
			MtpEnumForm //FormFlag is 0x02
		> FORM;
	};
	typedef struct MtpDevicePropDescDS_ MtpDevicePropDescDS;

}