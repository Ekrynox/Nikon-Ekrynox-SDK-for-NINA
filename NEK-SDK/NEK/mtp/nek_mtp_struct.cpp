#include "nek_mtp_struct.hpp"



using namespace nek::mtp;



bool nek::mtp::TryGetInteger(const MtpDatatypeVariant& data, int64_t& value) {
	value = 0;
	return TryGetAs<int64_t, int8_t, int16_t, int32_t>(data, value);
}
bool nek::mtp::TryGetUInteger(const MtpDatatypeVariant& data, uint64_t& value) {
	value = 0;
	return TryGetAs<uint64_t, uint8_t, uint16_t, uint32_t>(data, value);
}
bool nek::mtp::TryGetArrayInteger(const MtpDatatypeVariant& data, std::vector<int64_t>& value) {
	value = std::vector<int64_t>(0);
	return TryGetAsArray<int64_t, int8_t, int16_t, int32_t>(data, value);
}
bool nek::mtp::TryGetArrayUInteger(const MtpDatatypeVariant& data, std::vector<uint64_t>& value) {
	value = std::vector<uint64_t>(0);
	return TryGetAsArray<uint64_t, uint8_t, uint16_t, uint32_t>(data, value);
}


bool MtpDevicePropDescDSV::TryGetInteger(MtpDevicePropDescDS<int64_t>& desc) {
	desc = MtpDevicePropDescDS<int64_t>();

	if ((this->DataType != MtpDatatypeCode::Int8) && (this->DataType != MtpDatatypeCode::Int16) && (this->DataType != MtpDatatypeCode::Int32) && (this->DataType != MtpDatatypeCode::Int64)) return false;
	if ((this->CurrentValue.index() == 1) && (this->DataType != MtpDatatypeCode::Int8)) return false;
	if ((this->CurrentValue.index() == 3) && (this->DataType != MtpDatatypeCode::Int16)) return false;
	if ((this->CurrentValue.index() == 5) && (this->DataType != MtpDatatypeCode::Int32)) return false;
	if ((this->CurrentValue.index() == 7) && (this->DataType != MtpDatatypeCode::Int64)) return false;
	bool res = true;

	desc.DevicePropertyCode = this->DevicePropertyCode;
	desc.DataType = this->DataType;
	desc.GetSet = this->GetSet;
	desc.FormFlag = this->FormFlag;

	res = res && nek::mtp::TryGetInteger(this->FactoryDefaultValue, desc.FactoryDefaultValue);
	res = res && nek::mtp::TryGetInteger(this->CurrentValue, desc.CurrentValue);

	if (this->FormFlag == MtpFormtypeCode::Range) {
		if (MtpRangeFormV* rangeVar = std::get_if<MtpRangeFormV>(&this->FORM)) {
			MtpRangeForm<int64_t> converted;
			res = res && nek::mtp::TryGetInteger(rangeVar->min, converted.min);
			res = res && nek::mtp::TryGetInteger(rangeVar->max, converted.max);
			res = res && nek::mtp::TryGetInteger(rangeVar->step, converted.step);

			desc.FORM = converted;
		}
	}
	else if (this->FormFlag == MtpFormtypeCode::Enum) {
		if (MtpEnumFormV* enumVar = std::get_if<MtpEnumFormV>(&this->FORM)) {
			MtpEnumForm<int64_t> converted = MtpEnumForm<int64_t>(0);
			for (MtpDatatypeVariant i : *enumVar) {
				int64_t v;
				if (nek::mtp::TryGetInteger(i, v)) {
					converted.push_back(v);
				}
				else {
					res = false;
				}
			}
			desc.FORM = converted;
		}
	}

	return res;
}
bool MtpDevicePropDescDSV::TryGetUInteger(MtpDevicePropDescDS<uint64_t>& desc) {
	desc = MtpDevicePropDescDS<uint64_t>();

	if ((this->DataType != MtpDatatypeCode::UInt8) && (this->DataType != MtpDatatypeCode::UInt16) && (this->DataType != MtpDatatypeCode::UInt32) && (this->DataType != MtpDatatypeCode::UInt64)) return false;
	if ((this->CurrentValue.index() == 2) && (this->DataType != MtpDatatypeCode::UInt8)) return false;
	if ((this->CurrentValue.index() == 4) && (this->DataType != MtpDatatypeCode::UInt16)) return false;
	if ((this->CurrentValue.index() == 6) && (this->DataType != MtpDatatypeCode::UInt32)) return false;
	if ((this->CurrentValue.index() == 8) && (this->DataType != MtpDatatypeCode::UInt64)) return false;
	bool res = true;

	desc.DevicePropertyCode = this->DevicePropertyCode;
	desc.DataType = this->DataType;
	desc.GetSet = this->GetSet;
	desc.FormFlag = this->FormFlag;

	res = res && nek::mtp::TryGetUInteger(this->FactoryDefaultValue, desc.FactoryDefaultValue);
	res = res && nek::mtp::TryGetUInteger(this->CurrentValue, desc.CurrentValue);

	if (this->FormFlag == MtpFormtypeCode::Range) {
		if (MtpRangeFormV* rangeVar = std::get_if<MtpRangeFormV>(&this->FORM)) {
			MtpRangeForm<uint64_t> converted;
			res = res && nek::mtp::TryGetUInteger(rangeVar->min, converted.min);
			res = res && nek::mtp::TryGetUInteger(rangeVar->max, converted.max);
			res = res && nek::mtp::TryGetUInteger(rangeVar->step, converted.step);

			desc.FORM = converted;
		}
	}
	else if (this->FormFlag == MtpFormtypeCode::Enum) {
		if (MtpEnumFormV* enumVar = std::get_if<MtpEnumFormV>(&this->FORM)) {
			MtpEnumForm<uint64_t> converted = MtpEnumForm<uint64_t>(0);
			for (MtpDatatypeVariant i : *enumVar) {
				uint64_t v;
				if (nek::mtp::TryGetUInteger(i, v)) {
					converted.push_back(v);
				}
				else {
					res = false;
				}
			}
			desc.FORM = converted;
		}
	}

	return res;
}
bool MtpDevicePropDescDSV::TryGetArrayInteger(MtpDevicePropDescDS<std::vector<int64_t>>& desc) {
	desc = MtpDevicePropDescDS<std::vector<int64_t>>();

	if ((this->DataType != MtpDatatypeCode::ArrayInt8) && (this->DataType != MtpDatatypeCode::ArrayInt16) && (this->DataType != MtpDatatypeCode::ArrayInt32) && (this->DataType != MtpDatatypeCode::ArrayInt64)) return false;
	if ((this->CurrentValue.index() == 11) && (this->DataType != MtpDatatypeCode::ArrayInt8)) return false;
	if ((this->CurrentValue.index() == 13) && (this->DataType != MtpDatatypeCode::ArrayInt16)) return false;
	if ((this->CurrentValue.index() == 15) && (this->DataType != MtpDatatypeCode::ArrayInt32)) return false;
	if ((this->CurrentValue.index() == 17) && (this->DataType != MtpDatatypeCode::ArrayInt64)) return false;
	bool res = true;

	desc.DevicePropertyCode = this->DevicePropertyCode;
	desc.DataType = this->DataType;
	desc.GetSet = this->GetSet;
	desc.FormFlag = this->FormFlag;

	res = res && nek::mtp::TryGetArrayInteger(this->FactoryDefaultValue, desc.FactoryDefaultValue);
	res = res && nek::mtp::TryGetArrayInteger(this->CurrentValue, desc.CurrentValue);

	if (this->FormFlag == MtpFormtypeCode::Range) {
		if (MtpRangeFormV* rangeVar = std::get_if<MtpRangeFormV>(&this->FORM)) {
			MtpRangeForm<std::vector<int64_t>> converted;
			res = res && nek::mtp::TryGetArrayInteger(rangeVar->min, converted.min);
			res = res && nek::mtp::TryGetArrayInteger(rangeVar->max, converted.max);
			res = res && nek::mtp::TryGetArrayInteger(rangeVar->step, converted.step);

			desc.FORM = converted;
		}
	}
	else if (this->FormFlag == MtpFormtypeCode::Enum) {
		if (MtpEnumFormV* enumVar = std::get_if<MtpEnumFormV>(&this->FORM)) {
			MtpEnumForm<std::vector<int64_t>> converted = MtpEnumForm<std::vector<int64_t>>(0);
			for (MtpDatatypeVariant i : *enumVar) {
				std::vector<int64_t> v;
				if (nek::mtp::TryGetArrayInteger(i, v)) {
					converted.push_back(v);
				}
				else {
					res = false;
				}
			}
			desc.FORM = converted;
		}
	}

	return res;
}
bool MtpDevicePropDescDSV::TryGetArrayUInteger(MtpDevicePropDescDS<std::vector<uint64_t>>& desc) {
	desc = MtpDevicePropDescDS<std::vector<uint64_t>>();

	if ((this->DataType != MtpDatatypeCode::ArrayUInt8) && (this->DataType != MtpDatatypeCode::ArrayUInt16) && (this->DataType != MtpDatatypeCode::ArrayUInt32) && (this->DataType != MtpDatatypeCode::ArrayUInt64)) return false;
	if ((this->CurrentValue.index() == 12) && (this->DataType != MtpDatatypeCode::ArrayUInt8)) return false;
	if ((this->CurrentValue.index() == 14) && (this->DataType != MtpDatatypeCode::ArrayUInt16)) return false;
	if ((this->CurrentValue.index() == 16) && (this->DataType != MtpDatatypeCode::ArrayUInt32)) return false;
	if ((this->CurrentValue.index() == 18) && (this->DataType != MtpDatatypeCode::ArrayUInt64)) return false;
	bool res = true;

	desc.DevicePropertyCode = this->DevicePropertyCode;
	desc.DataType = this->DataType;
	desc.GetSet = this->GetSet;
	desc.FormFlag = this->FormFlag;

	res = res && nek::mtp::TryGetArrayUInteger(this->FactoryDefaultValue, desc.FactoryDefaultValue);
	res = res && nek::mtp::TryGetArrayUInteger(this->CurrentValue, desc.CurrentValue);

	if (this->FormFlag == MtpFormtypeCode::Range) {
		if (MtpRangeFormV* rangeVar = std::get_if<MtpRangeFormV>(&this->FORM)) {
			MtpRangeForm<std::vector<uint64_t>> converted;
			res = res && nek::mtp::TryGetArrayUInteger(rangeVar->min, converted.min);
			res = res && nek::mtp::TryGetArrayUInteger(rangeVar->max, converted.max);
			res = res && nek::mtp::TryGetArrayUInteger(rangeVar->step, converted.step);

			desc.FORM = converted;
		}
	}
	else if (this->FormFlag == MtpFormtypeCode::Enum) {
		if (MtpEnumFormV* enumVar = std::get_if<MtpEnumFormV>(&this->FORM)) {
			MtpEnumForm<std::vector<uint64_t>> converted = MtpEnumForm<std::vector<uint64_t>>(0);
			for (MtpDatatypeVariant i : *enumVar) {
				std::vector<uint64_t> v;
				if (nek::mtp::TryGetArrayUInteger(i, v)) {
					converted.push_back(v);
				}
				else {
					res = false;
				}
			}
			desc.FORM = converted;
		}
	}

	return res;
}