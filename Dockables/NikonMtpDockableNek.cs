using LucasAlias.NINA.NEK.Drivers;
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

        private readonly ICameraMediator cameraMediator;


        [ImportingConstructor]
        public NikonMtpDockableNek(IProfileService profileService, ICameraMediator cameraMediator) : base(profileService) {
            this.cameraMediator = cameraMediator;
            this.cameraMediator.RegisterConsumer(this);

            this.Title = "Nikon MTP Settings (NEK)";
        }

        public void Dispose() {
            this.cameraMediator.RemoveConsumer(this);
        }

        public CameraInfo CameraInfo { get; private set; }
        public Boolean Connected {
            get => cameraNek != null && cameraNek.Connected;
        }
        public void UpdateDeviceInfo(CameraInfo deviceInfo) {
            this.CameraInfo = deviceInfo;
            RaisePropertyChanged(nameof(CameraInfo));
            RaisePropertyChanged(nameof(Connected));
        }
    }
}
