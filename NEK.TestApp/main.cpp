#include <nikon.hpp>
#include <iostream>
#include <fstream>


using namespace std;


static nek::NikonCamera *camera;



uint32_t handle = 0xFFFF0001;

static void eventFunc (nek::mtp::MtpEvent event) {
	cout << "event: " << std::hex << event.eventCode << " [ ";
	for (uint32_t p : event.eventParams) {
		cout << std::hex << p << " ";
	}
	cout << "]" << std::endl;

	if (event.eventCode == nek::NikonMtpEventCode::ObjectAddedInSdram) {
		if (event.eventParams.size() > 0 && event.eventParams[0] != 0) {
			handle = event.eventParams[0];
		}

		auto info = camera->GetObjectInfo(handle);

		auto params = nek::mtp::MtpParams();
		params.addUint32(handle);
		nek::mtp::MtpResponse result = camera->SendCommandAndRead(nek::NikonMtpOperationCode::GetObject, params);
		if (result.responseCode == nek::NikonMtpResponseCode::OK) {
			ofstream MyFile("image.NEF", ios::out | ios::binary);
			MyFile.write((char*)result.data.data(), result.data.size());
			MyFile.close();
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
		camera->SendCommand(nek::NikonMtpOperationCode::ChangeCameraMode, params);
	}

	{
		auto params = nek::mtp::MtpParams();
		params.addUint32(0);
		camera->SendCommand(nek::NikonMtpOperationCode::DeleteImagesInSdram, params);
	}

	auto propdesc = camera->GetDevicePropDesc(nek::NikonMtpDevicePropCode::ExposureIndex);
	nek::mtp::MtpDevicePropDescDS<uint16_t> desc;
	auto descok = propdesc.TryGet(desc);

	camera->SetDevicePropValueTypesafe(nek::NikonMtpDevicePropCode::ExposureIndex, (uint64_t)1600);
	auto propvalue1 = camera->GetDevicePropValue(nek::NikonMtpDevicePropCode::ExposureIndex);

	auto propvalue2 = camera->GetDevicePropValue(nek::NikonMtpDevicePropCode::ExposureIndex);

	camera->SetDevicePropValueTypesafe(nek::NikonMtpDevicePropCode::ExposureIndex, (uint64_t)400);
	auto propvalue3 = camera->GetDevicePropValue(nek::NikonMtpDevicePropCode::ExposureIndex);


	auto a = camera->GetDevicePropDesc(nek::NikonMtpDevicePropCode::TimeCodeOrigin);
	nek::mtp::MtpDevicePropDescDS<std::vector<uint64_t>> b;
	auto c = a.TryGetArrayUInteger(b);

	camera->SetDevicePropValueTypesafe(nek::NikonMtpDevicePropCode::TimeCodeOrigin, std::vector<uint8_t>{2, 0, 0, 0, 0} );

	int wait = 1;
	while (true) {
		cin >> wait;
		if (wait == 0) break;

		
		auto params = nek::mtp::MtpParams();
		params.addUint32(0xFFFFFFFF);
		nek::mtp::MtpResponse result;
		try {
			result = camera->SendCommand(nek::NikonMtpOperationCode::InitiateCaptureRecInSdram, params);
		}
		catch (const nek::mtp::MtpDeviceException& e) {
			if (e.code == nek::mtp::MtpExCode::DEVICE_DISCONNECTED) {
				break;
			}
			throw;
		}
		cout << std::hex << "Response code: " << result.responseCode << endl;
		
	}

	delete camera;

	return 0;
}