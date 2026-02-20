using Accord.Statistics.Moving;
using RelayCommand = CommunityToolkit.Mvvm.Input.RelayCommand;
using AsyncRelayCommand = CommunityToolkit.Mvvm.Input.AsyncRelayCommand;
using NEKCS;
using Newtonsoft.Json.Linq;
using NINA.Core.Enum;
using NINA.Core.Utility;
using NINA.Core.Utility.Notification;
using NINA.Equipment.Interfaces;
using NINA.Equipment.Interfaces.Mediator;
using NINA.Profile.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;
using System.Windows.Markup.Localizer;
using System.Diagnostics.Eventing.Reader;
using System.Windows;

namespace LucasAlias.NINA.NEK.Drivers {
    public partial class NikonCameraNek {
        public class NikonFocuserNek : BaseINPC, IFocuser {
            public const string sourceFile = @"NEKDrivers\NikonFocuserNEK.cs";

            public NikonFocuserNek(IProfileService profileService, ICameraMediator cameraMediator, IFocuserMediator focuserMediator) {
                this.profileService = profileService;
                this.cameraMediator = cameraMediator;
                this.focuserMediator = focuserMediator;

                StartCalibration = new AsyncRelayCommand(async () => {
                    if (!this.Connected) return;
                    this._CalibrationToken = new CancellationTokenSource();
                    await Task.Run(() => this.Calibrate(this._CalibrationToken.Token));
                    if (this._CalibrationToken.IsCancellationRequested) {
                        Logger.Info("Calibration have been canceled.", "Calibrate", sourceFile);
                        Notification.ShowWarning("NekFocuser: Calibration have been canceled.\nThe Focuser will certainly behave wrongly!", TimeSpan.FromSeconds(10));
                        return;
                    }
                });
                CancelCalibration = new RelayCommand(() => this._CalibrationToken?.Cancel());

                Integration.LensAF.RegisterFocuser.Send(this.DisplayName);
            }

            private NikonCameraNek cameraNek { get => this.cameraMediator.GetDevice() != null && this.cameraMediator.GetDevice() is NikonCameraNek cam && cam.Connected ? cam : null; }

            private readonly IProfileService profileService;
            private readonly ICameraMediator cameraMediator;
            private readonly IFocuserMediator focuserMediator;

            private bool _isConnected = false;

            private UInt32 _minStepSize;
            private UInt32 _maxStepSize;
            private UInt32 _nbSteps;
            private UInt32 _position;
            private bool _ismoving;



            public bool HasSetupDialog { get => false; }
            public string Id { get => "NEKLF"; }
            public string Name { get => cameraNek != null ? "Nikon " + cameraNek.LensName : "NEK Lens Focuser"; }
            public string DisplayName { get => cameraNek != null ? "Nikon " + cameraNek.LensName + " (NEK)" : "Nikon Lens Focuser (NEK)"; }
            public string Category { get => "Nikon"; }
            public bool Connected { get => this._isConnected && this.cameraNek != null; }
            public string Description { get => "The lens focus driver of your Nikon Camera !"; }
            public string DriverInfo { get => "Nikon Ekrynox SDK"; }
            public string DriverVersion { get => cameraNek != null ? cameraNek.DriverVersion : ""; }


