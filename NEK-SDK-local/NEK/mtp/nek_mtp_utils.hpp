#pragma once
#include "../nek.hpp"

#include <future>
#include <vector>

#include <atlbase.h>
#include <PortableDeviceApi.h>
#include <PortableDevice.h>
#include <WpdMtpExtensions.h>
#include <functional>



namespace nek::mtp {

	class MtpReponseParams {
	public:
		NEK_API MtpReponseParams();
		NEK_API MtpReponseParams(CComPtr<IPortableDevicePropVariantCollection> paramsCollection);
		NEK_API MtpReponseParams(CComPtr<IPortableDeviceValues> eventParameters);
		NEK_API MtpReponseParams(const MtpReponseParams& other);
		NEK_API MtpReponseParams& operator=(const MtpReponseParams& other);
		NEK_API ~MtpReponseParams();

		NEK_API void SetCollection(CComPtr<IPortableDevicePropVariantCollection> paramsCollection);
		NEK_API void SetCollection(CComPtr<IPortableDeviceValues> eventParameters);

		/*uint32_t getUint32(size_t pos);
		uint16_t getUint16(size_t param);
		int32_t getInt32(size_t pos);
		int16_t getInt16(size_t pos);*/

	protected:
		std::vector<PROPVARIANT> pv_;
	};


	class MtpParams {
	public:
		NEK_API MtpParams() {};
		NEK_API ~MtpParams();

		NEK_API CComPtr<IPortableDevicePropVariantCollection> GetCollection();

		NEK_API void addUint32(uint32_t param);
		NEK_API void addUint16(uint16_t param);
		NEK_API void addInt32(int32_t param);
		NEK_API void addInt16(int16_t param);

	protected:
		std::vector<PROPVARIANT> pv_;
	};


	class MtpResponse {
	public:
		NEK_API MtpResponse();

		NEK_API MtpReponseParams& GetParams();

		uint16_t responseCode;
		std::vector<uint8_t> data;

	private:
		MtpReponseParams responseParams_;
	};


	class MtpEvent {
	public:
		NEK_API MtpEvent(uint16_t eventCode);
		NEK_API MtpEvent(uint16_t eventCode, uint32_t param);
		NEK_API MtpEvent(uint16_t eventCode, std::vector<uint32_t> params);

		uint16_t eventCode;
		std::vector<uint32_t> eventParams;
	};


	class MtpEventCallback : public IPortableDeviceEventCallback {
	public:
		MtpEventCallback();
		~MtpEventCallback();

		HRESULT STDMETHODCALLTYPE OnEvent(IPortableDeviceValues* pEventParameters);
		HRESULT STDMETHODCALLTYPE OnEvent(MtpEvent event);
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
		ULONG STDMETHODCALLTYPE AddRef();
		ULONG STDMETHODCALLTYPE Release();

		size_t RegisterCallback(std::function<void(MtpEvent)> callback);
		void UnregisterCallback(size_t id);

	private:
		ULONG ref_;

		std::mutex mutex_;
		size_t nextId;
		std::vector<std::pair<size_t, std::function<void(MtpEvent)>>>* callbacks_;
	};

}