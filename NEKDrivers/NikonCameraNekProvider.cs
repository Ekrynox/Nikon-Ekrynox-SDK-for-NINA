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



namespace LucasAlias.NINA.NEK.NEKDrivers {
	[Export(typeof(IEquipmentProvider))]
	public class NikonCameraNekProvider : IEquipmentProvider<ICamera> {
		private IProfileService profileService;
        private IExposureDataFactory exposureDataFactory;
        private ICameraMediator cameraMediator;
        private IFocuserMediator focuserMediator;

        [ImportingConstructor]
		public NikonCameraNekProvider(IProfileService profileService, IExposureDataFactory exposureDataFactory, ICameraMediator cameraMediator, IFocuserMediator focuserMediator) {
			this.profileService = profileService;
            this.exposureDataFactory = exposureDataFactory;
            this.cameraMediator = cameraMediator;
            this.focuserMediator = focuserMediator;
        }

		public string Name => "Nikon";

		public IList<ICamera> GetEquipment() {
			var devices = new List<ICamera>();

			var nekCameraList = NEKCS.NikonCamera.listNikonCameras();
			foreach (var camera in nekCameraList) {
			    devices.Add(new NikonCameraNek(camera.Key, camera.Value, profileService, exposureDataFactory, cameraMediator, focuserMediator));
			}

			return devices;
		}
	}
}