            public Task<bool> Connect(CancellationToken token) {
                return Task.Run(() => {
                    Logger.Info("Start connecting to the Lens Focuser for the Camera: " + this.Name, "Connect", sourceFile);

                    if (cameraNek == null) {
                        Logger.Error("No camera are connected with NEK inside NINA!", "Connect", sourceFile);
                        Notification.ShowError("No camera are connected with NEK inside NINA!");
                        return false;
                    }

                    try {
                        var result = cameraNek.camera.GetDevicePropValue(NikonMtpDevicePropCode.FocusMode);
                        if (result.TryGetUInteger(out var focus)) {
                            if (focus == 0x0001) {
                                Logger.Info("The camera is in Manual Focus.", "Connect", sourceFile);
                                Notification.ShowError("The lens focuser cannot work in MF mode. Please switch to AF (recommended: AF-S).");
                                return false;
                            }
                        } else {
                            Logger.Error("Wrong Datatype UInteger! Expected: " + result.GetType().ToString() + " for FocusMode on " + this.Name, "Connect", sourceFile);
                        }
                    } catch (MtpDeviceException e) {
                        Logger.Error("Error while checking Focus mode: " + this.Name, e, "Connect", sourceFile);
                        return false;
                    } catch (MtpException e) {
                        Logger.Error("Error while checking Focus mode: " + this.Name, e, "Connect", sourceFile);
                        return false;
                    }

                    this._isConnected = true;
                    this._ismoving = false;

                    cameraNek.camera.OnMtpEvent += camPropEvent;

                    if (token.IsCancellationRequested || !this.Connected) {
                        this._isConnected = false;
                        this._ismoving = false;
                        return false;
                    }

                    this.focuserMediator.Connected += OnConnected;

                    return true;
                });
            }

            private async Task OnConnected(object arg1, EventArgs args) {
                this.focuserMediator.Connected -= OnConnected;
                if (focuserMediator.GetDevice() == this && Connected) {
                    await Application.Current.Dispatcher.BeginInvoke(new Action(() => {
                        StartCalibration.Execute(null);
                    }));
                }
            }

            public void Disconnect() {
                if (!_isConnected) return;
                Logger.Info("Start diconnecting from the Lens Focuser for the Camera.", "Diconnect", sourceFile);
                _isConnected = false;
                RaiseAllPropertiesChanged();

                if (cameraNek != null || cameraNek.camera != null) cameraNek.camera.OnMtpEvent -= camPropEvent;
            }

            public void SetupDialog() => throw new NotImplementedException();

            public IList<string> SupportedActions { get => new List<string>(); }

            public string Action(string actionName, string actionParameters) => throw new NotImplementedException();

            public string SendCommandString(string command, bool raw = true) => throw new NotImplementedException();

            public bool SendCommandBool(string command, bool raw = true) => throw new NotImplementedException();

            public void SendCommandBlind(string command, bool raw = true) => throw new NotImplementedException();



            public bool IsMoving { get => _ismoving; }
            public int MaxIncrement { get => (int)_maxStepSize; }
            public int MaxStep { get => (int)_nbSteps; }
            public int Position { get => (int)_position; }
            public double StepSize { get => (double)_minStepSize; }
            public bool TempCompAvailable { get => false; } //TOCHECK
            public bool TempComp { get => false; set { } } //TOCHECK
            public double Temperature { get => double.NaN; } //TOCHECK

            public Task Move(int position, CancellationToken ct, int waitInMs = 1000) => this.Move(position, ct, waitInMs, true);
            private Task Move(int position, CancellationToken ct, int waitInMs, bool needInit) { //Manage Errors and the time limit
                return Task.Run(() => {
                    if (!Connected) return;
                    if (needInit) if (!InitFocusingProcess()) return;

                    if (position <= 0) {
                        _position = _nbSteps;
                        while (true) {
                            var result = MoveBy(_maxStepSize, false, ct, false).Result;
                            if (ct.IsCancellationRequested) break;
                            if (result == NikonMtpResponseCode.MfDrive_Step_End || result == NikonMtpResponseCode.MfDrive_Step_Insufficiency) {
                                _ = MoveBy(_maxStepSize, false, ct, false).Result; //To Ensure we are at the End on older Camera (TO RECHECK)
                                break;
                            }
                        }
                    } else if (position >= (int)_nbSteps) {
                        _position = 0;
                        while (true) {
                            var result = MoveBy(_maxStepSize, true, ct, false).Result;
                            if (ct.IsCancellationRequested) break;
                            if (result == NikonMtpResponseCode.MfDrive_Step_End || result == NikonMtpResponseCode.MfDrive_Step_Insufficiency) {
                                _ = MoveBy(_maxStepSize, true, ct, false).Result; //To Ensure we are at the End on older Camera (TO RECHECK)
                                break;
                            }
                        }
                    } else {
                        while (position != (int)_position && !ct.IsCancellationRequested) {
                            int steps = position - (int)_position;
                            bool toInf = steps >= 0;
                            steps = Math.Abs(steps);
                            steps = Math.Min(steps, (int)_maxStepSize);

                            if (steps < _minStepSize) {
                                if (toInf) {
                                    if (_position >= _minStepSize) {
                                        steps = (int)_minStepSize;
                                        toInf = false;
                                    } else {
                                        steps += (int)_minStepSize;
                                    }
                                }
                                else {
                                    if (_position + _minStepSize <= _nbSteps) {
                                        steps = (int)_minStepSize;
                                        toInf = true;
                                    } else {
                                        steps += (int)_minStepSize;
                                    }
                                }
                            }

                            var result = MoveBy((uint)steps, toInf, ct, false).Result;
                            if ((result != NikonMtpResponseCode.OK)) break;
                            if (ct.IsCancellationRequested) break;
                        }
                    }

                    if (needInit) StopFocusingProcess();
                });
            }

