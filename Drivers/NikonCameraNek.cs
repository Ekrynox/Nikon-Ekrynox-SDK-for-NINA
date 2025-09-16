using NEKCS;
using Newtonsoft.Json.Linq;
using NINA.Core.Enum;
using NINA.Core.Model.Equipment;
using NINA.Core.Utility;
using NINA.Core.Utility.Notification;
using NINA.Equipment.Interfaces;
using NINA.Equipment.Interfaces.Mediator;
using NINA.Equipment.Model;
using NINA.Equipment.Utility;
using NINA.Image.ImageData;
using NINA.Image.Interfaces;
using NINA.Profile;
using NINA.Profile.Interfaces;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;



namespace LucasAlias.NINA.NEK.Drivers {
    public partial class NikonCameraNek : BaseINPC, ICamera {
        public const string sourceFile = @"NEKDrivers\NikonCameraNEK.cs";

        public NikonCameraNek(string devicePath, NEKCS.NikonDeviceInfoDS cameraInfo, IProfileService profileService, IExposureDataFactory exposureDataFactory, ICameraMediator cameraMediator, IFocuserMediator focuserMediator) {
            this.devicePath = devicePath;
            this.cameraInfo = cameraInfo;

            if (NEKMediator.CameraList.ContainsKey(this.cameraInfo.Model)) this.cameraSpec = NEKMediator.CameraList[this.cameraInfo.Model];
            else this.cameraSpec = new Database.NikonCameraSpec();
            //Default Values
            if (this.cameraSpec.Name == "") this.cameraSpec.Name = this.cameraInfo.Model;
            if (this.cameraSpec.Sensor.BitDepth == -1) this.cameraSpec.Sensor.BitDepth = 16;

            this.profileService = profileService;
            this.exposureDataFactory = exposureDataFactory;
            this.cameraMediator = cameraMediator;
            this.focuserMediator = focuserMediator;
        }

        private readonly string devicePath; // WPD device path
        private NEKCS.NikonCamera camera; // Camera object for operations
        private NEKCS.NikonDeviceInfoDS cameraInfo; // GetDeviceInfo
        private Database.NikonCameraSpec cameraSpec;

        private readonly IProfileService profileService;
        private readonly IExposureDataFactory exposureDataFactory;
        private readonly ICameraMediator cameraMediator;
        private readonly IFocuserMediator focuserMediator;



        public bool HasSetupDialog { get => false; } // TODO
        public string Id { get => cameraInfo.SerialNumber; }
        public string Name { get => "Nikon " + this.cameraSpec.Name; }
        public string DisplayName { get => "Nikon " + this.cameraSpec.Name + " (NEK Experimental)"; }
        public string Category { get => "Nikon"; }
        public bool Connected {
            get {
                if (camera == null) return false;
                return camera.isConnected();
            }
        }
        public string Description { get => "Your Nikon Camera"; }
        public string DriverInfo { get => "Nikon Ekrynox SDK"; }
        public string DriverVersion { get => cameraInfo.DeviceVersion; }


        public Task<bool> Connect(CancellationToken token) {
            return Task.Run(() => {
                Logger.Info("Start connecting to the Camera: " + this.Name, "Connect", sourceFile);

                try {
                    this.camera = new NEKCS.NikonCamera(devicePath, NEKMediator.Plugin.NbAdditionalThreads);
                } catch (MtpDeviceException e) {
                    Logger.Error("Error while connecting to the Camera: " + this.Name, e, "Connect", sourceFile);
                    return false;
                }

                if (!this.camera.isConnected()) {
                    this.camera.Dispose();
                    this.camera = null;
                    return false;
                }

                try {
                    //Try to purge the Camera SDRAM to correctly receive handle at the first capture
                    this.camera.SendCommand(NikonMtpOperationCode.DeleteImagesInSdram, new NEKCS.MtpParams());
                } catch (MtpDeviceException e) {
                    Logger.Error("Error while purging SDRAM: " + this.Name, e, "Connect", sourceFile);
                    this.camera.Dispose();
                    this.camera = null;
                    return false;
                } catch (MtpException e) {
                    Logger.Error("Error while purging SDRAM: " + this.Name, e, "Connect", sourceFile);
                    this.camera.Dispose();
                    this.camera = null;
                    return false;
                }

                try {
                    //Get device info: Operation, Properties, Events, ... supported + Name, model number, serial, ...
                    this.cameraInfo = this.camera.GetDeviceInfo();
                } catch (MtpDeviceException e) {
                    Logger.Error("Error while requesting Device Info: " + this.Name, e, sourceFile);
                    this.camera.Dispose();
                    this.camera = null;
                    return false;
                } catch (MtpException e) {
                    Logger.Error("Error while requesting Device Info: " + this.Name, e, sourceFile);
                    this.camera.Dispose();
                    this.camera = null;
                    return false;
                }

                this.camera.OnMtpEvent += new MtpEventHandler(camPropEvent);
                this.camera.OnMtpEvent += new MtpEventHandler(camStateEvent);

                lock (_gateCameraState) {
                    this._cameraState = CameraStates.Idle;
                }

                this._bulbTime = 0;
                this._isBulb = false;

                this._imageInfo = null;
                this._imageStream = null;

                this._liveviewHeaderSize = -1;
                this._requestedLiveview = 0;
                this._liveviewEnabled = false;
                this.sdramHandle = 0xFFFF0001;

                updateLensInfo(true);

                return this.camera.isConnected();
            }, token);
        }

