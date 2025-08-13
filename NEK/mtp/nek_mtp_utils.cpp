#include "nek_mtp_utils.hpp"

#include <stdexcept>

#include <propvarutil.h>



using namespace nek::mtp;



//MtpResponseParams
MtpReponseParams::MtpReponseParams() {
	pv_ = std::vector<PROPVARIANT>();
}

MtpReponseParams::MtpReponseParams(CComPtr<IPortableDevicePropVariantCollection> paramsCollection) {
	pv_ = std::vector<PROPVARIANT>();
	SetCollection(paramsCollection);
}

MtpReponseParams::MtpReponseParams(CComPtr<IPortableDeviceValues> eventParameters) {
	pv_ = std::vector<PROPVARIANT>();
	SetCollection(eventParameters);
}

MtpReponseParams::MtpReponseParams(const MtpReponseParams& other) {
	pv_ = std::vector<PROPVARIANT>();
	for (const auto& pv : other.pv_) {
		PROPVARIANT newPv;
		PropVariantInit(&newPv);
		PropVariantCopy(&newPv, &pv);
		pv_.push_back(newPv);
	}
}

MtpReponseParams& MtpReponseParams::operator=(const MtpReponseParams& other) {
	if (this != &other) {
		for (auto& pv : pv_) {
			PropVariantClear(&pv);
		}
		pv_.clear();
		for (const auto& pv : other.pv_) {
			PROPVARIANT newPv;
			PropVariantInit(&newPv);
			PropVariantCopy(&newPv, &pv);
			this->pv_.push_back(newPv);
		}
	}
	return *this;
}

MtpReponseParams::~MtpReponseParams() {
	for (auto& pv : pv_) {
		PropVariantClear(&pv);
	}
	pv_.clear();
}

void MtpReponseParams::SetCollection(CComPtr<IPortableDevicePropVariantCollection> paramsCollection) {
	pv_.clear();
	DWORD size = 0;
	paramsCollection->GetCount(&size);
	for (DWORD i = 0; i < size; i++) {
		PROPVARIANT pv;
		PropVariantInit(&pv);
		paramsCollection->GetAt(i, &pv);
		pv_.push_back(pv);
	}
}

void MtpReponseParams::SetCollection(CComPtr<IPortableDeviceValues> eventParameters) {
	pv_.clear();
	DWORD size = 0;
	eventParameters->GetCount(&size);
	for (DWORD i = 0; i < size; i++) {
		PROPERTYKEY pk;
		PROPVARIANT pv;
		PropVariantInit(&pv);
		eventParameters->GetAt(i, &pk, &pv);
		pv_.push_back(pv);
	}
}



//MtpParams
MtpParams::~MtpParams() {
	for (auto& pv : pv_) {
		PropVariantClear(&pv);
	}
	pv_.clear();
}

CComPtr<IPortableDevicePropVariantCollection> MtpParams::GetCollection() {
	CComPtr<IPortableDevicePropVariantCollection> paramsCollection;
	HRESULT hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&paramsCollection));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create Prop Variant Collection: " + hr);
	}
	for (const auto& pv : pv_) {
		paramsCollection->Add(&pv);
	}

	return paramsCollection;
}

void MtpParams::addUint32(uint32_t value) {
	PROPVARIANT pv;
	InitPropVariantFromUInt32(value, &pv);
	pv_.push_back(pv);
}

void MtpParams::addUint16(uint16_t value) {
	PROPVARIANT pv;
	InitPropVariantFromUInt16(value, &pv);
	pv_.push_back(pv);
}

void MtpParams::addInt32(int32_t value) {
	PROPVARIANT pv;
	InitPropVariantFromInt32(value, &pv);
	pv_.push_back(pv);
}

void MtpParams::addInt16(int16_t value) {
	PROPVARIANT pv;
	InitPropVariantFromInt16(value, &pv);
	pv_.push_back(pv);
}



//MtpResponse
MtpResponse::MtpResponse() {
	responseCode = 0;
}

MtpReponseParams& MtpResponse::GetParams() {
	return responseParams_;
}



//MtpEvent
MtpEvent::MtpEvent(uint16_t eventCode) {
	this->eventCode = eventCode;
}
MtpEvent::MtpEvent(uint16_t eventCode, uint32_t param) {
	this->eventCode = eventCode;
	this->eventParams.push_back(param);
}
MtpEvent::MtpEvent(uint16_t eventCode, std::vector<uint32_t> params) {
	this->eventCode = eventCode;
	this->eventParams = params;
}



//MtpEventCallback
MtpEventCallback::MtpEventCallback() : ref_(0), nextId(0) {
	callbacks_ = new std::vector<std::pair<size_t, std::function<void(MtpEvent)>>>();
}

MtpEventCallback::~MtpEventCallback() {
	delete callbacks_;
}

HRESULT MtpEventCallback::OnEvent(IPortableDeviceValues* pEventParameters) {
	MtpEvent event = MtpEvent(0); //TO UPDATE TO EXTRACT DATA
	return OnEvent(event);
}

HRESULT MtpEventCallback::OnEvent(MtpEvent event) {
	mutex_.lock();
	auto callbacks = *callbacks_;
	mutex_.unlock();
	for (auto& [id, callback] : callbacks) {
		callback(event);
	}
	return HRESULT(0);
}

HRESULT MtpEventCallback::QueryInterface(REFIID riid, void** ppv) {
	static const QITAB qitab[] = { QITABENT(MtpEventCallback, IPortableDeviceEventCallback), { }, };
	return QISearch(this, qitab, riid, ppv);
}

ULONG MtpEventCallback::AddRef() {
	return InterlockedIncrement(&ref_);
}

ULONG MtpEventCallback::Release() {
	ULONG ref = _InterlockedDecrement(&ref_);
	if (ref == 0) {
		delete this;
	}

	return ref;
}


size_t MtpEventCallback::RegisterCallback(std::function<void(MtpEvent)> callback) {
	std::lock_guard<std::mutex> lock(mutex_);

	size_t id = nextId++;
	callbacks_->emplace_back(id, callback);
	return id;
}

void MtpEventCallback::UnregisterCallback(size_t id) {
	std::lock_guard<std::mutex> lock(mutex_);

	callbacks_->erase(std::remove_if(callbacks_->begin(), callbacks_->end(), [id](const auto& pair) { return pair.first == id; }), callbacks_->end());
}