            public void Halt() => throw new NotImplementedException();


            private bool InitFocusingProcess() {
                if (this.cameraNek == null) return false;
                lock (this.cameraNek._gateCameraState) {
                    if (this.cameraNek._cameraState != CameraStates.Idle) {
                        Logger.Warning("Could not Init Focusing process because camera is busy.", "InitFocusingProcess", sourceFile);
                        Notification.ShowWarning("Nikon NEK: Could not Init Focusing process because camera is busy.", TimeSpan.FromSeconds(10));
                        return false;
                    }

                    this.cameraNek._awaitersCameraState[CameraStates.Waiting] = new();
                    this.cameraNek._cameraState = CameraStates.Waiting;
                    this.cameraNek.RaisePropertyChanged(nameof(this.cameraNek.CameraState));
                }
                this.cameraNek.StartLiveViewBackground();
                _ismoving = true;
                RaisePropertyChanged(nameof(IsMoving));
                return true;
            }

            private void StopFocusingProcess() {
                if (this.cameraNek == null) return;
                _ismoving = false;
                RaisePropertyChanged(nameof(IsMoving));
                this.cameraNek.StopLiveViewBackground(5000);
                lock (this.cameraNek._gateCameraState) { if (this.cameraNek._cameraState == CameraStates.Waiting) this.cameraNek._cameraState = CameraStates.Idle; }
                if (this.cameraNek._awaitersCameraState.TryGetValue(CameraStates.Waiting, out var wtcs)) wtcs.TrySetResult(true);
                this.cameraNek.RaisePropertyChanged(nameof(this.cameraNek.CameraState));
            }

            private Task<NikonMtpResponseCode> MoveBy(UInt32 distance, bool toInf, CancellationToken ct) => this.MoveBy(distance, toInf, ct);
            private Task<NikonMtpResponseCode> MoveBy(UInt32 distance, bool toInf, CancellationToken ct, bool needInit) { //Time limited deviceReady for when stucked
                return Task.Run(() => {
                    if (!Connected) return NikonMtpResponseCode.General_Error;
                    if (needInit) if (!InitFocusingProcess()) return NikonMtpResponseCode.General_Error;

                    var parameters = new MtpParams();
                    parameters.addUint32((UInt32)(toInf ? 2 : 1));
                    parameters.addUint32(distance);
                    var response = cameraNek.camera.SendCommand(NikonMtpOperationCode.MfDrive, parameters);
                    var result = cameraNek.camera.DeviceReadyWhile(NikonMtpResponseCode.Device_Busy, 20);

                    if (response.responseCode == NikonMtpResponseCode.MfDrive_Step_Insufficiency || result == NikonMtpResponseCode.MfDrive_Step_Insufficiency) {
                        result = NikonMtpResponseCode.MfDrive_Step_Insufficiency;
                        _position = toInf ? _nbSteps : 0;
                    } else if (response.responseCode == NikonMtpResponseCode.MfDrive_Step_End || result == NikonMtpResponseCode.MfDrive_Step_End) {
                        result = NikonMtpResponseCode.MfDrive_Step_End;
                        _position = toInf ? _nbSteps : 0;
                    } else if (response.responseCode == NikonMtpResponseCode.OK && result == NikonMtpResponseCode.OK) {
                        result = NikonMtpResponseCode.OK;
                        _position += toInf ? distance : 0;
                        _position -= toInf ? 0 : distance;
                    } else {
                        var e = new MtpException(NikonMtpOperationCode.MfDrive, response.responseCode);
                        Logger.Error(e, "MoveBy", sourceFile);
                        if (needInit) StopFocusingProcess();
                        throw e;
                    }
                    RaisePropertyChanged(nameof(Position));

                    if (needInit) StopFocusingProcess();

                    return result;
                });
            }


