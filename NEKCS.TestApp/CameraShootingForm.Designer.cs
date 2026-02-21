namespace NEKCS.TestApp
{
    partial class CameraShootingForm
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
            PictureBox = new PictureBox();
            capture = new Button();
            liveview = new Button();
            ((System.ComponentModel.ISupportInitialize)PictureBox).BeginInit();
            SuspendLayout();
            // 
            // PictureBox
            // 
            PictureBox.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            PictureBox.Location = new Point(12, 12);
            PictureBox.Name = "PictureBox";
            PictureBox.Size = new Size(528, 426);
            PictureBox.SizeMode = PictureBoxSizeMode.StretchImage;
            PictureBox.TabIndex = 0;
            PictureBox.TabStop = false;
            // 
            // capture
            // 
            capture.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            capture.Location = new Point(545, 12);
            capture.Name = "capture";
            capture.Size = new Size(131, 23);
            capture.TabIndex = 1;
            capture.Text = "Capture image";
            capture.UseVisualStyleBackColor = true;
            capture.Click += capture_Click;
            // 
            // liveview
            // 
            liveview.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            liveview.Location = new Point(546, 41);
            liveview.Name = "liveview";
            liveview.Size = new Size(131, 23);
            liveview.TabIndex = 2;
            liveview.Text = "Start Liveview";
            liveview.UseVisualStyleBackColor = true;
            liveview.Click += liveview_Click;
            // 
            // CameraShootingForm
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(688, 450);
            Controls.Add(liveview);
            Controls.Add(capture);
            Controls.Add(PictureBox);
            Name = "CameraShootingForm";
            Text = "CameraShootingForm";
            ((System.ComponentModel.ISupportInitialize)PictureBox).EndInit();
            ResumeLayout(false);
        }

        #endregion

        private PictureBox PictureBox;
        private Button capture;
        private Button liveview;
    }
}