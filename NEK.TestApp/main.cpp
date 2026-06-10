#include "nikon.hpp"

#include <iostream>
#include <fstream>


using namespace std;


void onEvent(const nek::mtp::MtpEvent& event) {
	wcout << "Event: " << hex << "0x" << event.eventCode << " => [";
	for (size_t i = event.parameters.size(); i-- > 0;) {
		wcout << event.parameters[i];
		if (i != 0) wcout << ", ";
	}
	wcout << "]" << dec << endl;
}


int main() {
	size_t nbNikonCamera = nek::NikonCamera::countNikonCameras();
	cout << "Nikon Camera detected: " << nbNikonCamera << endl;
	if (nbNikonCamera == 0) return 0;

	auto nikonCameras = nek::NikonCamera::getNikonCameras();
	for (auto& pair : nikonCameras) {
		wcout << std::get<1>(pair).Manufacture << " " << std::get<1>(pair).Model << " " << std::get<1>(pair).SerialNumber << endl;
	}
	if (nikonCameras.size() == 0) return 0;


	std::unique_ptr<nek::NikonCamera> camT = std::move(std::get<2>(*nikonCameras.begin()));
	camT->Connect();
	size_t callbackId = camT->RegisterCallback(onEvent);
	
	auto res = camT->GetDeviceInfo();
	res;

	char c = 0;
	do {
		c = getchar();
	} while (c != 'q');

	camT->UnregisterCallback(callbackId);
	camT->Disconnect();

	return 0;
}