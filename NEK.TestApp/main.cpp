#include "nikon.hpp"

#include <iostream>
#include <fstream>


using namespace std;


int main() {
	size_t nbNikonCamera = nek::NikonCamera::countNikonCameras();
	cout << "Nikon Camera detected: " << nbNikonCamera << endl;
	if (nbNikonCamera == 0) return 0;

	auto nikonCameras = nek::NikonCamera::getNikonCameras();
	for (auto& pair : nikonCameras) {
		wcout << pair.second.Manufacture << " " << pair.second.Model << " " << pair.second.SerialNumber << endl;
	}
	if (nikonCameras.size() == 0) return 0;


	auto camT = std::move(nikonCameras.begin()->first);
	camT.Connect();
	
	auto res = camT.GetDeviceInfo();
	res;

	return 0;
}