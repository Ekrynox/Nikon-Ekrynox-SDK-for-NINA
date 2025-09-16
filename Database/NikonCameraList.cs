using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

namespace LucasAlias.NINA.NEK.Database {
    public class NikonCameraSpec {
        public string Name { get; set; } = "";
        public int year { get; set; } = -1;
        public string Processor { get; set; } = "";
        public NikonSensorSpec Sensor { get; set; } = new NikonSensorSpec();


        public class NikonSensorSpec {
            public string Name { get; set; } = "";
            public string Type { get; set; } = "";
            public int BitDepth { get; set; } = -1;
            public int ResX { get; set; } = -1;
            public int ResY { get; set; } = -1;
            public double SizeX { get; set; } = double.NaN;
            public double SizeY { get; set; } = double.NaN;
            public double PixelSizeX { get; set; } = double.NaN;
            public double PixelSizeY { get; set; } = double.NaN;
        }

        internal static Dictionary<string, Database.NikonCameraSpec> ReadDatabase() {
            string fileName = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "Database\\NikonCameraList.json");
            string jsonString = File.ReadAllText(fileName);
            return JsonSerializer.Deserialize<Dictionary<string, Database.NikonCameraSpec>>(jsonString)!;
        }
    }
}