        public void Disconnect() {
            Logger.Info("Start diconnecting from the Camera: " + this.Name, "Disconnect", sourceFile);
            lock (_gateCameraState) {
                this._cameraState = CameraStates.NoState;
            }

            if (this.focuserMediator.GetDevice() != null && focuserMediator.GetDevice().Connected && focuserMediator.GetDevice() is NikonFocuserNek) {
                this.focuserMediator.Disconnect();
            }

            if (this.camera != null) {
                this.camera.OnMtpEvent -= new MtpEventHandler(camPropEvent);
                this.camera.OnMtpEvent -= new MtpEventHandler(camStateEvent);

                this._requestedLiveview = 0;
                this._liveviewEnabled = false;
                this.StopLiveView();
                this.AbortExposure();

                this.camera.Dispose();
                this.camera = null;
            }

            foreach (var i in this._awaitersCameraState) {
                if (i.Value != null) i.Value.TrySetCanceled();
            }
            RaiseAllPropertiesChanged();
        }

        public void SetupDialog() { throw new NotImplementedException(); }

        public IList<string> SupportedActions { get => new List<string>(); } //TOCHECK

        public string Action(string actionName, string actionParameters) { throw new NotImplementedException(); }

        public string SendCommandString(string command, bool raw = true) { throw new NotImplementedException(); }

        public bool SendCommandBool(string command, bool raw = true) { throw new NotImplementedException(); }

        public void SendCommandBlind(string command, bool raw = true) { throw new NotImplementedException(); }




        public bool HasShutter { get => true; } //TO RECHECK: not true for all camera => Z6/7 in Silence mode, Z8, ...
        public string SensorName { get => this.cameraSpec.Sensor.Name; } //TO RECHECK: doesn't seem easly feasible
        public SensorType SensorType { get => SensorType.RGGB; } //TO RECHECK: certainly that
        public short BayerOffsetX { get => 0; } //TO RECHECK
        public short BayerOffsetY { get => 0; } //TO RECHECK
        public int CameraXSize { get => this.cameraSpec.Sensor.ResX; } //TODO
        public int CameraYSize { get => this.cameraSpec.Sensor.ResY; } //TODO
        public double PixelSizeX { get => this.cameraSpec.Sensor.PixelSizeX; }
        public double PixelSizeY { get => this.cameraSpec.Sensor.PixelSizeY; }
        public int BitDepth { get => (int)this.profileService.ActiveProfile.CameraSettings.BitDepth; } //TODO:

        public short BinX { get => 1; set { } } //TO RECHECK
        public short BinY { get => 1; set { } } //TO RECHECK
        public AsyncObservableCollection<BinningMode> BinningModes { get => new AsyncObservableCollection<BinningMode> { new BinningMode(1, 1) }; } //TO RECHECK
        public void SetBinning(short x, short y) { } //TO RECHECK
        public short MaxBinX { get => 1; } //TODO
        public short MaxBinY { get => 1; } //TODO

        public bool CanSubSample { get => false; } //TO RECHECK:
        public bool EnableSubSample { get; set; }
        public int SubSampleX { get; set; }
        public int SubSampleY { get; set; }
        public int SubSampleWidth { get; set; }
        public int SubSampleHeight { get; set; }
        public void UpdateSubSampleArea() { throw new NotImplementedException(); }

        public bool CanSetOffset { get => false; } //TO RECHECK: doesn't seem possible
        public int Offset { get; set; }
        public int OffsetMin { get; }
        public int OffsetMax { get; }

        public bool HasBattery { get => Connected && this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.BatteryLevel); }
        public int BatteryLevel {
            get {
                if (Connected) {
                    try {
                        var result = this.camera.GetDevicePropValue(NEKCS.NikonMtpDevicePropCode.BatteryLevel);
                        if (!result.TryGetUInteger(out var level)) {
                            Logger.Error("Wrong Datatype UInteger! Expected: " + result.GetType().ToString() + " on " + this.Name, "BatteryLevel", sourceFile);
                            return -1;
                        }
                        return (int)level;
                    } catch (MtpDeviceException e) {
                        Logger.Error(this.Name, e, "BatteryLevel", sourceFile);
                        return -1;
                    } catch (MtpException e) {
                        Logger.Error(this.Name, e, "BatteryLevel", sourceFile);
                        return -1;
                    }
                }
                return -1;
            }
        }

