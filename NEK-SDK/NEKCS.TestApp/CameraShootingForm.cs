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

            NEKCS.MtpParams param = new NEKCS.MtpParams();
            param.addUint32(0);
            NEKCS.MtpResponse result = camera.SendCommandAndRead(NEKCS.NikonMtpOperationCode.DeleteImagesInSdram, param);

            byte lvstatus = 0;
            camera.GetDevicePropValue(NikonMtpDevicePropCode.RemoteLiveViewStatus).TryGetUInt8(ref lvstatus);
            if (lvstatus == 1) camera.EndLiveView();

        }


        private void cameraEvent(NEKCS.NikonCamera cam, NEKCS.MtpEvent e)
        {
            if (e.eventCode == NEKCS.NikonMtpEventCode.ObjectAddedInSdram)
            {
                if (e.eventParams.Length > 0 && e.eventParams[0] != 0)
                {
                    _sdramHandle = e.eventParams[0];
                }
                else
                {
                    _sdramHandle = 0xFFFF0001;
                }

                camera.GetObjectInfo(_sdramHandle);

                NEKCS.MtpParams param = new NEKCS.MtpParams();
                param.addUint32(_sdramHandle);
                NEKCS.MtpResponse result = camera.SendCommandAndRead(NEKCS.NikonMtpOperationCode.GetObject, param);
                if (result.responseCode == NEKCS.NikonMtpResponseCode.OK)
                {
                    _syncContext.Post(delegate
                    {
                        var stream = new MemoryStream(result.data.ToArray());
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

            NEKCS.MtpParams param = new NEKCS.MtpParams();
            param.addUint32(0xFFFFFFFF);
            NEKCS.MtpResponse result = camera.SendCommand(NEKCS.NikonMtpOperationCode.InitiateCaptureRecInSdram, param);

            camera.DeviceReady(NikonMtpResponseCode.Device_Busy);

            this.capture.Enabled = true;
        }

        private void liveview_Click(object sender, EventArgs e)
        {
            byte lvstatus = 0;
            camera.GetDevicePropValue(NikonMtpDevicePropCode.RemoteLiveViewStatus).TryGetUInt8(ref lvstatus);
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
                var parameters = new MtpParams();
                MtpResponse response = this.camera.SendCommandAndRead(NikonMtpOperationCode.GetLiveViewImage, parameters);
                for (var i = 1; i < response.data.Length; i++)
                {
                    if (response.data[i] == 0xD8)
                    {
                        if (response.data[i - 1] == 0xFF)
                        {
                            headersize = i - 1;
                        }
                    }
                }

                var img = response.data.Skip(headersize).ToArray();

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
