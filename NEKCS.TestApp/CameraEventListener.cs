using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;


namespace NEKCS.TestApp
{
    public partial class CameraEventListener : Form
    {
        private NEKCS.NikonCamera camera;
        private SynchronizationContext? _syncContext;
        private CameraShootingForm? _cameraShootingForm;
        private CameraCapabilitiesForm? _cameraCapabilitiesForm;


        public CameraEventListener(NEKCS.MtpConnectionInfo connectionInfo)
        {
            InitializeComponent();
            _syncContext = SynchronizationContext.Current;

            camera = new NEKCS.NikonCamera(connectionInfo);
            camera.OnMtpEvent += new NEKCS.MtpEventHandler(newCamEvent);

            _cameraShootingForm = new CameraShootingForm(camera);
            _cameraShootingForm.Show();

            _cameraCapabilitiesForm = new CameraCapabilitiesForm(camera);
            _cameraCapabilitiesForm.Show();
        }

        void newCamEvent(NEKCS.NikonCamera cam, NEKCS.MtpEvent e)
        {
            _syncContext?.Post(_ =>
            {
                NEKCS.NikonMtpEventCode ecode = e.EventCode;
                this.EventList.Text += "Event: " + Enum.GetName(typeof(NEKCS.NikonMtpEventCode), ecode) + $" 0x{((int)ecode):X}";
                foreach (var item in e.Parameters)
                {
                    if (ecode == NEKCS.NikonMtpEventCode.DevicePropChanged)
                    {
                        this.EventList.Text += " -> " + Enum.GetName(typeof(NEKCS.NikonMtpDevicePropCode), item) + $" 0x{((int)item):X}";
                        //var desc = this.camera.GetDevicePropDesc((NEKCS.NikonMtpDevicePropCode)item);
                    }
                    else
                    {
                        this.EventList.Text += " -> " + item;
                    }
                }
                this.EventList.Text += "\n";

                if (ecode == NEKCS.NikonMtpEventCode.DeviceInfoChanged && !camera.isConnected())
                {
                    this.Close();
                    return;
                }
            }, null);
        }

        private void CameraEventListener_FormClosing(object sender, FormClosingEventArgs e)
        {
            _cameraShootingForm?.Dispose();
            _cameraCapabilitiesForm?.Dispose();
            camera.Dispose();
        }

        private void CameraEventListener_Load(object sender, EventArgs e)
        {

        }
    }
}
