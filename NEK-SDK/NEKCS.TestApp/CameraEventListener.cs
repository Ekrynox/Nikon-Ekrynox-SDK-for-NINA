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


        public CameraEventListener(string devicePath)
        {
            InitializeComponent();
            _syncContext = SynchronizationContext.Current;

            camera = new NEKCS.NikonCamera(devicePath, 2);
            camera.OnMtpEvent += new NEKCS.MtpEventHandler(newCamEvent);


            _cameraShootingForm = new CameraShootingForm(camera);
            _cameraShootingForm.Show();
        }

        void newCamEvent(NEKCS.NikonCamera cam, NEKCS.MtpEvent e)
        {
            _syncContext?.Post(_ =>
            {
                NEKCS.NikonMtpEventCode ecode = (NEKCS.NikonMtpEventCode)e.eventCode;
                this.EventList.Text += "Event: " + Enum.GetName(typeof(NEKCS.NikonMtpEventCode), ecode);
                foreach (var item in e.eventParams)
                {
                    if (ecode == NEKCS.NikonMtpEventCode.DevicePropChanged)
                    {
                        this.EventList.Text += " -> " + Enum.GetName(typeof(NEKCS.NikonMtpDevicePropCode), item);
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
            camera.Dispose();
        }

        private void CameraEventListener_Load(object sender, EventArgs e)
        {

        }
    }
}