            private void camPropEvent(NEKCS.NikonCamera cam, NEKCS.MtpEvent e) {
                if (e.eventCode == NikonMtpEventCode.DevicePropChanged) {
                    switch ((NikonMtpDevicePropCode)e.eventParams[0]) {
                        case NikonMtpDevicePropCode.LensID:
                        case NikonMtpDevicePropCode.LensSort:
                            if (Connected && !this.cameraNek.isFocusDrivableLens()) {
                                Notification.ShowError("Nikon NEK: The lens have been unmounted!");
                                this.focuserMediator.Disconnect();
                            }
                            break;
                        case NikonMtpDevicePropCode.FocalLength:
                        case NikonMtpDevicePropCode.Fnumber:
                            Notification.ShowWarning("Nikon NEK: The Focal Length or Aperture have changed. We recommended to rerun calibration (disconnect then reconnect the focuser).", TimeSpan.FromSeconds(10));
                            break;
                    }
                }
            }


            private CancellationTokenSource _CalibrationToken;
            public ICommand StartCalibration { get; }
            public ICommand CancelCalibration { get; }

            public bool Calibrate(CancellationToken token) {
                this._minStepSize = 32;
                this._maxStepSize = 32767;
                this._nbSteps = int.MaxValue;
                this._position = 0;
                bool isOk = true;

                Logger.Info("Start detecting the max step.", "Calibrate", sourceFile);
                isOk = DetectMaxStep(token);
                if (token.IsCancellationRequested) {
                    Logger.Info("Detected max step after cancellation: " + this._maxStepSize, "Calibrate", sourceFile);
                    return false;
                }
                if (isOk) Logger.Info("Detected max step: " + this._maxStepSize, "Calibrate", sourceFile);
                else if (!isOk) {
                    Logger.Error("Focuser failed to detect the max step.", "Calibrate", sourceFile);
                    Notification.ShowError("Nikon NEK: Focuser failed to detect the max step.");
                    return false;
                }

                /*Logger.Info("Start detecting the min step.", "Calibrate", sourceFile);
                DetectMinStep(token);
                Logger.Info("Detected min step: " + this._minStepSize, "Calibrate", sourceFile);
                if (token.IsCancellationRequested) {
                    return;
                }*/
                Logger.Info("Start detecting the number of steps.", "Calibrate", sourceFile);
                DetectStepsNb(token);
                if (token.IsCancellationRequested) {
                    Logger.Info("Detected number of steps after cancellation: " + this._nbSteps, "Calibrate", sourceFile);
                    return false;
                }
                else if (isOk) Logger.Info("Detected number of steps: " + this._nbSteps, "Calibrate", sourceFile);
                else if (!isOk) {
                    Logger.Error("Focuser failed to detect the number of steps.", "Calibrate", sourceFile);
                    Notification.ShowError("Nikon NEK: Focuser failed to detect the number of steps.");
                    return false;
                }


                Move((int)_nbSteps, token).Wait(); //Go to Inf
                Integration.LensAF.GotoFocus.Send();

                return true;
            }


