using Accord.Statistics.Moving;
using NEKCS;
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

            private uint _minStepSize = 0;
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

                    _minStepSize = DetectMinStep(token);

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



            public bool IsMoving { get => _ismoving; } //TODO
            public int MaxIncrement { get => 0; } //TODO
            public int MaxStep { get => 0; } //TODO
            public int Position { get => 0; } //TODO
            public double StepSize { get => _minStepSize; } //TODO
            public bool TempCompAvailable { get => false; } //TOCHECK
            public bool TempComp { get => false; set { } } //TOCHECK
            public double Temperature { get => double.NaN; } //TOCHECK

            public Task Move(int position, CancellationToken ct, int waitInMs = 1000) => throw new NotImplementedException();

            public void Halt() => throw new NotImplementedException();


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
            public uint DetectMinStep(CancellationToken token) { //TODO: wait for device ready
                if (!Connected) return 0;
                cameraNek.camera.StartLiveView();

                uint minStepSize = 100;
                uint maxStepSize = 32767;
                uint dir = 1;

                while (minStepSize < maxStepSize) {
                    uint stepSize = (maxStepSize - minStepSize) / 2 + minStepSize;

                    var parameters = new MtpParams();
                    parameters.addUint32(dir);
                    parameters.addUint32(stepSize);
                    _ismoving = true;
                    var response = cameraNek.camera.SendCommand(NikonMtpOperationCode.MfDrive, parameters);
                    var result = cameraNek.camera.DeviceReadyWhile(NikonMtpResponseCode.Device_Busy, token, 500);
                    _ismoving = false;
                    if (token.IsCancellationRequested) return minStepSize >= maxStepSize ? minStepSize : 0;

                    dir = dir == (uint)1 ? (uint)2 : (uint)1;
                    if (response.responseCode == NikonMtpResponseCode.MfDrive_Step_End || result == NikonMtpResponseCode.MfDrive_Step_End) {
                        continue;
                    } else if (response.responseCode == NikonMtpResponseCode.MfDrive_Step_Insufficiency || result == NikonMtpResponseCode.MfDrive_Step_Insufficiency) {
                        minStepSize = stepSize;
                        continue;
                    } else if (response.responseCode == NikonMtpResponseCode.OK && result == NikonMtpResponseCode.OK) {
                        maxStepSize = stepSize;
                        continue;
                    }
                    break;
                }

                cameraNek.camera.EndLiveView();
                return minStepSize >= maxStepSize ? minStepSize : 0;
            }
        }
    }
}
