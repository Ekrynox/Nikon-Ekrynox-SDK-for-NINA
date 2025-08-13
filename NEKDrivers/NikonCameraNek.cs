using NEKCS;
using NINA.Core.Enum;
using NINA.Core.Model.Equipment;
using NINA.Core.Utility;
using NINA.Equipment.Interfaces;
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



namespace LucasAlias.NINA.NEK.NEKDrivers {
    public class NikonCameraNek : BaseINPC, ICamera {

        public NikonCameraNek(string devicePath, NEKCS.NikonDeviceInfoDS cameraInfo, IProfileService profileService, IExposureDataFactory exposureDataFactory) {
            this.devicePath = devicePath;
            this.cameraInfo = cameraInfo;

            this.profileService = profileService;
            this.exposureDataFactory = exposureDataFactory;
        }

        private string devicePath; // WPD device path
        private NEKCS.NikonDeviceInfoDS cameraInfo; // GetDeviceInfo
        private NEKCS.NikonCamera camera; // Camera object for operations

        private readonly IProfileService profileService;
        private readonly IExposureDataFactory exposureDataFactory;



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

                    this.camera.SendCommand(NikonMtpOperationCode.DeleteImagesInSdram, new NEKCS.MtpParams()); //Try to purge the Camera SDRAM to correctly receive handle 
                    this.cameraInfo = this.camera.GetDeviceInfo();

                    this.camera.OnMtpEvent += new MtpEventHandler(camPropEvent);
                    this.camera.OnMtpEvent += new MtpEventHandler(camStateEvent);

                    lock (_gateCameraState) {
                        _cameraState = CameraStates.Idle;
                    }

                    updateLensInfo();

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
                lock(_gateCameraState) {
                    _cameraState = CameraStates.NoState;
                }

                this.camera.OnMtpEvent -= new MtpEventHandler(camPropEvent);
                this.camera.OnMtpEvent -= new MtpEventHandler(camStateEvent);
                this.camera.Dispose();
                this.camera = null;

                foreach (var i in _awaitersCameraState) {
                    i.Value.SetCanceled();
                }
                RaisePropertyChanged("Connected");
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
        public string SensorName { get => ""; } //TO RECHECK: doesn't seem easly feasible
        public SensorType SensorType { get => SensorType.RGGB; } //TO RECHECK: certainly that
        public short BayerOffsetX { get => 0; } //TO RECHECK
        public short BayerOffsetY { get => 0; } //TO RECHECK
        public int CameraXSize { get => 0; } //TODO
        public int CameraYSize { get => 0; } //TODO
        public double PixelSizeX { get => 0; } //TODO
        public double PixelSizeY { get => 0; } //TODO
        public int BitDepth { get => 16; } //TODO: set at 16bits for dcraw

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

