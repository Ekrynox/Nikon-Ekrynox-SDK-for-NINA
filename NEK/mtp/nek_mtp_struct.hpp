#pragma once
#include "../nek.hpp"
#include "../utils/nek_int128.hpp"
#include "nek_mtp_enum.hpp"

#include <string>
#include <variant>
#include <vector>
#include <cstdint>
#include <utility>



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
	NEK_API bool TryGetInteger(const MtpDatatypeVariant& data, int64_t& value);
	NEK_API bool TryGetUInteger(const MtpDatatypeVariant& data, uint64_t& value);
	NEK_API bool TryGetArrayInteger(const MtpDatatypeVariant& data, std::vector<int64_t>& value);
	NEK_API bool TryGetArrayUInteger(const MtpDatatypeVariant& data, std::vector<uint64_t>& value);

	template<typename Target, typename... Sources> bool TryGetAs(const MtpDatatypeVariant& data, MtpDatatypeVariant& newdata) {
		if (auto* d = std::get_if<Target>(&data)) {
			newdata = *d;
			return true;
		}
		return (([&] {
			if (auto* d = std::get_if<Sources>(&data)) {
				if (!std::in_range<Target>(*d)) return false;
				newdata = static_cast<Target>(*d);
				return true;
			}
			return false;
			} ()) || ...);
	}
	template<typename Target, typename... Sources> bool TryGetAs(const MtpDatatypeVariant& data, Target& newdata) {
		if (auto* d = std::get_if<Target>(&data)) {
			newdata = *d;
			return true;
		}
		return (([&] {
			if (auto* d = std::get_if<Sources>(&data)) {
				if (!std::in_range<Target>(*d)) return false;
				newdata = static_cast<Target>(*d);
				return true;
			}
			return false;
			} ()) || ...);
	}
	template<typename Target, typename... Sources> bool TryGetAsArray(const MtpDatatypeVariant& data, MtpDatatypeVariant& newdata) {
		if (auto* d = std::get_if<std::vector<Target>>(&data)) {
			newdata = *d;
			return true;
		}
		return (([&] {
			if (auto* d = std::get_if<std::vector<Sources>>(&data)) {
				auto newarr = std::vector<Target>(d->size());
				for (size_t i = 0; i < d->size(); i++) {
					if (!std::in_range<Target>(d->at(i))) return false;
					newarr[i] = static_cast<Target>(d->at(i));
				}
				newdata = newarr;
				return true;
			}
			return false;
			} ()) || ...);
	}
	template<typename Target, typename... Sources> bool TryGetAsArray(const MtpDatatypeVariant& data, std::vector<Target>& newdata) {
		if (auto* d = std::get_if<std::vector<Target>>(&data)) {
			newdata = *d;
			return true;
		}
		return (([&] {
			if (auto* d = std::get_if<std::vector<Sources>>(&data)) {
				newdata = std::vector<Target>(d->size());
				for (size_t i = 0; i < d->size(); i++) {
					if (!std::in_range<Target>(d->at(i))) return false;
					newdata[i] = static_cast<Target>(d->at(i));
				}

				return true;
			}
			return false;
			} ()) || ...);
	}

	
	template<typename T>
	struct MtpRangeForm_ {
		T min;
		T max;
		T step;
	};
	template<typename T> using MtpRangeForm = struct MtpRangeForm_<T>;
	template<typename T> using MtpEnumForm = std::vector<T>;
	using MtpRangeFormV = MtpRangeForm<MtpDatatypeVariant>;
	using MtpEnumFormV = MtpEnumForm<MtpDatatypeVariant>;

	template<typename T>
	struct MtpDevicePropDescDS_ {
		uint32_t DevicePropertyCode = 0; //uint16_t for the GetDevicePropDesc operation //uint32_t for the Nikon GetDevicePropDescEx operation
		uint16_t DataType = 0;
		uint8_t GetSet = 0; //0x00 read-only, 0x01 read-write
		T FactoryDefaultValue; //See DataType for the type of data stored here
		T CurrentValue; //See DataType for the type of data stored here
		uint8_t FormFlag = 0; //0x00 normal, 0x01 range, 0x02 enum

		std::variant<
			std::monostate, //FormFlag is 0x00
			MtpRangeForm<T>, //FormFlag is 0x01 
			MtpEnumForm<T> //FormFlag is 0x02
		> FORM;
	};
	template<typename T> using MtpDevicePropDescDS = struct MtpDevicePropDescDS_<T>;

	struct MtpDevicePropDescDSV_ : public MtpDevicePropDescDS<MtpDatatypeVariant> {
		template<typename T> bool TryGet(MtpDevicePropDescDS<T>& desc) {
			desc = MtpDevicePropDescDS<T>();

			if (!std::holds_alternative<T>(this->CurrentValue)) return false;
			if ((this->CurrentValue.index() == 1) && (this->DataType != MtpDatatypeCode::Int8)) return false;
			if ((this->CurrentValue.index() == 2) && (this->DataType != MtpDatatypeCode::UInt8)) return false;
			if ((this->CurrentValue.index() == 3) && (this->DataType != MtpDatatypeCode::Int16)) return false;
			if ((this->CurrentValue.index() == 4) && (this->DataType != MtpDatatypeCode::UInt16)) return false;
			if ((this->CurrentValue.index() == 5) && (this->DataType != MtpDatatypeCode::Int32)) return false;
			if ((this->CurrentValue.index() == 6) && (this->DataType != MtpDatatypeCode::UInt32)) return false;
			if ((this->CurrentValue.index() == 7) && (this->DataType != MtpDatatypeCode::Int64)) return false;
			if ((this->CurrentValue.index() == 8) && (this->DataType != MtpDatatypeCode::UInt64)) return false;
			if ((this->CurrentValue.index() == 9) && (this->DataType != MtpDatatypeCode::Int128)) return false;
			if ((this->CurrentValue.index() == 10) && (this->DataType != MtpDatatypeCode::UInt128)) return false;
			if ((this->CurrentValue.index() == 11) && (this->DataType != MtpDatatypeCode::ArrayInt8)) return false;
			if ((this->CurrentValue.index() == 12) && (this->DataType != MtpDatatypeCode::ArrayUInt8)) return false;
			if ((this->CurrentValue.index() == 13) && (this->DataType != MtpDatatypeCode::ArrayInt16)) return false;
			if ((this->CurrentValue.index() == 14) && (this->DataType != MtpDatatypeCode::ArrayUInt16)) return false;
			if ((this->CurrentValue.index() == 15) && (this->DataType != MtpDatatypeCode::ArrayInt32)) return false;
			if ((this->CurrentValue.index() == 16) && (this->DataType != MtpDatatypeCode::ArrayUInt32)) return false;
			if ((this->CurrentValue.index() == 17) && (this->DataType != MtpDatatypeCode::ArrayInt64)) return false;
			if ((this->CurrentValue.index() == 18) && (this->DataType != MtpDatatypeCode::ArrayUInt64)) return false;
			if ((this->CurrentValue.index() == 19) && (this->DataType != MtpDatatypeCode::ArrayInt128)) return false;
			if ((this->CurrentValue.index() == 20) && (this->DataType != MtpDatatypeCode::ArrayUInt128)) return false;
			if ((this->CurrentValue.index() == 21) && (this->DataType != MtpDatatypeCode::String)) return false;

			desc.DevicePropertyCode = this->DevicePropertyCode;
			desc.DataType = this->DataType;
			desc.GetSet = this->GetSet;
			desc.FormFlag = this->FormFlag;

			desc.FactoryDefaultValue = std::get<T>(this->FactoryDefaultValue);
			desc.CurrentValue = std::get<T>(this->CurrentValue);

			if (this->FormFlag == MtpFormtypeCode::Range) {
				if (MtpRangeFormV* rangeVar = std::get_if<MtpRangeFormV>(&this->FORM)) {
					MtpRangeForm<T> converted;
					converted.min = std::get<T>(rangeVar->min);
					converted.max = std::get<T>(rangeVar->max);
					converted.step = std::get<T>(rangeVar->step);

					desc.FORM = converted;
				}
			}
			else if (this->FormFlag == MtpFormtypeCode::Enum) {
				if (MtpEnumFormV* enumVar = std::get_if<MtpEnumFormV>(&this->FORM)) {
					MtpEnumForm<T> converted = MtpEnumForm<T>(0);
					for (MtpDatatypeVariant i : *enumVar) {
						converted.push_back(std::get<T>(i));
					}
					desc.FORM = converted;
				}
			}

			return true;
		}
		NEK_API bool TryGetInteger(MtpDevicePropDescDS<int64_t>& desc);
		NEK_API bool TryGetUInteger(MtpDevicePropDescDS<uint64_t>& desc);
		NEK_API bool TryGetArrayInteger(MtpDevicePropDescDS<std::vector<int64_t>>& desc);
		NEK_API bool TryGetArrayUInteger(MtpDevicePropDescDS<std::vector<uint64_t>>& desc);
	};
	using MtpDevicePropDescDSV = struct MtpDevicePropDescDSV_;

}