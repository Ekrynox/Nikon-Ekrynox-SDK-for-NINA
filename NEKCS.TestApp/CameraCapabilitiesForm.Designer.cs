namespace NEKCS.TestApp
{
    partial class CameraCapabilitiesForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
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
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            CameraCapabilities = new RichTextBox();
            SuspendLayout();
            // 
            // CameraCapabilities
            // 
            CameraCapabilities.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            CameraCapabilities.Location = new Point(12, 12);
            CameraCapabilities.Name = "CameraCapabilities";
            CameraCapabilities.ReadOnly = true;
            CameraCapabilities.RightToLeft = RightToLeft.No;
            CameraCapabilities.Size = new Size(776, 426);
            CameraCapabilities.TabIndex = 0;
            CameraCapabilities.Text = "";
            // 
            // CameraCapabilitiesForm
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(800, 450);
            Controls.Add(CameraCapabilities);
            Name = "CameraCapabilitiesForm";
            Text = "Camera Capabilities";
            FormClosing += CameraCapabilitiesForm_FormClosing;
            Load += CameraCapabilitiesForm_Load;
            ResumeLayout(false);
        }

        #endregion

        private RichTextBox CameraCapabilities;
    }
}