        public bool CanGetGain { get => this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndex) || this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx); }
        public bool CanSetGain {
            get {
                if (Connected) {
                    try {
                        if (this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx)) {
                            var result = this.camera.GetDevicePropDesc(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx);
                            NikonDevicePropDescDS<UInt32> gain = new();
                            return result.TryGetUInt32(ref gain) ? gain.GetSet > 0 : false;
                        } else if (this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndex)) {
                            var result = this.camera.GetDevicePropDesc(NEKCS.NikonMtpDevicePropCode.ExposureIndex);
                            NikonDevicePropDescDS<UInt16> gain = new();
                            return result.TryGetUInt16(ref gain) ? gain.GetSet > 0 : false;
                        }
                        return false;
                    } catch {
                        throw;
                    }
                }
                return false;
            }
        }
        public int GainMax { get; }
        public int GainMin { get; }
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
            set {
                if (Connected) {
                    try {
                        if (this.cameraInfo.DevicePropertiesSupported.Contains(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx)) {
                            this.camera.SetDevicePropValue(NEKCS.NikonMtpDevicePropCode.ExposureIndexEx, new MtpDatatypeVariant((UInt32)value));
                        } else {
                            this.camera.SetDevicePropValue(NEKCS.NikonMtpDevicePropCode.ExposureIndex, new MtpDatatypeVariant((UInt16)value));
                        }
                        RaisePropertyChanged();
                    } catch {
                        throw;
                    }
                }
            }
        }

        public IList<double> Exposures {
            get {
                if (Connected) {
                    try {
                        var result = this.camera.GetDevicePropDesc(NEKCS.NikonMtpDevicePropCode.ExposureTime);
                        NikonDevicePropDescDS<UInt32> exp = new();
                        if (!result.TryGetUInt32(ref exp)) return new List<double>();
                        var exps = exp.EnumFORM.ToList();
                        exps.Remove(0xFFFFFFFF); //Bulb
                        exps.Remove(0xFFFFFFFD); //Time
                        return exps.Select(x => x / 10000.0).ToList();
                    } catch {
                        throw;
                    }
                }
                return new List<double>();
            }
        }
        public double ExposureMin {
            get {
                if (Connected) {
                    return Exposures.Min();
                }
                return double.NaN;
            }
        }
        public double ExposureMax { //TODO: set to infinity or large value when in bulb/time
            get {
                if (Connected) {
                    return Exposures.Max();
                }
                return double.NaN;
            }
        }
        public double ExposureTime {
            get {
                if (Connected) {
                    try {
                        var result = this.camera.GetDevicePropValue(NEKCS.NikonMtpDevicePropCode.ExposureTime);
                        UInt32 exp = 0;
                        return result.TryGetUInt32(ref exp) ? (double)exp / 10000.0 : 0;
                    } catch {
                        throw;
                    }
                }
                return 0;
            }
            set {
                if (Connected) {
                    try {
                        UInt32 newExp = (UInt32)(Exposures.OrderBy(x => Math.Abs(value - x)).First() * 10000);
                        this.camera.SetDevicePropValue(NikonMtpDevicePropCode.ExposureTime, new MtpDatatypeVariant(newExp));
                        RaisePropertyChanged();
                    } catch {
                        throw;
                    }
                }
            }
        }


        private void camPropEvent(NEKCS.NikonCamera cam, NEKCS.MtpEvent e) {
            if (e.eventCode == NikonMtpEventCode.DeviceInfoChanged) {
                if (Connected) {
                    //this.cameraInfo = this.camera.GetDeviceInfo();
                    //RaiseAllPropertiesChanged();
                } else {
                    this.Disconnect();
                }
            } else if (e.eventCode == NikonMtpEventCode.DevicePropChanged) {
                switch ((NikonMtpDevicePropCode)e.eventParams[0]) {
                    case NikonMtpDevicePropCode.BatteryLevel:
                        RaisePropertyChanged("BatteryLevel");
                        break;
                    case NikonMtpDevicePropCode.ExposureIndexEx:
                    case NikonMtpDevicePropCode.ExposureIndex:
                        RaisePropertyChanged("Gain");
                        break;
                    case NikonMtpDevicePropCode.ExposureTime:
                        RaisePropertyChanged("ExposureMin");
                        RaisePropertyChanged("ExposureMax");
                        break;
                    case NikonMtpDevicePropCode.Fnumber:
                    case NikonMtpDevicePropCode.FocalLength:
                        updateLensInfo();
                        break;
                }
            }
        }

        private bool isCpuLensMounted() {
            try {
                var result = camera.GetDevicePropValue(NikonMtpDevicePropCode.LensSort);
                byte lensSort = 0;
                return result.TryGetUInt8(ref lensSort) && lensSort == 1;
            } catch { return false; }
        }
        private void updateLensInfo() {
            if (isCpuLensMounted()) {
                try {
                    var result = camera.GetDevicePropValue(NikonMtpDevicePropCode.FocalLength);
                    UInt32 flength = 0;
                    if (result.TryGetUInt32(ref flength)) {
                        this.profileService.ActiveProfile.TelescopeSettings.FocalLength = flength / 100.0;
                    }
                } catch { }
                try {
                    var result = camera.GetDevicePropValue(NikonMtpDevicePropCode.Fnumber);
                    UInt16 fnumber = 0;
                    if (result.TryGetUInt16(ref fnumber)) {
                        this.profileService.ActiveProfile.TelescopeSettings.FocalRatio = fnumber / 100.0;
                    }
                } catch { }
            }
        }



        private CameraStates _cameraState;
        private readonly object _gateCameraState = new();
        private readonly Dictionary<CameraStates, TaskCompletionSource<bool>> _awaitersCameraState = new();
        private MemoryStream _imageStream;
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

                    if (_awaitersCameraState.TryGetValue(CameraStates.Exposing, out var exp)) {
                        exp.TrySetResult(true);
                    }
                }

                try {
                    NEKCS.MtpParams param = new();
                    param.addUint32(e.eventParams[0] == 0 ? 0xFFFF0001 : e.eventParams[0]);
                    NEKCS.MtpResponse result = this.camera.SendCommandAndRead(NikonMtpOperationCode.GetObject, param);
                    _imageStream = new(result.data);
                    if (_awaitersCameraState.TryGetValue(CameraStates.Download, out var dl)) {
                        dl.TrySetResult(true);
                    }
                } catch {
                    lock (_gateCameraState) {
                        _cameraState = CameraStates.Idle;
                        if (_awaitersCameraState.TryGetValue(CameraStates.Download, out var dl)) {
                            dl.TrySetCanceled();
                        }
                        _cameraState = CameraStates.Download;
                    }
                    throw;
                }
            } else if (e.eventCode == NikonMtpEventCode.LiveViewStateChanged) {
                RaisePropertyChanged("LiveViewEnabled");
            }
        }

        public void StartExposure(CaptureSequence sequence) { //TODO bulb
            lock (_gateCameraState) {
                if (!Connected || _cameraState == CameraStates.Error || _cameraState == CameraStates.NoState) return;

                if (_cameraState != CameraStates.Idle) {
                    AbortExposure(); //Need to add support for CameraStateBusy
                }

                camera.DeviceReady(NikonMtpResponseCode.Device_Busy);

                _awaitersCameraState[CameraStates.Exposing] = new();
                _awaitersCameraState[CameraStates.Download] = new();
                _cameraState = CameraStates.Exposing;
            }

            try {
                this.ExposureTime = sequence.ExposureTime;

                NEKCS.MtpParams param = new();
                param.addUint32(0xFFFFFFFF);
                var result = this.camera.SendCommand(NikonMtpOperationCode.InitiateCaptureRecInSdram, param);
                if (result.responseCode != NikonMtpResponseCode.OK) {
                    throw new NEKCS.MtpException(NikonMtpOperationCode.InitiateCaptureRecInSdram, result.responseCode);
                }
            } catch {
                _awaitersCameraState[CameraStates.Exposing].TrySetCanceled();
                _awaitersCameraState[CameraStates.Download].TrySetCanceled();
                lock (_gateCameraState) {
                    _cameraState = CameraStates.Idle;
                }
                throw;
            }
        }
        public async Task WaitUntilExposureIsReady(CancellationToken token) {
            lock (_gateCameraState) {
                if (_cameraState > CameraStates.Exposing) {
                    return;
                }
            }

            using (token.Register(() => AbortExposure())) {
                if (_awaitersCameraState.TryGetValue(CameraStates.Exposing, out var tcs)) {
                    await Task.Run(() => camera.DeviceReady(NikonMtpResponseCode.Device_Busy));
                    tcs.TrySetResult(true);
                    lock (_gateCameraState) {
                        _cameraState = CameraStates.Download;
                    }
                }
            }
        }

        public void StopExposure() { throw new NotImplementedException(); }

        public void AbortExposure() { //TODO
            if (_awaitersCameraState.TryGetValue(CameraStates.Download, out var tcs)) {
                tcs.TrySetCanceled();
            }
        }

        public async Task<IExposureData> DownloadExposure(CancellationToken token) {
            lock (_gateCameraState) {
                if (_cameraState < CameraStates.Exposing) {
                    return null;
                }
            }

            if (_awaitersCameraState.TryGetValue(CameraStates.Download, out var tcs)) {
                await tcs.Task;
                if (tcs.Task.IsCanceled) {
                    lock (_gateCameraState) {
                        _cameraState = CameraStates.Download;
                    }
                }
            }

            try {
                var rawImageData = _imageStream.ToArray();
                var metaData = new ImageMetaData();
                metaData.FromCamera(this);
                return exposureDataFactory.CreateRAWExposureData(converter: profileService.ActiveProfile.CameraSettings.RawConverter, rawBytes: rawImageData, rawType: "nef", bitDepth: this.BitDepth, metaData: metaData);
            } finally {
                if (_imageStream != null) {
                    _imageStream.Dispose();
                    _imageStream = null;
                }
                lock (_gateCameraState) {
                    _cameraState = CameraStates.Idle;
                }
            }
        }


        int liveviewHeaderSize = -1;
        public bool CanShowLiveView { get => cameraInfo.OperationsSupported.Contains(NikonMtpOperationCode.GetLiveViewImage); }
        public bool LiveViewEnabled {
            get {
                try {
                    byte lvState = 0;
                    camera.GetDevicePropValue(NikonMtpDevicePropCode.RemoteLiveViewStatus).TryGetUInt8(ref lvState);
                    return (lvState == 1);
                } catch {
                    return false;
                }
            }
        }

        public void StartLiveView(CaptureSequence sequence) { camera.StartLiveView(); }

        public Task<IExposureData> DownloadLiveView(CancellationToken token) {
            return Task.Run<IExposureData>(() => {
                NEKCS.MtpParams parameters = new();
                NEKCS.MtpResponse response;
                try {
                    response = camera.SendCommandAndRead(NikonMtpOperationCode.GetLiveViewImage, parameters);
                    if (response.responseCode != NikonMtpResponseCode.OK) {
                        return null;
                    }
                } catch {
                    return null;
                }

                if (liveviewHeaderSize < 0) {
                    liveviewHeaderSize = 0;
                    for (int i = 1; i < response.data.Length; i++) {
                        if (response.data[i] == 0xD8 && response.data[i - 1] == 0xFF) {
                            liveviewHeaderSize = i - 1;
                        }
                    }
                }

                var imageStream = new MemoryStream(response.data.Skip(liveviewHeaderSize).ToArray());
                JpegBitmapDecoder decoder = new JpegBitmapDecoder(imageStream, BitmapCreateOptions.IgnoreColorProfile, BitmapCacheOption.OnLoad);

                FormatConvertedBitmap bitmap = new FormatConvertedBitmap();
                bitmap.BeginInit();
                bitmap.Source = decoder.Frames[0];
                bitmap.DestinationFormat = System.Windows.Media.PixelFormats.Gray16;
                bitmap.EndInit();

                return exposureDataFactory.CreateImageArrayExposureDataFromBitmapSource(decoder.Frames[0]).Result;
            }, token);
        }

        public void StopLiveView() { camera.EndLiveView(); }

    }
}
