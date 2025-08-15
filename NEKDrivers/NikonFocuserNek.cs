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



            public bool IsMoving { get => false; } //TODO
            public int MaxIncrement { get => 0; } //TODO
            public int MaxStep { get => 0; } //TODO
            public int Position { get => 0; } //TODO
            public double StepSize { get => 0; } //TODO
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
                            if (cameraNek.isFocusDrivableLens()) {
                                //Rerun Calibration
                            } else {
                                if (cameraNek.focuserMediator.GetDevice() != null) {
                                    cameraNek.focuserMediator.Disconnect();
                                }
                            }
                            break;
                    }
                }
            }
        }
    }
}