        public bool CanSetTemperature { get => false; } //TO RECHECK: doesn't seem possible
        public double Temperature { get => double.NaN; }
        public double TemperatureSetPoint { get; set; }
        public bool CoolerOn { get => false; set {} }
        public double CoolerPower { get => double.NaN; }
        public bool HasDewHeater { get => false; }
        public bool DewHeaterOn { get => false; set { } }

        public bool CanSetUSBLimit { get => false; } //TO RECHECK: that doesn't seem possible
        public int USBLimit { get; set; }
        public int USBLimitMin { get; }
        public int USBLimitMax { get; }
        public int USBLimitStep { get; }

        public double ElectronsPerADU { get => double.NaN; } //TO CHECK: seem hard to do...
        public IList<string> ReadoutModes { get => new List<String> { "Default" }; } //TO CHECK
        public short ReadoutMode { get => 0; set { } } //TO CHECK
        public short ReadoutModeForSnapImages { get => 0; set { } } //TO CHECK
        public short ReadoutModeForNormalImages { get => 0; set { } } //TO CHECK

        public bool CanGetGain { get => Connected && this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndex) || this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx); }
        public bool CanSetGain {
            get {
                if (Connected) {
                    try {
                        if (this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx)) {
                            var result = this.camera.GetDevicePropDesc(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx);
                            return result.GetSet > 0;
                        } else if (this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndex)) {
                            var result = this.camera.GetDevicePropDesc(NEKCS.NikonMtpDevicePropCode.ExposureIndex);
                            return result.GetSet > 0;
                        }
                        return false;
                    } catch (MtpDeviceException e) {
                        Logger.Error(this.Name, e, "CanSetGain", sourceFile);
                        return false;
                    } catch (MtpException e) {
                        Logger.Error(this.Name, e, "CanSetGain", sourceFile);
                        return false;
                    }
                }
                return false;
            }
        }
        public int GainMax {
            get {
                var gains = Gains;
                return gains.Count > 0 ? gains.Max() : 0;
            }
        }
        public int GainMin {
            get {
                var gains = Gains;
                return gains.Count > 0 ? gains.Min() : 0;
            }
        }
        public IList<int> Gains {
            get {
                if (Connected) {
                    try {
                        if (this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx)) {
                            var result = this.camera.GetDevicePropDesc(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx);
                            if (!result.TryGetUInteger(out var gains)) {
                                Logger.Error("Wrong Datatype UInteger! Expected: " + result.DataType.ToString() + " for ExposureIndexEx on " + this.Name, "Gains", sourceFile);
                                return new List<int>();
                            }
                            return gains.EnumFORM.Select(x => (int)x).ToList();
                        } else {
                            var result = this.camera.GetDevicePropDesc(NEKCS.NikonMtpDevicePropCode.ExposureIndex);
                            if (!result.TryGetUInteger(out var gains)) {
                                Logger.Error("Wrong Datatype UInteger! Expected: " + result.DataType.ToString() + " for ExposureIndex on " + this.Name, "Gains", sourceFile);
                                return new List<int>();
                            }
                            return gains.EnumFORM.Select(x => (int)x).ToList();
                        }
                    } catch (MtpDeviceException e) {
                        Logger.Error(this.Name, e, "Gains", sourceFile);
                        return new List<int>();
                    } catch (MtpException e) {
                        Logger.Error(this.Name, e, "Gains", sourceFile);
                        return new List<int>();
                    }
                }
                return new List<int>();
            }
        }
        public int Gain {
            get { 
                if (Connected) {
                    try {
                        if (this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx)) {
                            var result = this.camera.GetDevicePropValue(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx);
                            if (!result.TryGetUInteger(out var gain)) {
                                Logger.Error("Wrong Datatype UInteger! Expected: " + result.GetType().ToString() + " for ExposureIndexEx on " + this.Name, "Gain -> Getter", sourceFile);
                                return -1;
                            }
                            return (int)gain;
                        } else {
                            var result = this.camera.GetDevicePropValue(NEKCS.NikonMtpDevicePropCode.ExposureIndex);
                            if (!result.TryGetUInteger(out var gain)) {
                                Logger.Error("Wrong Datatype UInteger! Expected: " + result.GetType().ToString() + " for ExposureIndex on " + this.Name, "Gain -> Getter", sourceFile);
                                return -1;
                            }
                            return (int)gain;
                        }
                    } catch (MtpDeviceException e) {
                        Logger.Error(this.Name, e, "Gain -> Getter", sourceFile);
                        return -1;
                    } catch (MtpException e) {
                        Logger.Error(this.Name, e, "Gain -> Getter", sourceFile);
                        return -1;
                    }
                }
                return -1;
            } 
            set {
                if (Connected) {
                    try {
                        if (this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx)) {
                            this.camera.SetDevicePropValueTypesafe(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx, new MtpDatatypeVariant((UInt32)value));
                        } else {
                            this.camera.SetDevicePropValueTypesafe(NEKCS.NikonMtpDevicePropCode.ExposureIndex, new MtpDatatypeVariant((UInt16)value));
                        }
                        RaisePropertyChanged(nameof(Gain));
                    } catch (MtpDeviceException e) {
                        Logger.Error(this.Name, e, "Gain -> Setter: " + value, sourceFile);
                    } catch (MtpException e) {
                        Logger.Error(this.Name, e, "Gain -> Setter: " + value, sourceFile);
                    }
                }
            }
        }


        private bool _isBulb;
        private double _bulbTime;

        public bool CanSetBulb {
            get {
                if (Connected) {
                    if (!cameraInfo.OperationsSupported.Contains(NikonMtpOperationCode.InitiateCaptureRecInMedia)) return false;
                    try {
                        var result = this.camera.GetDevicePropDesc(NEKCS.NikonMtpDevicePropCode.ExposureTime);
                        if (!result.TryGetUInteger(out var exp)) {
                            Logger.Error("Wrong Datatype UInteger! Expected: " + result.DataType.ToString() + " for ExposureTime on " + this.Name, "CanSetBulb", sourceFile);
                            return false;
                        }
                        return exp.EnumFORM.ToList().Contains(0xFFFFFFFF);
                    } catch (MtpDeviceException e) {
                        Logger.Error(this.Name, e, "CanSetBulb", sourceFile);
                    } catch (MtpException e) {
                        Logger.Error(this.Name, e, "CanSetBulb", sourceFile);
                    }
                }
                return false;
            }
        }
        public IList<double> Exposures {
            get {
                if (Connected) {
                    try {
                        var result = this.camera.GetDevicePropDesc(NEKCS.NikonMtpDevicePropCode.ExposureTime);
                        if (!result.TryGetUInteger(out var exp)) {
                            Logger.Error("Wrong Datatype UInteger! Expected: " + result.DataType.ToString() + " for ExposureTime on " + this.Name, "Exposures", sourceFile);
                            return new List<double>();
                        }

                        _isBulb = (exp.CurrentValue == 0xFFFFFFFF); //Bulb

                        var exps = exp.EnumFORM.ToList();
                        exps.Remove(0xFFFFFFFF); //Bulb
                        exps.Remove(0xFFFFFFFD); //Time
                        return exps.Select(x => x / 10000.0).ToList();
                    } catch (MtpDeviceException e) {
                        Logger.Error(this.Name, e, "Exposures", sourceFile);
                    } catch (MtpException e) {
                        Logger.Error(this.Name, e, "Exposures", sourceFile);
                    }
                }
                return new List<double>();
            }
        }
        public double ExposureMin {
            get {
                var exps = Exposures;
                return exps.Count > 0 ? exps.Min() : double.NaN;
            }
        }
        public double ExposureMax {
            get {
                if (CanSetBulb) {
                    return double.PositiveInfinity;
                }
                var exps = Exposures;
                return exps.Count > 0 ? exps.Max() : double.NaN;
            }
        }
        public double ExposureTime {
            get {
                if (Connected) {
                    if (this._isBulb) {
                        return this._bulbTime;
                    } else {
                        try {
                            var result = this.camera.GetDevicePropValue(NikonMtpDevicePropCode.ExposureTime);
                            if (!result.TryGetUInteger(out var exp)) {
                                Logger.Error("Wrong Datatype UInteger! Expected: " + result.GetType().ToString() + " for ExposureTime on " + this.Name, "ExposureTime -> Getter", sourceFile);
                                return 0;
                            }
                            return exp / 10000.0;
                        } catch (MtpDeviceException e) {
                            Logger.Error(this.Name, e, "ExposureTime -> Getter", sourceFile);
                        } catch (MtpException e) {
                            Logger.Error(this.Name, e, "ExposureTime -> Getter", sourceFile);
                        }
                    }
                }
                return 0;
            }
            set {
                if (Connected) {
                    double newExp = Exposures.OrderBy(x => Math.Abs(value - x)).First();

                    if ((value > 1.0) && CanSetBulb) {
                        try {
                            this.camera.SetDevicePropValueTypesafe(NikonMtpDevicePropCode.ExposureTime, new MtpDatatypeVariant((UInt32)0xFFFFFFFF));
                            this._isBulb = true;
                            this._bulbTime = value;
                            RaisePropertyChanged(nameof(ExposureTime));
                        } catch (MtpDeviceException e) {
                            Logger.Error(this.Name, e, "ExposureTime -> Setter: " + value, sourceFile);
                            throw;
                        } catch (MtpException e) {
                            Logger.Error(this.Name, e, "ExposureTime -> Setter: " + value, sourceFile);
                            throw;
                        }
                    } else {
                        try {
                            this.camera.SetDevicePropValueTypesafe(NikonMtpDevicePropCode.ExposureTime, new MtpDatatypeVariant((UInt32)(newExp * 10000)));
                            this._isBulb = false;
                            RaisePropertyChanged(nameof(ExposureTime));
                        } catch (MtpDeviceException e) {
                            Logger.Error(this.Name, e, "ExposureTime -> Setter: " + value, sourceFile);
                            throw;
                        } catch (MtpException e) {
                            Logger.Error(this.Name, e, "ExposureTime -> Setter: " + value, sourceFile);
                            throw;
                        }
                    }
                }
            }
        }


        private void camPropEvent(NEKCS.NikonCamera cam, NEKCS.MtpEvent e) {
            if (e.eventCode == NikonMtpEventCode.DeviceInfoChanged) {
                if (Connected) {
                    this.cameraInfo = this.camera.GetDeviceInfo();
                    RaiseAllPropertiesChanged();
                } else {
                    Notification.ShowError("Nikon NEK: The camera have been brutally disconnected!");
                    this.cameraMediator.Disconnect();
                }
            } else if (e.eventCode == NikonMtpEventCode.DevicePropChanged) {
                switch ((NikonMtpDevicePropCode)e.eventParams[0]) {
                    case NikonMtpDevicePropCode.BatteryLevel:
                        RaisePropertyChanged(nameof(BatteryLevel));
                        break;
                    case NikonMtpDevicePropCode.ExposureIndexEx:
                    case NikonMtpDevicePropCode.ExposureIndex:
                        RaisePropertyChanged(nameof(Gain));
                        break;
                    case NikonMtpDevicePropCode.ExposureTime:
                        RaisePropertyChanged(nameof(ExposureMin));
                        RaisePropertyChanged(nameof(ExposureMax));
                        break;
                    case NikonMtpDevicePropCode.LensID:
                    case NikonMtpDevicePropCode.LensSort:
                    case NikonMtpDevicePropCode.Fnumber:
                    case NikonMtpDevicePropCode.FocalLength:
                        updateLensInfo();
                        break;
                }
            }
        }

        public bool isCpuLensMounted() {
            if (Connected) {
                try {
                    var result = camera.GetDevicePropValue(NikonMtpDevicePropCode.LensSort);
                    if (!result.TryGetUInteger(out var lensSort)) {
                        Logger.Error("Wrong Datatype UInteger! Expected: " + result.GetType().ToString() + " for LensSort on " + this.Name, "isCpuLensMounted", sourceFile);
                        return false;
                    }
                    return lensSort == 1;
                } catch (MtpDeviceException e) {
                    Logger.Error(this.Name, e, "isCpuLensMounted", sourceFile);
                } catch (MtpException e) {
                    Logger.Error(this.Name, e, "isCpuLensMounted", sourceFile);
                }
            }
            return false;
        }
        public bool isFocusDrivableLens() {
            if (Connected) {
                if (!cameraInfo.OperationsSupported.Contains(NikonMtpOperationCode.MfDrive)) return false;
                if (!isCpuLensMounted()) return false;

                try {
                    //MirrorLess doesn't support non AF-S lens with the Z adapter => no Screew in
                    if (!cameraInfo.DevicePropertiesSupported.Contains(NikonMtpDevicePropCode.LensTypeML)) return true; //Not Mirroless Z

                    var result = camera.GetDevicePropValue(NikonMtpDevicePropCode.LensTypeML);
                    if (!result.TryGetUInteger(out var lenstype)) {
                        Logger.Error("Wrong Datatype Uinteger! Expected: " + result.GetType().ToString() + " for LensTypeML on " + this.Name, "isCpuLensMounted", sourceFile);
                        return false;
                    }
                    if ((lenstype & 0b1) == 0) return true; //Native Z lens

                    result = camera.GetDevicePropValue(NikonMtpDevicePropCode.LensTypeF);
                    if (!result.TryGetUInteger(out lenstype)) { //On mirrorless this property is 64bits
                        Logger.Error("Wrong Datatype UInteger! Expected: " + result.GetType().ToString() + " for LensTypeF on " + this.Name, "isCpuLensMounted", sourceFile);
                        return false;
                    }
                    if ((lenstype & 0b10000) != 0) return true; //AF-S Lens with mount adaptor

                    return true;
                } catch (MtpDeviceException e) {
                    Logger.Error(this.Name, e, "isFocusDrivableLens", sourceFile);
                } catch (MtpException e) {
                    Logger.Error(this.Name, e, "isFocusDrivableLens", sourceFile);
                }
            }
            return false;
        }
        private void updateLensInfo(bool init = false) {
            if (Connected && NEKMediator.Plugin.UpdateLensInfo) {
                if (isCpuLensMounted()) {
                    try {
                        if (this.cameraInfo.DevicePropertiesSupported.Contains(NikonMtpDevicePropCode.FocalLength)) {
                            var result = camera.GetDevicePropValue(NikonMtpDevicePropCode.FocalLength);
                            if (result.TryGetUInteger(out var flength)) {
                                this.profileService.ActiveProfile.TelescopeSettings.FocalLength = flength / 100.0;
                            } else {
                                Logger.Error("Wrong Datatype UInteger! Expected: " + result.GetType().ToString() + " for FocalLength on " + this.Name, "updateLensInfo", sourceFile);
                            }
                        }
                    } catch (MtpDeviceException e) {
                        Logger.Error(this.Name, e, "updateLensInfo", sourceFile);
                    } catch (MtpException e) {
                        Logger.Error(this.Name, e, "updateLensInfo", sourceFile);
                    }

                    try {
                        var result = camera.GetDevicePropValue(NikonMtpDevicePropCode.Fnumber);
                        if (result.TryGetUInteger(out var fnumber)) {
                            this.profileService.ActiveProfile.TelescopeSettings.FocalRatio = fnumber / 100.0;
                        } else {
                            Logger.Error("Wrong Datatype UInteger! Expected: " + result.GetType().ToString() + " for Fnumber on " + this.Name, "updateLensInfo", sourceFile);
                        }
                    } catch (MtpDeviceException e) {
                        Logger.Error(this.Name, e, "updateLensInfo", sourceFile);
                    } catch (MtpException e) {
                        Logger.Error(this.Name, e, "updateLensInfo", sourceFile);
                    }
                } else if (!init) { //Lens disconnected
                    this.profileService.ActiveProfile.TelescopeSettings.Name = "";
                    this.profileService.ActiveProfile.TelescopeSettings.FocalLength = double.NaN;
                    this.profileService.ActiveProfile.TelescopeSettings.FocalRatio = double.NaN;
                }
            }
        }



        private CameraStates _cameraState;
        private readonly object _gateCameraState = new();
        private readonly Dictionary<CameraStates, TaskCompletionSource<bool>> _awaitersCameraState = new();
        private CancellationTokenSource bulbToken;
        private double _oldExposureTime;

        private MemoryStream _imageStream;
        private NikonObjectInfoDS _imageInfo;
        private UInt32 sdramHandle;
        public CameraStates CameraState {
            get {
                if (Connected) {
                    lock (_gateCameraState) {
                        return _cameraState;
                    }
                }
                return CameraStates.Error;
            }
        }

        private void camStateEvent(NEKCS.NikonCamera cam, NEKCS.MtpEvent e) {
            if (e.eventCode == NikonMtpEventCode.ObjectAddedInSdram) {
                lock (_gateCameraState) {
                    _cameraState = CameraStates.Download;
                }
                if (_awaitersCameraState.TryGetValue(CameraStates.Exposing, out var exp)) {
                    exp.TrySetResult(true);
                }

                RaisePropertyChanged(nameof(CameraState));

                try {
                    if (e.eventParams.Length > 0) {
                        sdramHandle = e.eventParams[0] == 0 ? 0xFFFF0001 : e.eventParams[0];
                    }

                    //Stop the Liveview started to prevent AF (needed for the D7100, ...)
                    StopLiveViewBackground();

                    //Retreive the Image Metadata (needed for the D80, ...)
                    _imageInfo = camera.GetObjectInfo(sdramHandle);

                    NEKCS.MtpParams param = new();
                    param.addUint32(sdramHandle);
                    NEKCS.MtpResponse result = this.camera.SendCommandAndRead(NikonMtpOperationCode.GetObject, param);
                    _imageStream = new(result.data);

                    try {
                        this.ExposureTime = _oldExposureTime;
                    } catch { }

                    if (_awaitersCameraState.TryGetValue(CameraStates.Download, out var dl)) {
                        dl.SetResult(true);
                    }
                } catch (Exception ex) {
                    Logger.Error(this.Name, ex, "camStateEvent", sourceFile);
                    if (_awaitersCameraState.TryGetValue(CameraStates.Download, out var dl)) dl.TrySetCanceled();
                } finally {
                    lock (_gateCameraState) _cameraState = CameraStates.Idle;
                    RaisePropertyChanged(nameof(CameraState));
                }
            }
        }

        public void StartExposure(CaptureSequence sequence) {
            if (!Connected || this._cameraState == CameraStates.Error || this._cameraState == CameraStates.NoState) return;

            if (this._cameraState == CameraStates.Waiting && this._awaitersCameraState.TryGetValue(CameraStates.Waiting, out var wtcs)) wtcs.Task.Wait();

            if (this._cameraState != CameraStates.Idle) AbortExposure();

            if (this._cameraState == CameraStates.Exposing && this._awaitersCameraState.TryGetValue(CameraStates.Exposing, out var etcs)) etcs.Task.Wait();
            if (this._cameraState == CameraStates.Download && this._awaitersCameraState.TryGetValue(CameraStates.Download, out var dtcs)) dtcs.Task.Wait();

            lock (_gateCameraState) {
                this._awaitersCameraState[CameraStates.Exposing] = new();
                this._awaitersCameraState[CameraStates.Download] = new();
                this._cameraState = CameraStates.Exposing;
            }

            this._oldExposureTime = this.ExposureTime;
            try {
                this.ExposureTime = sequence.ExposureTime;
                if (!this.CanSetBulb && (this.ExposureTime != this._oldExposureTime) && (this.ExposureTime > 1)) {
                    Notification.ShowWarning("Nikon NEK: Bulb is not available!\nAre you in M mode?", TimeSpan.FromSeconds(10));
                }
            } catch (MtpException) {
                if (this._isBulb) {
                    Notification.ShowError("Nikon NEK: Bulb could not be set!\nAre you in M mode?");
                } else {
                    Notification.ShowError("Nikon NEK: Shutter speed could not be set!\nAre you in M or S mode?");
                }
            }

            //Start the Liveview started to prevent AF (needed for the D7100, ...)
            StartLiveViewBackground();

            try {
                NEKCS.MtpParams param = new();
                MtpResponse result;
                if (_isBulb) {
                    param.addUint32(0xFFFFFFFF);
                    param.addUint32(0x0001);
                    result = this.camera.SendCommand(NikonMtpOperationCode.InitiateCaptureRecInMedia, param);
                    RaisePropertyChanged(nameof(CameraState));
                } else {
                    param.addUint32(0xFFFFFFFF);
                    result = this.camera.SendCommand(NikonMtpOperationCode.InitiateCaptureRecInSdram, param);
                    RaisePropertyChanged(nameof(CameraState));
                }
                if (result.responseCode != NikonMtpResponseCode.OK) {
                    throw new NEKCS.MtpException(NikonMtpOperationCode.InitiateCaptureRecInSdram, result.responseCode);
                }
            } catch (Exception e) {
                Logger.Error(this.Name, e, "StartExposure", sourceFile);
                _awaitersCameraState[CameraStates.Exposing].TrySetCanceled();
                _awaitersCameraState[CameraStates.Download].TrySetCanceled();
                lock (_gateCameraState) {
                    _cameraState = CameraStates.Idle;
                }
                throw;
            }

            if (_isBulb) {
                bulbToken = new CancellationTokenSource();
                Task.Run(async () => {
                    await Task.Delay((int)(sequence.ExposureTime * 1000), bulbToken.Token);
                    if (bulbToken.IsCancellationRequested) return;
                    lock (_gateCameraState) {
                        if (_cameraState == CameraStates.Exposing) {
                            try {
                                var p = new MtpParams();
                                p.addUint32(0);
                                p.addUint32(0);
                                camera.SendCommand(NikonMtpOperationCode.TerminateCapture, p);
                            } catch (MtpDeviceException e) {
                                Logger.Error(this.Name, e, "StartExposure", sourceFile);
                            } catch (MtpException e) {
                                Logger.Error(this.Name, e, "StartExposure", sourceFile);
                            }
                        }
                    }
                }, bulbToken.Token);
            }
        }
        public async Task WaitUntilExposureIsReady(CancellationToken token) {
            lock (_gateCameraState) {
                if (_cameraState != CameraStates.Exposing && _cameraState != CameraStates.Idle) return;
            }

            using (token.Register(() => AbortExposure())) {
                if (_awaitersCameraState.TryGetValue(CameraStates.Exposing, out var etcs)) {
                    await etcs.Task;
                }
            }
        }

        public void StopExposure() {
            if (_isBulb) {
                lock (_gateCameraState) {
                    if (_cameraState == CameraStates.Exposing) {
                        bulbToken.Cancel();
                        try {
                            var p = new MtpParams();
                            p.addUint32(0);
                            p.addUint32(0);
                            camera.SendCommand(NikonMtpOperationCode.TerminateCapture, p);
                        } catch (MtpDeviceException e) {
                            Logger.Error(this.Name, e, "StopExposure", sourceFile);
                        } catch (MtpException e) {
                            Logger.Error(this.Name, e, "StopExposure", sourceFile);
                        }
                    }
                }
            } 
        }

        public void AbortExposure() {
            if (_isBulb) {
                lock (_gateCameraState) {
                    if (_cameraState == CameraStates.Exposing) {
                        bulbToken.Cancel();
                        try {
                            var p = new MtpParams();
                            p.addUint32(0);
                            p.addUint32(0);
                            camera.SendCommand(NikonMtpOperationCode.TerminateCapture, p);
                        } catch (MtpDeviceException e) {
                            Logger.Error(this.Name, e, "AbortExposure", sourceFile);
                        } catch (MtpException e) {
                            Logger.Error(this.Name, e, "AbortExposure", sourceFile);
                        }
                    }
                }
            }
        }

        public async Task<IExposureData> DownloadExposure(CancellationToken token) {
            if (_awaitersCameraState.TryGetValue(CameraStates.Download, out var tcs)) {
                await tcs.Task;
                if (tcs.Task.IsCanceled) return null;
            }

            if (_imageStream == null) {
                return null;
            }

            try {
                var rawImageData = _imageStream.ToArray();
                var metaData = new ImageMetaData();
                metaData.FromCamera(this);
                if (_imageStream != null) {
                    _imageStream.Dispose();
                    _imageStream = null;
                }
                return exposureDataFactory.CreateRAWExposureData(converter: profileService.ActiveProfile.CameraSettings.RawConverter, rawBytes: rawImageData, rawType: "nef", bitDepth: this.BitDepth, metaData: metaData);
            } catch { throw; }
        }


        private int _liveviewHeaderSize;
        private uint _requestedLiveview;
        private bool _liveviewEnabled = false;
        public bool CanShowLiveView { get => cameraInfo.OperationsSupported.Contains(NikonMtpOperationCode.GetLiveViewImage); }
        public bool LiveViewEnabled {
            get {
                if (!this._liveviewEnabled) return false;
                try {
                    camera.GetDevicePropValue(NikonMtpDevicePropCode.RemoteLiveViewStatus).TryGetUInteger(out var lvState);
                    return (lvState == 1);
                } catch {
                    return false;
                }
            }
        }

        public void StartLiveView(CaptureSequence sequence) {
            try {
                this._liveviewEnabled = true;
                camera.StartLiveView();
            } catch (Exception e) {
                Logger.Error(this.Name, e, "StartLiveView", sourceFile);
                this._liveviewEnabled = false;
            }
            RaisePropertyChanged(nameof(LiveViewEnabled));
        }
        public void StartLiveViewBackground() {
            try {
                Interlocked.Increment(ref this._requestedLiveview);
                camera.StartLiveView();
            } catch (Exception e) {
                Logger.Error(this.Name, e, "StartLiveView", sourceFile);
                Interlocked.Decrement(ref this._requestedLiveview);
            }
        }

        public Task<IExposureData> DownloadLiveView(CancellationToken token) {
            return Task.Run<IExposureData>(() => {
                NEKCS.MtpParams parameters = new();
                NEKCS.MtpResponse response;
                try {
                    if (cameraInfo.OperationsSupported.Contains(NikonMtpOperationCode.GetLiveViewImageEx)) {
                        response = camera.SendCommandAndRead(NikonMtpOperationCode.GetLiveViewImageEx, parameters);
                    } else {
                        response = camera.SendCommandAndRead(NikonMtpOperationCode.GetLiveViewImage, parameters);
                    }
                    if (response.responseCode != NikonMtpResponseCode.OK) {
                        return null;
                    }
                } catch (Exception e) {
                    Logger.Error(this.Name, e, "DowloadLiveview", sourceFile);
                    return null;
                }

                if (_liveviewHeaderSize < 0) {
                    _liveviewHeaderSize = 0;
                    for (int i = 1; i < response.data.Length; i++) {
                        if (response.data[i] == 0xD8 && response.data[i - 1] == 0xFF) {
                            _liveviewHeaderSize = i - 1;
                        }
                    }
                }

                var imageStream = new MemoryStream(response.data.Skip(_liveviewHeaderSize).ToArray());
                JpegBitmapDecoder decoder = new JpegBitmapDecoder(imageStream, BitmapCreateOptions.PreservePixelFormat, BitmapCacheOption.OnLoad);

                FormatConvertedBitmap bitmap = new FormatConvertedBitmap();
                bitmap.BeginInit();
                bitmap.Source = decoder.Frames[0];
                bitmap.DestinationFormat = System.Windows.Media.PixelFormats.Gray16;
                bitmap.EndInit();

                return exposureDataFactory.CreateImageArrayExposureDataFromBitmapSource(decoder.Frames[0]).Result;
            }, token);
        }

        public void StopLiveView() {
            try {
                this._liveviewEnabled = false;
                RaisePropertyChanged(nameof(LiveViewEnabled));
                if (!this._liveviewEnabled && Interlocked.Equals(this._requestedLiveview, (uint)0)) {
                    camera.EndLiveView();
                }
            } catch (Exception e) {
                Logger.Error(this.Name, e, "StopLiveView", sourceFile);
            }
        }
        public void StopLiveViewBackground() {
            try {
                Interlocked.Decrement(ref this._requestedLiveview);
                if (!this._liveviewEnabled && Interlocked.Equals(this._requestedLiveview, (uint)0)) camera.EndLiveView();
            } catch (Exception e) {
                Logger.Error(this.Name, e, "StopLiveView", sourceFile);
            }
        }
        public void StopLiveViewBackground(int waitms) {
            try {
                Interlocked.Decrement(ref this._requestedLiveview);
                Task.Run(async () => {
                    await Task.Delay(waitms);
                    if (!this._liveviewEnabled && Interlocked.Equals(this._requestedLiveview, (uint)0)) camera.EndLiveView();
                });
            } catch (Exception e) {
                Logger.Error(this.Name, e, "StopLiveView", sourceFile);
            }
        }

    }
}
