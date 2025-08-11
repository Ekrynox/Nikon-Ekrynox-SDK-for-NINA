using NEKCS;
using NINA.Core.Enum;
using NINA.Core.Model.Equipment;
using NINA.Core.Utility;
using NINA.Equipment.Interfaces;
using NINA.Equipment.Model;
using NINA.Image.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
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
        public string Description { get => "Your Nikon Camera"; }
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

        public IList<string> SupportedActions { get => new List<string>(); } //TODO

        public string Action(string actionName, string actionParameters) { throw new NotImplementedException(); }

        public string SendCommandString(string command, bool raw = true) { throw new NotImplementedException(); }

        public bool SendCommandBool(string command, bool raw = true) { throw new NotImplementedException(); }

        public void SendCommandBlind(string command, bool raw = true) { throw new NotImplementedException(); }




        // ICamera
        public bool HasShutter { get => true; } //TO RECHECK: not true for all camera => Z6/7 in Silence mode, Z8, ...
        public double Temperature { get => double.NaN; }
        public double TemperatureSetPoint { get; set; }
        public short BinX { get => 1; set {} } //TO RECHECK
        public short BinY { get => 1; set {} } //TO RECHECK
        public string SensorName { get => ""; } //TO RECHECK: doesn't seem easly feasible
        public SensorType SensorType { get => SensorType.RGGB; } //TO RECHECK: certainly that
        public short BayerOffsetX { get => 0; } //TO RECHECK
        public short BayerOffsetY { get => 0; } //TO RECHECK
        public int CameraXSize { get => 0; } //TODO
        public int CameraYSize { get => 0; } //TODO
        public double ExposureMin { 
            get {
                if (Connected) {
                    try {
                        var result = this.camera.GetDevicePropDesc(NEKCS.NikonMtpDevicePropCode.ExposureTime);
                        NikonDevicePropDescDS<UInt32> exp = new();
                        if (!result.TryGetUInt32(ref exp)) return double.NaN;
                        var exps = exp.EnumFORM.ToList();
                        exps.Remove(0xFFFFFFFF); //Bulb
                        exps.Remove(0xFFFFFFFD); //Time
                        return exps.Min() / 10000.0;
                    } catch {
                        throw;
                    }
                }
                return double.NaN;
            }
        }
        public double ExposureMax { //TODO: set to infinity or large value when in bulb/time
            get {
                if (Connected) {
                    try {
                        var result = this.camera.GetDevicePropDesc(NEKCS.NikonMtpDevicePropCode.ExposureTime);
                        NikonDevicePropDescDS<UInt32> exp = new();
                        if (!result.TryGetUInt32(ref exp)) return double.NaN;
                        var exps = exp.EnumFORM.ToList();
                        exps.Remove(0xFFFFFFFF); //Bulb
                        exps.Remove(0xFFFFFFFD); //Time
                        return exps.Max() / 10000.0;
                    } catch {
                        throw;
                    }
                }
                return double.NaN;
            }
        }
        public short MaxBinX { get => 1; } //TODO
        public short MaxBinY { get => 1; } //TODO
        public double PixelSizeX { get => 0; } //TODO
        public double PixelSizeY { get => 0; } //TODO
        public bool CanSetTemperature { get => false; } //TO RECHECK: doesn't seem possible
        public bool CoolerOn { get => false; set {} }
        public double CoolerPower { get => double.NaN; }
        public bool HasDewHeater { get => false; }
        public bool DewHeaterOn { get => false; set { } }
        public CameraStates CameraState { //TO IMPROVE: event busydevice, ...
            get {
                if (this.camera != null && this.camera.isConnected()) {
                    return CameraStates.Idle;
                }
                return CameraStates.Error;
            }
        }
        public bool CanSubSample { get => false; } //TO RECHECK:
        public bool EnableSubSample { get; set; } //TO RECHECK:
        public int SubSampleX { get; set; } //TO RECHECK:
        public int SubSampleY { get; set; } //TO RECHECK:
        public int SubSampleWidth { get; set; } //TO RECHECK:
        public int SubSampleHeight { get; set; } //TO RECHECK:
        public bool CanShowLiveView { get => false; } //TODO: that temporary ;)
        public bool LiveViewEnabled { get => false; } //TODO: that temporary ;)
        public bool HasBattery { get => this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.BatteryLevel); }
        public int BatteryLevel {
            get {
                if (Connected) {
                    try {
                        var result = this.camera.GetDevicePropValue(NEKCS.NikonMtpDevicePropCode.BatteryLevel);
                        byte level = 0;
                        return result.TryGetUInt8(ref level) ? (int)level : 0;
                    } catch {
                        throw;
                    }
                }
                return -1;
            } 
        }
        public int BitDepth { get => 16; } //TODO: set at 16bits for dcraw

        public bool CanSetOffset { get => false; } //TO RECHECK: doesn't seem possible
        public int Offset { get; set; }
        public int OffsetMin { get; }
        public int OffsetMax { get; }
        public bool CanSetUSBLimit { get => false; } //TO RECHECK: that doesn't seem possible
        public int USBLimit { get; set; }
        public int USBLimitMin { get; }
        public int USBLimitMax { get; }
        public int USBLimitStep { get; }
        public bool CanGetGain { get => this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndex) || this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx); }
        public bool CanSetGain { get => false; } //TODO: false just for now => need to implement GetDevicePropDesc
        public int GainMax { get => this.Gains.Min(); }
        public int GainMin { get => this.Gains.Max(); }
        public int Gain {
            get { 
                if (Connected) {
                    try {
                        if (this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx)) {
                            var result = this.camera.GetDevicePropValue(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx);
                            UInt32 gain = 0;
                            return result.TryGetUInt32(ref gain) ? (int)gain : 0;
                        } else {
                            var result = this.camera.GetDevicePropValue(NEKCS.NikonMtpDevicePropCode.ExposureIndex);
                            UInt16 gain = 0;
                            return result.TryGetUInt16(ref gain) ? (int)gain : 0;
                        }
                    } 
                    catch {
                        throw;
                    }
                }
                return -1;
            } 
            set => throw new NotImplementedException(); 
        }
        public double ElectronsPerADU { get => double.NaN; } //TO CHECK: seem hard to do...
        public IList<string> ReadoutModes { get => new List<String> { "Default" }; } //TO CHECK
        public short ReadoutMode { get => 0; set {} } //TO CHECK
        public short ReadoutModeForSnapImages { get => 0; set {} } //TO CHECK
        public short ReadoutModeForNormalImages { get => 0; set {} } //TO CHECK

        public IList<int> Gains { 
            get {
                if (Connected) {
                    try {
                        if (this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx)) {
                            var result = this.camera.GetDevicePropDesc(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx);
                            NikonDevicePropDescDS<UInt32> gains = new();
                            return result.TryGetUInt32(ref gains) ? gains.EnumFORM.Select(x => (int)x).ToList() : new List<int>();
                        } else {
                            var result = this.camera.GetDevicePropDesc(NEKCS.NikonMtpDevicePropCode.ExposureIndex);
                            NikonDevicePropDescDS<UInt16> gains = new();
                            return result.TryGetUInt16(ref gains) ? gains.EnumFORM.Select(x => (int)x).ToList() : new List<int>();
                        }
                    } catch {
                        throw;
                    }
                }
                return new List<int>();
            }
        }

        public AsyncObservableCollection<BinningMode> BinningModes { get => new AsyncObservableCollection<BinningMode> { new BinningMode(1, 1) }; } //TO RECHECK

        public void SetBinning(short x, short y) {} //TO RECHECK

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
