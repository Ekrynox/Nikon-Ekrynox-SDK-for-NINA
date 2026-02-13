using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

[assembly: System.Windows.ThemeInfo(System.Windows.ResourceDictionaryLocation.None, System.Windows.ResourceDictionaryLocation.SourceAssembly)]

// [MANDATORY] The following GUID is used as a unique identifier of the plugin. Generate a fresh one for your plugin!
[assembly: Guid("43a52070-20f3-43a1-b3bb-5a456f6ad988")]

// [MANDATORY] The assembly versioning
//Should be incremented for each new release build of a plugin
[assembly: AssemblyVersion("1.1.0.0")]
[assembly: AssemblyFileVersion("1.1.0.0")]

// [MANDATORY] The name of your plugin
[assembly: AssemblyTitle("Nikon Ekrynox SDK")]
// [MANDATORY] A short description of your plugin
[assembly: AssemblyDescription("A new Nikon SDK based on direct PTP/MTP call with Windows Portable API.")]

// The following attributes are not required for the plugin per se, but are required by the official manifest meta data

// Your name
[assembly: AssemblyCompany("Lucas Alias")]
// The product name that this plugin is part of
[assembly: AssemblyProduct("Nikon Ekrynox SDK")]
[assembly: AssemblyCopyright("Copyright © 2025 Lucas Alias")]

// The minimum Version of N.I.N.A. that this plugin is compatible with
[assembly: AssemblyMetadata("MinimumApplicationVersion", "3.2.0.9001")]

// The license your plugin code is using
[assembly: AssemblyMetadata("License", "MPL-2.0")]
// The url to the license
[assembly: AssemblyMetadata("LicenseURL", "https://www.mozilla.org/en-US/MPL/2.0/")]
// The repository where your pluggin is hosted
[assembly: AssemblyMetadata("Repository", "https://github.com/Ekrynox/Nikon-Ekrynox-SDK-for-NINA")]

// The following attributes are optional for the official manifest meta data

//[Optional] Your plugin homepage URL - omit if not applicaple
[assembly: AssemblyMetadata("Homepage", "https://github.com/Ekrynox/Nikon-Ekrynox-SDK-for-NINA")]

//[Optional] Common tags that quickly describe your plugin
[assembly: AssemblyMetadata("Tags", "Nikon")]

//[Optional] A link that will show a log of all changes in between your plugin's versions
[assembly: AssemblyMetadata("ChangelogURL", "https://github.com/Ekrynox/Nikon-Ekrynox-SDK-for-NINA/blob/main/CHANGELOG.md")]

//[Optional] The url to a featured logo that will be displayed in the plugin list next to the name
[assembly: AssemblyMetadata("FeaturedImageURL", "https://github.com/Ekrynox/Nikon-Ekrynox-SDK-for-NINA/blob/main/assets/images/NEF%20for%20NINA.png?raw=true")]
//[Optional] A url to an example screenshot of your plugin in action
[assembly: AssemblyMetadata("ScreenshotURL", "https://github.com/Ekrynox/Nikon-Ekrynox-SDK-for-NINA/blob/main/assets/images/Screenshot1.png?raw=true")]
//[Optional] An additional url to an example example screenshot of your plugin in action
[assembly: AssemblyMetadata("AltScreenshotURL", "https://github.com/Ekrynox/Nikon-Ekrynox-SDK-for-NINA/blob/main/assets/images/Screenshot2.png?raw=true")]
//[Optional] An in-depth description of your plugin
[assembly: AssemblyMetadata("LongDescription", @"
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
If you encounter an ""NEKCS.Wrapper"" not found, you need to install [Microsoft VC Redist X64](https://aka.ms/vs/17/release/vc_redist.x64.exe).

The focuser have some difficulties with older models like the D7100/D7200. It has difficulties to accurately detect the focus limit (0 and infinity), and overestimate the step range.

## Special Notes
Thanks to all the contributors of NINA for the great work done on the main application.  
Thanks to **LEX** for all the work on testing his cameras and providing feedback.")]

// Setting ComVisible to false makes the types in this assembly not visible
// to COM components.  If you need to access a type in this assembly from
// COM, set the ComVisible attribute to true on that type.
[assembly: ComVisible(false)]
// [Unused]
[assembly: AssemblyConfiguration("")]
// [Unused]
[assembly: AssemblyTrademark("")]
// [Unused]
[assembly: AssemblyCulture("")]