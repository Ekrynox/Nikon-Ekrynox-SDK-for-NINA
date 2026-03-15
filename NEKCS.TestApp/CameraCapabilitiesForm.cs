using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Text.Json.Serialization.Metadata;
using System.Threading.Tasks;
using System.Windows.Forms;


namespace NEKCS.TestApp
{

    public class HexEventArrayConverter : JsonConverter<List<NEKCS.NikonMtpEventCode>>
    {
        public override List<NEKCS.NikonMtpEventCode> Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
        {
            var list = new List<NEKCS.NikonMtpEventCode>();

            while (reader.Read() && reader.TokenType != JsonTokenType.EndArray)
            {
                var s = reader.GetString();
                //list.Add(Convert.ToInt32(s, 16));
            }

            return list;
        }

        public override void Write(Utf8JsonWriter writer, List<NEKCS.NikonMtpEventCode> value, JsonSerializerOptions options)
        {
            writer.WriteStartArray();
            foreach (var v in value)
                writer.WriteStringValue($"0x{v:X}");
            writer.WriteEndArray();
        }
    }
    public class HexOpArrayConverter : JsonConverter<List<NEKCS.NikonMtpOperationCode>>
    {
        public override List<NEKCS.NikonMtpOperationCode> Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
        {
            var list = new List<NEKCS.NikonMtpOperationCode>();

            while (reader.Read() && reader.TokenType != JsonTokenType.EndArray)
            {
                var s = reader.GetString();
                //list.Add(Convert.ToInt32(s, 16));
            }

            return list;
        }

        public override void Write(Utf8JsonWriter writer, List<NEKCS.NikonMtpOperationCode> value, JsonSerializerOptions options)
        {
            writer.WriteStartArray();
            foreach (var v in value)
                writer.WriteStringValue($"0x{v:X}");
            writer.WriteEndArray();
        }
    }
    public class HexDevicePropArrayConverter : JsonConverter<List<NEKCS.NikonMtpDevicePropCode>>
    {
        public override List<NEKCS.NikonMtpDevicePropCode> Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
        {
            var list = new List<NEKCS.NikonMtpDevicePropCode>();

            while (reader.Read() && reader.TokenType != JsonTokenType.EndArray)
            {
                var s = reader.GetString();
                //list.Add(Convert.ToInt32(s, 16));
            }

            return list;
        }

        public override void Write(Utf8JsonWriter writer, List<NEKCS.NikonMtpDevicePropCode> value, JsonSerializerOptions options)
        {
            writer.WriteStartArray();
            foreach (var v in value)
                writer.WriteStringValue($"0x{v:X}");
            writer.WriteEndArray();
        }
    }
    public class HexUShortArrayConverter : JsonConverter<List<ushort>>
    {
        public override List<ushort> Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
        {
            var list = new List<ushort>();

            while (reader.Read() && reader.TokenType != JsonTokenType.EndArray)
            {
                var s = reader.GetString();
                //list.Add(Convert.ToInt32(s, 16));
            }

            return list;
        }

        public override void Write(Utf8JsonWriter writer, List<ushort> value, JsonSerializerOptions options)
        {
            writer.WriteStartArray();
            foreach (var v in value)
                writer.WriteStringValue($"0x{v:X}");
            writer.WriteEndArray();
        }
    }



    public partial class CameraCapabilitiesForm : Form
    {
        private NEKCS.NikonCamera camera;
        private SynchronizationContext? _syncContext;


        public CameraCapabilitiesForm(NEKCS.NikonCamera camera)
        {
            InitializeComponent();
            _syncContext = SynchronizationContext.Current;

            this.camera = camera;
            camera.OnMtpEvent += new NEKCS.MtpEventHandler(newCamEvent);

            reloadCapabilities();
        }

        void reloadCapabilities() {
            var options = new JsonSerializerOptions {
                WriteIndented = true,
                TypeInfoResolver = new DefaultJsonTypeInfoResolver {
                    Modifiers = {
                        ti => {
                            if (ti.Type == typeof(NEKCS.NikonDeviceInfoDS)) {
                                foreach (var prop in ti.Properties) {
                                    if (prop.Name == nameof(NEKCS.NikonDeviceInfoDS.EventsSupported)) prop.CustomConverter = new HexEventArrayConverter();
                                    if (prop.Name == nameof(NEKCS.NikonDeviceInfoDS.OperationsSupported)) prop.CustomConverter = new HexOpArrayConverter();
                                    if (prop.Name == nameof(NEKCS.NikonDeviceInfoDS.DevicePropertiesSupported)) prop.CustomConverter = new HexDevicePropArrayConverter();
                                    if (prop.Name == nameof(NEKCS.NikonDeviceInfoDS.CaptureFormats)) prop.CustomConverter = new HexUShortArrayConverter();
                                    if (prop.Name == nameof(NEKCS.NikonDeviceInfoDS.ImageFormats)) prop.CustomConverter = new HexUShortArrayConverter();
                                }
                            }
                        }
                    }
                }
            };

            this.CameraCapabilities.Text = "";
            try {
                this.CameraCapabilities.Text += JsonSerializer.Serialize(this.camera.GetDeviceInfo(), options);
            }
            catch (NEKCS.MtpDeviceException e) when (e.Code == MtpExCode.DEVICE_DISCONNECTED) {}
        }

        void newCamEvent(NEKCS.NikonCamera cam, NEKCS.MtpEvent e)
        {
            _syncContext?.Post(_ =>
            {
                if (e.EventCode == NikonMtpEventCode.DeviceInfoChanged) {
                    reloadCapabilities();
                }
                
            }, null);
        }

        private void CameraCapabilitiesForm_FormClosing(object sender, FormClosingEventArgs e)
        {

        }

        private void CameraCapabilitiesForm_Load(object sender, EventArgs e)
        {

        }
    }
}
