using NINA.Core.Enum;
using NINA.Core.Model.Equipment;
using NINA.Core.Utility;
using NINA.Equipment.Interfaces;
using NINA.Equipment.Model;
using NINA.Image.Interfaces;
using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;



namespace LucasAlias.NINA.NEK.NEKDrivers {
    public class NikonCameraNek : BaseINPC, ICamera {

        public NikonCameraNek(string devicePath, NEKCS.NikonDeviceInfoDS cameraInfo) {
            this.devicePath = devicePath;
            this.cameraInfo = cameraInfo;
        }

        private string devicePath; // WPD device path
        private NEKCS.NikonDeviceInfoDS cameraInfo; // GetDeviceInfo
        private NEKCS.NikonCamera camera; // Camera object for operations



        // IDevice
        public bool HasSetupDialog { get => false; } // TODO
        public string Id { get => cameraInfo.SerialNumber; }
        public string Name { get => "Nikon " + cameraInfo.Model; }
        public string DisplayName { get => "Nikon " + cameraInfo.Model + " (NEK Experimental)"; }
        public string Category { get => "Nikon"; }
        public bool Connected {
            get {
                if (camera == null) {
                    return false;
                }
                return camera.isConnected();
            }
        }
        public string Description { get => throw new NotImplementedException(); }
        public string DriverInfo { get => "Nikon Ekrynox SDK"; }
        public string DriverVersion { get => cameraInfo.DeviceVersion; }


        public Task<bool> Connect(CancellationToken token) { //TODO: Add error management
            return Task.Run(() => {
                try {
                    this.camera = new NEKCS.NikonCamera(devicePath, 1);
                    if (!this.camera.isConnected()) {
                        return false;
                    }

                    this.cameraInfo = this.camera.GetDeviceInfo();

                    return this.camera.isConnected();
                } catch (Exception ex) {
                    // Handle connection errors
                    Console.WriteLine($"Error connecting to Nikon camera: {ex.Message}");
                }
                return false;
            }, token);
        }

        public void Disconnect() {
            if (this.camera != null) {
                this.camera.Dispose();
                this.camera = null;
            }
        }

        public void SetupDialog() { throw new NotImplementedException(); }

        public IList<string> SupportedActions { get => throw new NotImplementedException(); }

        public string Action(string actionName, string actionParameters) { throw new NotImplementedException(); }

        public string SendCommandString(string command, bool raw = true) { throw new NotImplementedException(); }

        public bool SendCommandBool(string command, bool raw = true) { throw new NotImplementedException(); }

        public void SendCommandBlind(string command, bool raw = true) { throw new NotImplementedException(); }




        // ICamera
        public bool HasShutter { get => throw new NotImplementedException(); }
        public double Temperature { get => throw new NotImplementedException(); }
        public double TemperatureSetPoint { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public short BinX { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public short BinY { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public string SensorName { get => throw new NotImplementedException(); }
        public SensorType SensorType { get => throw new NotImplementedException(); }
        public short BayerOffsetX { get => throw new NotImplementedException(); }
        public short BayerOffsetY { get => throw new NotImplementedException(); }
        public int CameraXSize { get => throw new NotImplementedException(); }
        public int CameraYSize { get => throw new NotImplementedException(); }
        public double ExposureMin { get => throw new NotImplementedException(); }
        public double ExposureMax { get => throw new NotImplementedException(); }
        public short MaxBinX { get => throw new NotImplementedException(); }
        public short MaxBinY { get => throw new NotImplementedException(); }
        public double PixelSizeX { get => throw new NotImplementedException(); }
        public double PixelSizeY { get => throw new NotImplementedException(); }
        public bool CanSetTemperature { get => throw new NotImplementedException(); }
        public bool CoolerOn { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public double CoolerPower { get => throw new NotImplementedException(); }
        public bool HasDewHeater { get => throw new NotImplementedException(); }
        public bool DewHeaterOn { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public CameraStates CameraState { get => throw new NotImplementedException(); }
        public bool CanSubSample { get => throw new NotImplementedException(); }
        public bool EnableSubSample { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public int SubSampleX { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public int SubSampleY { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public int SubSampleWidth { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public int SubSampleHeight { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public bool CanShowLiveView { get => throw new NotImplementedException(); }
        public bool LiveViewEnabled { get => throw new NotImplementedException(); }
        public bool HasBattery { get => throw new NotImplementedException(); }
        public int BatteryLevel { get => throw new NotImplementedException(); }
        public int BitDepth { get => throw new NotImplementedException(); }

        public bool CanSetOffset { get => throw new NotImplementedException(); }
        public int Offset { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public int OffsetMin { get => throw new NotImplementedException(); }
        public int OffsetMax { get => throw new NotImplementedException(); }
        public bool CanSetUSBLimit { get => throw new NotImplementedException(); }
        public int USBLimit { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public int USBLimitMin { get => throw new NotImplementedException(); }
        public int USBLimitMax { get => throw new NotImplementedException(); }
        public int USBLimitStep { get => throw new NotImplementedException(); }
        public bool CanGetGain { get => throw new NotImplementedException(); }
        public bool CanSetGain { get => throw new NotImplementedException(); }
        public int GainMax { get => throw new NotImplementedException(); }
        public int GainMin { get => throw new NotImplementedException(); }
        public int Gain { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public double ElectronsPerADU { get => throw new NotImplementedException(); }
        public IList<string> ReadoutModes { get => throw new NotImplementedException(); }
        public short ReadoutMode { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public short ReadoutModeForSnapImages { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
        public short ReadoutModeForNormalImages { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }

        public IList<int> Gains { get => throw new NotImplementedException(); }

        public AsyncObservableCollection<BinningMode> BinningModes { get => throw new NotImplementedException(); }

        public void SetBinning(short x, short y) { throw new NotImplementedException(); }

        public void StartExposure(CaptureSequence sequence) { throw new NotImplementedException(); }

        public Task WaitUntilExposureIsReady(CancellationToken token) { throw new NotImplementedException(); }

        public void StopExposure() { throw new NotImplementedException(); }

        public void AbortExposure() { throw new NotImplementedException(); }

        public Task<IExposureData> DownloadExposure(CancellationToken token) { throw new NotImplementedException(); }

        public void StartLiveView(CaptureSequence sequence) { throw new NotImplementedException(); }

        public Task<IExposureData> DownloadLiveView(CancellationToken token) { throw new NotImplementedException(); }

        public void StopLiveView() { throw new NotImplementedException(); }

        public void UpdateSubSampleArea() { throw new NotImplementedException(); }

    }
}
