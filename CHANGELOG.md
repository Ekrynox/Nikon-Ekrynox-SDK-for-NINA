# Nikon Ekrynox SDK

## 1.2.0.0
- Add Docakable to modify MTP Device Properties
- Add support for HostMode
- Fix issues when cammera is abruptly diconnected


## 1.1.0.0
- Add Support to LensAF Focus Position setting
- Generate a Name for CPU Lens


## 1.0.0.0
- Complete the Database for D3XXX serie, and fix some errors in Ids
- Improve Disconnection handling
- Improve Settings UI, Information Notifications, and Description
- Official Release


## 0.4.0.0
- Add a Database for Sensor Informations: Nb Pixels, Pixel's size, ...
- Add detection of Bit depth for FreeImage
- Add detection of Crop Area
- First Iteration of button to rerun Focuser Calibration
- First Iteration of Settings UI for the plugin
- Rework of Focuser code


## 0.3.7.0
- Some fixes & optimizations on the focuser


## 0.3.6.0
- Acquire device info only on Nikon devices
- Fix Operation codes & Property codes that weren't stored as 32bits


## 0.3.5.0
- Fix error in type error message for propdesc
- Potential Fix for misaligned data fetching in GetDevicePropDescEx


## 0.3.4.0
- Fix Gains selection in sequencer


## 0.3.3.0
- Fix Loop due to int converion of division in Focuser calibration
- Start Liveview to prevent AF when taking pictures
- Switch to new version of NEK and use of Smart Type for Device Properties


## 0.3.2.0
- Focuser doesn't need to refresh the list after being disconnected


## 0.3.1.0
- Ensure focuser null state at disconnect
- Fix liveview being too dark
- Refactor liveview state counter & ensure liveview end at disconnect


## 0.3.0.0
- Fix some Bugs
- Information Notifications
- Logging & Error handling


## 0.2.1.0
- Exposure time revert back after capture
- Fix breaking bug on exposure setting


## 0.2.0.0
- Focuser for compatible focus lenses
- In camera Bulb


## 0.1.0.0
- Connection to Nikon Camera
- Image Capture
- Initial release
- Liveview
- Set Gain & Exposure
