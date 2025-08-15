using Accord.Statistics.Moving;
using NEKCS;
using Newtonsoft.Json.Linq;
using NINA.Core.Utility;
using NINA.Equipment.Interfaces;
using NINA.Equipment.Interfaces.Mediator;
using NINA.Profile.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace LucasAlias.NINA.NEK.NEKDrivers {
    public partial class NikonCameraNek {
        public class NikonFocuserNek : BaseINPC, IFocuser {

            public NikonFocuserNek(IProfileService profileService, ICameraMediator cameraMediator) {
                this.profileService = profileService;
                this.cameraMediator = cameraMediator;

                if (cameraMediator.GetDevice() != null && cameraMediator.GetDevice() is NikonCameraNek cam && cam.Connected) {
                    cameraNek = cam;
                }
            }

            private NikonCameraNek cameraNek = null;

            private readonly IProfileService profileService;
            private readonly ICameraMediator cameraMediator;

            private UInt32 _minStepSize = 100;
            private UInt32 _maxStepSize = 32767;
            private UInt32 _nbSteps = int.MaxValue;
            private UInt32 _position = 0;
            private bool _ismoving = false;



            public bool HasSetupDialog { get => false; }
            public string Id { get => cameraNek.Id + "LF"; }
            public string Name { get => cameraNek.Name; }
            public string DisplayName { get => cameraNek.Name + " Lens (NEK Experimental)"; }
            public string Category { get => "Nikon"; }
            public bool Connected { get => (cameraNek != null) && (cameraNek.camera != null) && cameraNek.Connected; }
            public string Description { get => "The lens focus driver of your Nikon Camera !"; }
            public string DriverInfo { get => "Nikon Ekrynox SDK"; }
            public string DriverVersion { get => cameraNek.DriverVersion; }


            public Task<bool> Connect(CancellationToken token) {
                return Task.Run(() => {
                    if (!cameraNek.Connected) {
                        return false;
                    }

                    DetectMinStep(token);
                    DetectStepsNb(token);
                    if (token.IsCancellationRequested) return false;

                    cameraNek.camera.OnMtpEvent += new MtpEventHandler(camPropEvent);
                    return true;
                });
            } //TODO

            public void Disconnect() {
                if (cameraNek == null || cameraNek.camera == null) return;

                cameraNek.camera.OnMtpEvent -= new MtpEventHandler(camPropEvent);
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

            public Task Move(int position, CancellationToken ct, int waitInMs = 1000) { //Manage Errors and the time limit & need to lick the deadlock (expect to do move less thant the min increment)
                return Task.Run(() => {
                    if (position == 0) {
                        while (true) {
                            var result = MoveBy(_maxStepSize, false, ct).Result;
                            if (ct.IsCancellationRequested) break;
                            if (result == NikonMtpResponseCode.MfDrive_Step_End) break;
                        }
                    } 
                    else if (position == _nbSteps) {
                        while (true) {
                            var result = MoveBy(_maxStepSize, true, ct).Result;
                            if (ct.IsCancellationRequested) break;
                            if (result == NikonMtpResponseCode.MfDrive_Step_End) break;
                        }
                    }


                    while ((Math.Abs(position - _position) >= _minStepSize) && !ct.IsCancellationRequested) {
                        int steps = position - (int)_position;
                        bool toInf = steps > 0;
                        steps = Math.Abs(steps);
                        steps = Math.Min(steps, (int)_maxStepSize);
                        var result = MoveBy((uint)steps, toInf, ct).Result;
                        if (result != NikonMtpResponseCode.OK) return;
                        if (ct.IsCancellationRequested) return;
                    }
                });
            }

            public void Halt() => throw new NotImplementedException();


            public Task<NikonMtpResponseCode> MoveBy(UInt32 distance, bool toInf, CancellationToken ct) { //Thow error if out of range & Time limited deviceReady
                return Task.Run(() => {
                    cameraNek.camera.StartLiveView(true, ct);

                    var parameters = new MtpParams();
                    parameters.addUint32((UInt32)(toInf ? 2 : 1));
                    parameters.addUint32(distance);
                    _ismoving = true;
                    RaisePropertyChanged("IsMoving");
                    var response = cameraNek.camera.SendCommand(NikonMtpOperationCode.MfDrive, parameters);
                    var result = cameraNek.camera.DeviceReadyWhile(NikonMtpResponseCode.Device_Busy, 100);
                    _ismoving = false;
                    RaisePropertyChanged("IsMoving");

                    if (response.responseCode == NikonMtpResponseCode.MfDrive_Step_End || result == NikonMtpResponseCode.MfDrive_Step_End) {
                        result = NikonMtpResponseCode.MfDrive_Step_End;
                        _position = toInf ? _nbSteps : 0;
                    } else if (response.responseCode == NikonMtpResponseCode.MfDrive_Step_Insufficiency || result == NikonMtpResponseCode.MfDrive_Step_Insufficiency) {
                        result = NikonMtpResponseCode.MfDrive_Step_Insufficiency;
                    } else if (response.responseCode == NikonMtpResponseCode.OK && result == NikonMtpResponseCode.OK) {
                        result = NikonMtpResponseCode.OK;
                        _position += toInf ? distance : 0;
                        _position -= toInf ? 0 : distance;
                    }
                    RaisePropertyChanged("Position");

                    cameraNek.StopLiveView(5000);
                    return result;
                });
            }


            private void camPropEvent(NEKCS.NikonCamera cam, NEKCS.MtpEvent e) {
                if (e.eventCode == NikonMtpEventCode.DevicePropChanged) {
                    switch ((NikonMtpDevicePropCode)e.eventParams[0]) {
                        case NikonMtpDevicePropCode.LensID:
                        case NikonMtpDevicePropCode.LensSort:
                            if (!cameraNek.isFocusDrivableLens()) {
                                cameraNek.focuserMediator.Disconnect();
                            }
                            break;
                    }
                }
            }

            // 1: to 0, 2: to inf, [1 - 32767]
            public void DetectMinStep(CancellationToken token) { //TODO: wait for device ready
                if (!Connected) return;
                cameraNek.camera.StartLiveView();

                UInt32 maxStepSize = (UInt32)_maxStepSize;
                bool toInf = true;

                while (_minStepSize < maxStepSize) {
                    uint stepSize = (maxStepSize - _minStepSize) / 2 + _minStepSize;
                    var result = MoveBy(stepSize, toInf, token);

                    toInf = !toInf;
                    if (result.Result == NikonMtpResponseCode.MfDrive_Step_End) {
                        continue;
                    } else if (result.Result == NikonMtpResponseCode.MfDrive_Step_Insufficiency) {
                        _minStepSize = stepSize;
                        continue;
                    } else if (result.Result == NikonMtpResponseCode.OK) {
                        maxStepSize = stepSize;
                        continue;
                    }
                    break;
                }

                cameraNek.camera.EndLiveView();
            }

            public void DetectStepsNb(CancellationToken token) {
                if (!Connected) return;
                cameraNek.camera.StartLiveView();

                UInt32 nbSteps0 = 0;
                UInt32 nbStepsInf = 0;
                UInt32 stepSize = _maxStepSize;
                NikonMtpResponseCode result;

                while (stepSize >= _minStepSize) {

                    //go to Start then to nbStepInf
                    while (true) {
                        result = MoveBy(_maxStepSize, false, token).Result;
                        if (token.IsCancellationRequested) break;
                        if (result == NikonMtpResponseCode.MfDrive_Step_End) break;
                    }
                    Move((int)nbStepsInf, token).Wait();
                    if (token.IsCancellationRequested) break;

                    while (true) {
                        result = MoveBy(stepSize, true, token).Result;
                        if (token.IsCancellationRequested) return;
                        if (result != NikonMtpResponseCode.OK) break;
                        nbStepsInf += stepSize;
                    }

                    //go to Inf then to Inf - nbSteps0
                    while (true) {
                        result = MoveBy(_maxStepSize, true, token).Result;
                        if (token.IsCancellationRequested) break;
                        if (result == NikonMtpResponseCode.MfDrive_Step_End) break;
                    }
                    Move((int)(_nbSteps - nbSteps0), token).Wait();
                    if (token.IsCancellationRequested) break;

                    while (true) {
                        result = MoveBy(stepSize, false, token).Result;
                        if (token.IsCancellationRequested) return;
                        if (result != NikonMtpResponseCode.OK) break;
                        nbSteps0 += stepSize;
                    }

                    _nbSteps = (nbSteps0 + nbStepsInf) / 2 + stepSize;
                    stepSize = stepSize / 2;
                }

                cameraNek.camera.EndLiveView();
            }


        }
    }
}
