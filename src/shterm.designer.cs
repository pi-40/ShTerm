namespace shterm
{
    partial class Form1
    {
        private System.ComponentModel.IContainer components = null;
        private System.Windows.Forms.TextBox txtInput;
        private System.Windows.Forms.TextBox txtTerminal;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            txtInput = new TextBox();
            txtTerminal = new TextBox();
            SuspendLayout();
            // 
            // txtInput
            // 
            txtInput.BackColor = Color.Black;
            txtInput.BorderStyle = BorderStyle.None;
            txtInput.Font = new Font("Consolas", 11F);
            txtInput.ForeColor = Color.White;
            txtInput.Location = new Point(6, 560);
            txtInput.Margin = new Padding(3, 4, 3, 4);
            txtInput.Name = "txtInput";
            txtInput.Size = new Size(749, 22);
            txtInput.TabIndex = 0;
            txtInput.KeyDown += txtInput_KeyDown;
            // 
            // txtTerminal
            // 
            txtTerminal.BackColor = Color.Black;
            txtTerminal.BorderStyle = BorderStyle.None;
            txtTerminal.Font = new Font("Consolas", 11F);
            txtTerminal.ForeColor = Color.White;
            txtTerminal.Location = new Point(6, 7);
            txtTerminal.Margin = new Padding(3, 4, 3, 4);
            txtTerminal.Multiline = true;
            txtTerminal.Name = "txtTerminal";
            txtTerminal.ReadOnly = true;
            txtTerminal.ScrollBars = ScrollBars.Vertical;
            txtTerminal.Size = new Size(1361, 545);
            txtTerminal.TabIndex = 1;
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(8F, 20F);
            AutoScaleMode = AutoScaleMode.Font;
            BackColor = Color.Black;
            ClientSize = new Size(1342, 620);
            Controls.Add(txtTerminal);
            Controls.Add(txtInput);
            FormBorderStyle = FormBorderStyle.FixedSingle;
            Icon = (Icon)resources.GetObject("$this.Icon");
            Margin = new Padding(3, 4, 3, 4);
            MaximizeBox = false;
            Name = "Form1";
            Text = "shterm";
            Load += Form1_Load;
            ResumeLayout(false);
            PerformLayout();
        }
    }
}
