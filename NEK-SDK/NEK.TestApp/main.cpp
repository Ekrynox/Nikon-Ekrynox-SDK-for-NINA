#include <nikon.hpp>
#include <iostream>
#include <fstream>


using namespace std;


static nek::NikonCamera *camera;


void eventFunc (nek::mtp::MtpEvent event) {
	cout << "event: " << std::hex << event.eventCode << " [ ";
	for (uint32_t p : event.eventParams) {
		cout << std::hex << p << " ";
	}
	cout << "]" << std::endl;

	if (event.eventCode == nek::NikonMtpEventCode::ObjectAddedInSdram) {
		auto params = nek::mtp::MtpParams();
		if (event.eventParams.size() > 0 && event.eventParams[0] != 0) {
			params.addUint32(event.eventParams[0]);
		}
		else {
			params.addUint32(0xFFFF0001);
		}
		//nek::mtp::MtpResponse result = camera->SendCommandAndRead(nek::NikonMtpOperationCode::GetObjectInfo, params);
		//result = camera->SendCommandAndRead(nek::NikonMtpOperationCode::GetThumb, params);
		nek::mtp::MtpResponse result = camera->SendCommandAndRead(nek::NikonMtpOperationCode::GetObject, params);
		if (result.responseCode == nek::NikonMtpResponseCode::OK) {
			ofstream MyFile("image.NEF", ios::out | ios::binary);
			MyFile.write((char*)result.data.data(), result.data.size());
			MyFile.close();
		}

		{
			auto params = nek::mtp::MtpParams();
			params.addUint32(nek::NikonMtpDevicePropCode::Fnumber);
			vector<BYTE> data;
			data.resize(sizeof(uint16_t));
			uint16_t fnum = 450;
			memcpy(data.data(), &fnum, sizeof(uint16_t));
			nek::mtp::MtpResponse result = camera->SendCommandAndWrite(nek::NikonMtpOperationCode::SetDevicePropValue, params, data);
			cout << std::hex << "Response code: " << result.responseCode << endl;
		}
	}

	return;
}


int main() {
	size_t nbNikonCamera = nek::NikonCamera::countNikonCameras();
	cout << "Nikon Camera detected: " << nbNikonCamera << endl;
	if (nbNikonCamera == 0) return 0;

	auto nikonCameras = nek::NikonCamera::listNikonCameras();
	for (auto& cameraInfo : nikonCameras) {
		wcout << cameraInfo.second.Manufacture << " " << cameraInfo.second.Model << " " << cameraInfo.second.SerialNumber << endl;
	}

	camera = new nek::NikonCamera(nikonCameras.begin()->first, 1);
	camera->RegisterCallback(eventFunc);

	{
		auto params = nek::mtp::MtpParams();
		params.addUint32(0);
		camera->SendCommand(nek::NikonMtpOperationCode::DeleteImagesInSdram, params);
	}

	auto propdesc = camera->GetDevicePropDesc(nek::NikonMtpDevicePropCode::ExposureIndex);

	camera->SetDevicePropValue(nek::NikonMtpDevicePropCode::ExposureIndex, (uint16_t)1250);
	auto propvalue1 = camera->GetDevicePropValue(nek::NikonMtpDevicePropCode::ExposureIndex);

	auto propvalue2 = camera->GetDevicePropValue(nek::NikonMtpDevicePropCode::ExposureIndex);

	camera->SetDevicePropValue(nek::NikonMtpDevicePropCode::ExposureIndex, (uint16_t)100);
	auto propvalue3 = camera->GetDevicePropValue(nek::NikonMtpDevicePropCode::ExposureIndex);


	int wait = 1;
	while (true) {
		cin >> wait;
		if (wait == 0) break;

		{
			auto params = nek::mtp::MtpParams();
			params.addUint32(nek::NikonMtpDevicePropCode::Fnumber);
			vector<BYTE> data;
			data.resize(sizeof(uint16_t));
			uint16_t fnum = 1100;
			memcpy(data.data(), &fnum, sizeof(uint16_t));
			nek::mtp::MtpResponse result = camera->SendCommandAndWrite(nek::NikonMtpOperationCode::SetDevicePropValue, params, data);
			cout << std::hex << "Response code: " << result.responseCode << endl;
		}

		{
			auto params = nek::mtp::MtpParams();
			params.addUint32(0xFFFFFFFF);
			nek::mtp::MtpResponse result = camera->SendCommand(nek::NikonMtpOperationCode::InitiateCaptureRecInSdram, params);
			cout << std::hex << "Response code: " << result.responseCode << endl;
		}
	}

	delete camera;

	return 0;
}