            public void DetectMinStep(CancellationToken token) {
                return; //NikonMtpResponseCode.MfDrive_Step_Insufficiency seem to indicate a already reached end on some lens => the function need to be rethinked
                if (!Connected) return;
                UInt32 stepSize = _minStepSize;
                UInt32 maxStepSize = _maxStepSize;
                bool toInf = true;

                InitFocusingProcess();

                while ((_minStepSize < maxStepSize) && (maxStepSize != stepSize)) {
                    var result = MoveBy(stepSize, toInf, token, false);
                    if (token.IsCancellationRequested) break;

                    toInf = !toInf;
                    if (result.Result == NikonMtpResponseCode.MfDrive_Step_End) {
                        continue;
                    } else if (result.Result == NikonMtpResponseCode.MfDrive_Step_Insufficiency) {
                        _minStepSize = stepSize;
                        stepSize = (UInt32)Math.Floor((maxStepSize + _minStepSize) / 2.0);
                        continue;
                    } else if (result.Result == NikonMtpResponseCode.OK) {
                        maxStepSize = stepSize;
                        stepSize = (UInt32)Math.Floor((maxStepSize + _minStepSize) / 2.0);
                        continue;
                    }
                    break;
                }

                StopFocusingProcess();
            }

            public bool DetectMaxStep(CancellationToken token) {
                if (!Connected) return false;
                UInt32 stepSize = _maxStepSize;
                UInt32 minStepSize = _minStepSize;
                bool toInf = true;
                bool isOk = true;

                if (!InitFocusingProcess()) return false;

                while ((minStepSize < _maxStepSize) && (minStepSize != stepSize)) {
                    Move(!toInf ? (int)_nbSteps : 0, token, 1000, false).Wait(CancellationToken.None);
                    if (token.IsCancellationRequested) break;
                    var result = MoveBy(stepSize, toInf, token, false).Result;
                    if (token.IsCancellationRequested) break;

                    toInf = !toInf;
                    if (result == NikonMtpResponseCode.MfDrive_Step_End || result == NikonMtpResponseCode.MfDrive_Step_Insufficiency) {
                        _maxStepSize = stepSize;
                        RaisePropertyChanged(nameof(this.MaxIncrement));
                        stepSize = (UInt32)Math.Floor((_maxStepSize + minStepSize) / 2.0);
                        continue;
                    } else if (result == NikonMtpResponseCode.OK) {
                        minStepSize = stepSize;
                        stepSize = (UInt32)Math.Floor((_maxStepSize + minStepSize) / 2.0);
                        continue;
                    } else {
                        isOk = false;
                    }
                    break;
                }

                StopFocusingProcess();

                if (token.IsCancellationRequested) return false;
                return isOk;
            }

            public bool DetectStepsNb(CancellationToken token) {
                if (!Connected) return false;
                UInt32 nbSteps0 = 0;
                UInt32 nbStepsInf = 0;
                UInt32 stepSize = _maxStepSize;

                if (!InitFocusingProcess()) return false;

                while (stepSize >= 1) {

                    //go to Start then to nbStepInf
                    Move(0, token, 1000, false).Wait(CancellationToken.None);
                    if (token.IsCancellationRequested) break;

                    while (nbStepsInf < _nbSteps) {
                        Move((int)nbStepsInf + (int)stepSize, token, 1000, false).Wait(CancellationToken.None);
                        if (_position >= _nbSteps) break;
                        nbStepsInf += stepSize;
                        if (token.IsCancellationRequested) break;
                    }
                    if (token.IsCancellationRequested) break;

                    //go to Inf then to Inf - nbSteps0
                    Move((int)_nbSteps, token, 1000, false).Wait(CancellationToken.None);
                    if (token.IsCancellationRequested) break;

                    while (nbSteps0 < _nbSteps) {
                        Move((int)_nbSteps - (int)nbSteps0 - (int)stepSize, token, 1000, false).Wait(CancellationToken.None);
                        if (_position <= 0) break;
                        nbSteps0 += stepSize;
                        if (token.IsCancellationRequested) break;
                    }
                    if (token.IsCancellationRequested) break;

                    _nbSteps = Math.Min(nbSteps0, nbStepsInf);
                    nbSteps0 = _nbSteps;
                    nbStepsInf = _nbSteps;
                    _nbSteps += stepSize;
                    RaisePropertyChanged(nameof(this.MaxStep));
                    stepSize = (UInt32)Math.Floor(stepSize / 2.0);
                }

                StopFocusingProcess();

                if (token.IsCancellationRequested) return false;
                return true;
            }


        }
    }
}
