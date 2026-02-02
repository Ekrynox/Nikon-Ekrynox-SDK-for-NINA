using LucasAlias.NINA.NEK.Drivers;
using NEKCS;
using NINA.Core.Utility;
using NINA.Core.Utility.Notification;
using NINA.Equipment.Equipment.MyCamera;
using NINA.Equipment.Interfaces.Mediator;
using NINA.Equipment.Interfaces.ViewModel;
using NINA.Profile.Interfaces;
using NINA.WPF.Base.ViewModel;
using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LucasAlias.NINA.NEK.Dockables {
    [Export(typeof(IDockableVM))]
    class NikonMtpDockableNek : DockableVM, ICameraConsumer {
        public const string sourceFile = @"NEKDockables\NikonMtpDockableNek.cs";

        private NikonCameraNek cameraNek { get => this.cameraMediator.GetDevice() != null && this.cameraMediator.GetDevice() is NikonCameraNek cam && cam.Connected ? cam : null; }
        private Dictionary<NEKCS.NikonMtpDevicePropCode, NEKCS.NikonDevicePropDescDS_Variant> _deviceProperties;

        private readonly ICameraMediator cameraMediator;

        private Boolean _connected = false;

        [ImportingConstructor]
        public NikonMtpDockableNek(IProfileService profileService, ICameraMediator cameraMediator) : base(profileService) {
            this.cameraMediator = cameraMediator;
            this.cameraMediator.RegisterConsumer(this);
            this.cameraMediator.Connected += CameraConnected;
            this.cameraMediator.Disconnected += CameraDisconnected;

            this.Title = "Nikon MTP Settings (NEK)";

            _deviceProperties = new();
        }

        public void Dispose() {
            this.cameraMediator.RemoveConsumer(this);
            this.cameraMediator.Connected -= CameraConnected;
            this.cameraMediator.Disconnected -= CameraDisconnected;
        }



        public Boolean Connected { get => _connected && cameraNek != null; }
        public Dictionary<NEKCS.NikonMtpDevicePropCode, NEKCS.NikonDevicePropDescDS_Variant> DeviceProperties { get => _deviceProperties; }
        public void UpdateDeviceInfo(CameraInfo deviceInfo) {}

        private async Task CameraConnected(object arg1, EventArgs arg2) {
            if (this.cameraNek != null) {
                foreach (var k in this.cameraNek.cameraInfo.DevicePropertiesSupported) {
                    try {
                        this._deviceProperties.Add(k, this.cameraNek.camera.GetDevicePropDesc(k));
                    } catch (MtpDeviceException e) {
                        Logger.Error("Error while trying to get Device Property Description: " + k.ToString(), e, sourceFile);

                        this._connected = false;
                        this._deviceProperties.Clear();

                        RaiseAllPropertiesChanged();
                    } catch (MtpException e) {
                        Logger.Error("Error while trying to get Device Property Description: " + k.ToString(), e, sourceFile);
                    }
                }

                this._connected = true;
                RaiseAllPropertiesChanged();
            }
        }

        private async Task CameraDisconnected(object arg1, EventArgs arg2) {
            if (this._connected) {
                this._deviceProperties.Clear();

                RaiseAllPropertiesChanged();
            }
            this._connected = false;
        }
    }
}
