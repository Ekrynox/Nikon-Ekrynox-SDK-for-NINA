# Nikon Ekrynox SDK

## 0.4.0.0
- First Iteration of Settings UI for the plugin
- First Iteration of button to rerun Focuser Calibration
- Rework of Focuser code
- Add a Database for Sensor Informations: Nb Pixels, Pixel's size, ...
- Add detection of Crop Area
- Add detection of Bit depth for FreeImage


## 0.3.7.0
- Some fixes & optimizations on the focuser


## 0.3.6.0
- Fix Operation codes & Property codes that weren't stored as 32bits
- Acquire device info only on Nikon devices


## 0.3.5.0
- Potential Fix for misaligned data fetching in GetDevicePropDescEx
- Fix error in type error message for propdesc


## 0.3.4.0
- Fix Gains selection in sequencer


## 0.3.3.0
- Fix Loop due to int converion of division in Focuser calibration
- Start Liveview to prevent AF when taking pictures
- Switch to new version of NEK and use of Smart Type for Device Properties


## 0.3.2.0
- Focuser doesn't need to refresh the list after being disconnected


## 0.3.1.0
- Fix liveview being too dark
- Refactor liveview state counter & ensure liveview end at disconnect
- Ensure focuser null state at disconnect


## 0.3.0.0
- Fix some Bugs
- Logging & Error handling
- Information Notifications


## 0.2.1.0
- Fix breaking bug on exposure setting
- Exposure time revert back after capture


## 0.2.0.0
- In camera Bulb
- Focuser for compatible focus lenses


## 0.1.0.0
- Initial release
- Connection to Nikon Camera
- Set Gain & Exposure
- Image Capture
- Liveview
