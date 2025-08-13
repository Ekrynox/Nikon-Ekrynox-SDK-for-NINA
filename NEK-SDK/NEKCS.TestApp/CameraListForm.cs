namespace NEKCS.TestApp
{
    public partial class CameraListForm : Form
    {
        System.Collections.Generic.Dictionary<string, NEKCS.NikonDeviceInfoDS> cameras;

        public CameraListForm()
        {
            InitializeComponent();
        }

        private void refreshCameraList_Click(object sender, EventArgs e)
        {
            cameras = NEKCS.NikonCamera.listNikonCameras();
            this.cameraList.Items.Clear();
            foreach (var camera in cameras)
            {
                this.cameraList.Items.Add(camera.Value.Manufacture + " " + camera.Value.Model + " " + camera.Value.SerialNumber);
            }
        }

        private void Form1_Shown(object sender, EventArgs e)
        {
            refreshCameraList_Click(sender, e);
        }

        private void cameraList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (this.cameraList.SelectedIndex >= 0 && this.cameraList.SelectedIndex < cameras.Count) { 
                CameraEventListener form = new CameraEventListener(cameras.ToList()[this.cameraList.SelectedIndex].Key);
                form.Show();
            }
        }
    }
}
