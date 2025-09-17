using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace LucasAlias.NINA.NEK.Database {
    public class NikonCameraSpec {
        public string Name { get; set; } = "";
        public int Year { get; set; } = -1;
        public string Processor { get; set; } = "";
        public NikonSensorSpec Sensor { get; set; } = new NikonSensorSpec();
        [JsonPropertyName("CropSubSampling")]
        public List<CropSubSamplingClass> CropSubSampling { get; set; } = new List<CropSubSamplingClass>();


        public class NikonSensorSpec {
            public string Name { get; set; } = "";
            public string Type { get; set; } = "";
            public string Bayer { get; set; } = "";
            public int BitDepth { get; set; } = -1;
            public int ResX { get; set; } = -1;
            public int ResY { get; set; } = -1;
            public double SizeX { get; set; } = double.NaN;
            public double SizeY { get; set; } = double.NaN;
            public double PixelSizeX { get; set; } = double.NaN;
            public double PixelSizeY { get; set; } = double.NaN;
        }

        public class CropSubSamplingClass {
            [JsonPropertyName("Crop")]
            public String Crop { get; set; }
            [JsonPropertyName("Subs")]
            public Dictionary<string, SubSamplingClass> Subs { get; set; }

            public class SubSamplingClass {
                [JsonPropertyName("ResX")]
                public int ResX { get; set; } = -1;
                [JsonPropertyName("ResY")]
                public int ResY { get; set; } = -1;
            }
        }

        internal static Dictionary<string, Database.NikonCameraSpec> ReadDatabase() {
            string fileName = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "Database\\NikonCameraList.json");
            string jsonString = File.ReadAllText(fileName);
            return JsonSerializer.Deserialize<Dictionary<string, Database.NikonCameraSpec>>(jsonString)!;
        }
    }
}
