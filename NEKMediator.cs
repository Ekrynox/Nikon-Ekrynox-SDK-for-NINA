using NINA.Plugin.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LucasAlias.NINA.NEK {
    internal static class NEKMediator {
        public static NEK Plugin { get; private set; }
        public static IMessageBroker MessageBroker { get; private set; }

        public static Dictionary<string, Database.NikonCameraSpec> CameraList { get; private set; }


        internal static void InitMediator(NEK plugin, IMessageBroker messageBroker) {
            Plugin = plugin;
            MessageBroker = messageBroker;

            CameraList = Database.NikonCameraSpec.ReadDatabase();
        }
    }
}
