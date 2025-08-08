using NINA.Core.Utility;
using NINA.Equipment.Interfaces;
using NINA.Equipment.Interfaces.ViewModel;
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

		[ImportingConstructor]
		public NikonCameraNekProvider(IProfileService profileService) {
			this.profileService = profileService;
		}

		public string Name => "Nikon";

		public IList<ICamera> GetEquipment() {
			var devices = new List<ICamera>();

			var nekCameraList = NEKCS.NikonCamera.listNikonCameras();
			foreach (var camera in nekCameraList) {
			    devices.Add(new NikonCameraNek(camera.Key, camera.Value));
			}

			return devices;
		}
	}
}
