using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LucasAlias.NINA.NEK {
    public static class NEKMediator {
        public static NEK Plugin { get; private set; }
        public static Dictionary<string, Database.NikonCameraSpec> CameraList { get; private set; }


        internal static void RegisterPlugin(NEK plugin) {
            Plugin = plugin;
        }

        internal static void InitDatabase() {
            CameraList = Database.NikonCameraSpec.ReadDatabase();
        }
    }
}
