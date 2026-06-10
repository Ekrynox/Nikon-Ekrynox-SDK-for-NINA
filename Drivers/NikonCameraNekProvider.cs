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
	public class NikonCameraNekProvider : IEquipmentProvider<ICamera> {
		private IProfileService profileService;
        private IExposureDataFactory exposureDataFactory;
        private ICameraMediator cameraMediator;
        private IFocuserMediator focuserMediator;
        private ITelescopeMediator telescopeMediator;

        [ImportingConstructor]
		public NikonCameraNekProvider(IProfileService profileService, IExposureDataFactory exposureDataFactory, ICameraMediator cameraMediator, IFocuserMediator focuserMediator, ITelescopeMediator telescopeMediator) {
			this.profileService = profileService;
            this.exposureDataFactory = exposureDataFactory;
            this.cameraMediator = cameraMediator;
            this.focuserMediator = focuserMediator;
            this.telescopeMediator = telescopeMediator;
        }

		public string Name => "Nikon";

		public IList<ICamera> GetEquipment() {
			var devices = new List<ICamera>();

			var nekCameraList = NEKCS.NikonCamera.getNikonCameras();
			foreach (var camera in nekCameraList) {
			    devices.Add(new NikonCameraNek(camera.Item1, camera.Item2, profileService, exposureDataFactory, cameraMediator, focuserMediator, telescopeMediator));
			}

			return devices;
		}
	}
}
