namespace NEKCS.TestApp
{
    partial class CameraListForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            Button refreshCameraList;
            cameraList = new ListBox();
            refreshCameraList = new Button();
            SuspendLayout();
            // 
            // refreshCameraList
            // 
            refreshCameraList.Location = new Point(12, 12);
            refreshCameraList.Name = "refreshCameraList";
            refreshCameraList.Size = new Size(75, 23);
            refreshCameraList.TabIndex = 0;
            refreshCameraList.Text = "Refresh List";
            refreshCameraList.UseVisualStyleBackColor = true;
            refreshCameraList.Click += refreshCameraList_Click;
            // 
            // cameraList
            // 
            cameraList.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            cameraList.FormattingEnabled = true;
            cameraList.Location = new Point(12, 41);
            cameraList.Name = "cameraList";
            cameraList.Size = new Size(776, 394);
            cameraList.TabIndex = 1;
            cameraList.SelectedIndexChanged += cameraList_SelectedIndexChanged;
            // 
            // CameraListForm
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(800, 450);
            Controls.Add(cameraList);
            Controls.Add(refreshCameraList);
            Name = "CameraListForm";
            Text = "Camera List";
            Shown += Form1_Shown;
            ResumeLayout(false);
        }

        #endregion

        private ListBox cameraList;
    }
}
