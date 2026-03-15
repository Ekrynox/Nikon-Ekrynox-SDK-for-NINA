using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Windows.Media.Imaging;

namespace NEKCS.TestApp
{
    public partial class CameraShootingForm : Form
    {
        private NEKCS.NikonCamera camera;
        private SynchronizationContext _syncContext;
        private uint _sdramHandle = 0;

        public CameraShootingForm(NEKCS.NikonCamera camera)
        {
            InitializeComponent();
            _syncContext = SynchronizationContext.Current;

            this.camera = camera;
            camera.OnMtpEvent += new NEKCS.MtpEventHandler(cameraEvent);

            NEKCS.MtpResponse result = camera.SendCommandAndRead(NEKCS.NikonMtpOperationCode.DeleteImagesInSdram, [0]);

            try {
                camera.GetDevicePropValue(NikonMtpDevicePropCode.RemoteLiveViewStatus).TryGetUInt8(out var lvstatus);
                if (lvstatus == 1) camera.EndLiveView();
            }
            catch {}

        }


        private void cameraEvent(NEKCS.NikonCamera cam, NEKCS.MtpEvent e)
        {
            if (e.EventCode == NEKCS.NikonMtpEventCode.ObjectAddedInSdram)
            {
                if (e.Parameters.Length > 0 && e.Parameters[0] != 0)
                {
                    _sdramHandle = e.Parameters[0];
                }
                else
                {
                    _sdramHandle = 0xFFFF0001;
                }

                camera.GetObjectInfo(_sdramHandle);

                NEKCS.MtpResponse result = camera.SendCommandAndRead(NEKCS.NikonMtpOperationCode.GetObject, [ _sdramHandle ]);
                if (result.ResponseCode == NEKCS.NikonMtpResponseCode.OK)
                {
                    _syncContext.Post(delegate
                    {
                        var stream = new MemoryStream(result.Data.ToArray());
                        var decoder = System.Windows.Media.Imaging.BitmapDecoder.Create(stream, System.Windows.Media.Imaging.BitmapCreateOptions.PreservePixelFormat, System.Windows.Media.Imaging.BitmapCacheOption.OnLoad);
                        var frame = decoder.Frames[0];
                        var encoder = new PngBitmapEncoder();
                        encoder.Frames.Add(frame);
                        var ms = new MemoryStream();
                        encoder.Save(ms);
                        PictureBox.Image = new System.Drawing.Bitmap(ms);
                    }, null);
                }
            }
        }

        private void capture_Click(object sender, EventArgs e)
        {
            this.capture.Enabled = false;

            NEKCS.MtpResponse result = camera.SendCommand(NEKCS.NikonMtpOperationCode.InitiateCaptureRecInSdram, [ 0xFFFFFFFF ]);

            camera.DeviceReadyWhile(NikonMtpResponseCode.Device_Busy);

            this.capture.Enabled = true;
        }

        private void liveview_Click(object sender, EventArgs e)
        {
            camera.GetDevicePropValue(NikonMtpDevicePropCode.RemoteLiveViewStatus).TryGetUInt8(out var lvstatus);
            if (lvstatus == 0)
            {
                var res = camera.StartLiveView();
                this.liveview.Text = "Stop Liveview";

                Task.Run(() =>
                {
                    while (getliveview())
                    {
                        Thread.Sleep(30); // small delay to avoid hammering CPU
                    }
                });
            } else {
                camera.EndLiveView();
                this.liveview.Text = "Start Liveview";
            }
        }

        private bool getliveview()
        {
            try
            {
                int headersize = 0;
                MtpResponse response = this.camera.SendCommandAndRead(NikonMtpOperationCode.GetLiveViewImage, []);
                for (var i = 1; i < response.Data.Length; i++)
                {
                    if (response.Data[i] == 0xD8)
                    {
                        if (response.Data[i - 1] == 0xFF)
                        {
                            headersize = i - 1;
                        }
                    }
                }

                var img = response.Data.Skip(headersize).ToArray();

                _syncContext.Post(delegate
                {
                    try
                    {
                        var stream = new MemoryStream(img);
                        var decoder = System.Windows.Media.Imaging.JpegBitmapDecoder.Create(stream, System.Windows.Media.Imaging.BitmapCreateOptions.PreservePixelFormat, System.Windows.Media.Imaging.BitmapCacheOption.OnLoad);
                        var frame = decoder.Frames[0];
                        var encoder = new PngBitmapEncoder();
                        encoder.Frames.Add(frame);
                        var ms = new MemoryStream();
                        encoder.Save(ms);
                        PictureBox.Image = new System.Drawing.Bitmap(ms);
                    }
                    catch {}
                }, null);

                return true;
            }
            catch
            {
                return false;
            }
        }
    }
}
