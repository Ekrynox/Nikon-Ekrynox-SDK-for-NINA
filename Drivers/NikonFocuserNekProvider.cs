using NINA.Core.Utility;
using NINA.Equipment.Interfaces;
using NINA.Equipment.Interfaces.Mediator;
using NINA.Equipment.Interfaces.ViewModel;
using NINA.Image.Interfaces;
using NINA.Profile.Interfaces;
using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;



namespace LucasAlias.NINA.NEK.Drivers {
    [Export(typeof(IEquipmentProvider))]
    public class NikonCameraNekFocuserProvider : IEquipmentProvider<IFocuser> {
        private IProfileService profileService;
        private ICameraMediator cameraMediator;

        [ImportingConstructor]
        public NikonCameraNekFocuserProvider(IProfileService profileService, ICameraMediator cameraMediator) {
            this.profileService = profileService;
            this.cameraMediator = cameraMediator;
        }

        public string Name => "Nikon";

        public IList<IFocuser> GetEquipment() {
            var devices = new List<IFocuser>();

            if (cameraMediator.GetDevice() != null && cameraMediator.GetDevice().Connected) {
                if (cameraMediator.GetDevice() is NikonCameraNek cam) {
                    if (cam.isFocusDrivableLens()) {
                        devices.Add(new NikonCameraNek.NikonFocuserNek(profileService, cameraMediator));
                    }
                }
            }

            return devices;
        }
    }
}
