namespace NEKCS.TestApp
{
    partial class CameraEventListener
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
            EventList = new RichTextBox();
            SuspendLayout();
            // 
            // EventList
            // 
            EventList.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            EventList.Location = new Point(12, 12);
            EventList.Name = "EventList";
            EventList.ReadOnly = true;
            EventList.RightToLeft = RightToLeft.No;
            EventList.Size = new Size(776, 426);
            EventList.TabIndex = 0;
            EventList.Text = "";
            // 
            // CameraEventListener
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(800, 450);
            Controls.Add(EventList);
            Name = "CameraEventListener";
            Text = "Event Listener";
            FormClosing += CameraEventListener_FormClosing;
            Load += CameraEventListener_Load;
            ResumeLayout(false);
        }

        #endregion

        private RichTextBox EventList;
    }
}