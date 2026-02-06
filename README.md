# Nikon Ekrynox SDK for NINA
This project is a [NINA](https://github.com/isbeorn/nina) plugin that brings a new alternative to the official Nikon SDK.  
Based on [Nikon Ekrynox SDK (NEK)](https://github.com/Ekrynox/Nikon-Ekrynox-SDK), the plugin doesn't require you to install any custom drivers as it uses Windows Portable Device API for direct PTP/MTP communications.

## Supported Cameras
Directly connecting to the camera through MTP/PTP and Windows Portable Device API, it should allow support for some unsupported cameras (as long as their MTP/PTP API remains similar to that of other cameras).  
**Repported working models:**
- Nikon D80
- Nikon D3500
- Nikon D7100
- Nikon D7200
- Nikon Z6 II
- Nikon Z50 II

## Features
- Capture (Bulb: USB only)
- Live View
- Focus Control for AF Lenses (Inbuilt NEK Focuser) (Camera must support Liveview) + Support for LensAF Liveview UI and Focus Point Setting
- Auto update of Focal Length/Ratio for CPU Lenses

## Known Issues
If you encounter an "NEKCS.Wrapper" not found, you need to install [Microsoft VC Redist X64](https://aka.ms/vs/17/release/vc_redist.x64.exe).

The focuser have some difficulties with older models like the D7100/D7200. It has difficulties to accurately detect the focus limit (0 and infinity), and overestimate the step range.

## Special Notes
Thanks to all the contributors of NINA for the great work done on the main application.  
Thanks to **LEX** for all the work on testing his cameras and providing feedback.
