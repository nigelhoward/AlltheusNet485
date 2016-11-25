namespace AllNetMonitor
{
    partial class FormAllNetMonitor
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
            this.components = new System.ComponentModel.Container();
            this.groupBoxConnect = new System.Windows.Forms.GroupBox();
            this.buttonConnectToBridge = new System.Windows.Forms.Button();
            this.buttonCloseConnectionToBridge = new System.Windows.Forms.Button();
            this.textBoxBridgeIpAddress = new System.Windows.Forms.TextBox();
            this.textBoxBridgeIpPort = new System.Windows.Forms.TextBox();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.labelMessageReceiveBufferCount = new System.Windows.Forms.Label();
            this.labelDevicesPerSecond = new System.Windows.Forms.Label();
            this.buttonSendTestData = new System.Windows.Forms.Button();
            this.textBoxSendTestData = new System.Windows.Forms.TextBox();
            this.dataGridViewReceiveBufferMessageList = new System.Windows.Forms.DataGridView();
            this.timer2 = new System.Windows.Forms.Timer(this.components);
            this.checkBoxReceiveBufferGridViewPaused = new System.Windows.Forms.CheckBox();
            this.dataGridViewAllNetBoards = new System.Windows.Forms.DataGridView();
            this.checkBoxSendInputToOutput = new System.Windows.Forms.CheckBox();
            this.allNetMessageBufferBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this.groupBoxConnect.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewReceiveBufferMessageList)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewAllNetBoards)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.allNetMessageBufferBindingSource)).BeginInit();
            this.SuspendLayout();
            // 
            // groupBoxConnect
            // 
            this.groupBoxConnect.Controls.Add(this.buttonConnectToBridge);
            this.groupBoxConnect.Controls.Add(this.buttonCloseConnectionToBridge);
            this.groupBoxConnect.Controls.Add(this.textBoxBridgeIpAddress);
            this.groupBoxConnect.Controls.Add(this.textBoxBridgeIpPort);
            this.groupBoxConnect.Location = new System.Drawing.Point(12, 12);
            this.groupBoxConnect.Name = "groupBoxConnect";
            this.groupBoxConnect.Size = new System.Drawing.Size(303, 59);
            this.groupBoxConnect.TabIndex = 86;
            this.groupBoxConnect.TabStop = false;
            this.groupBoxConnect.Text = "Connect";
            // 
            // buttonConnectToBridge
            // 
            this.buttonConnectToBridge.Location = new System.Drawing.Point(7, 18);
            this.buttonConnectToBridge.Name = "buttonConnectToBridge";
            this.buttonConnectToBridge.Size = new System.Drawing.Size(64, 23);
            this.buttonConnectToBridge.TabIndex = 7;
            this.buttonConnectToBridge.Text = "Connect";
            this.buttonConnectToBridge.UseVisualStyleBackColor = true;
            this.buttonConnectToBridge.Click += new System.EventHandler(this.buttonConnectToBridge_Click);
            // 
            // buttonCloseConnectionToBridge
            // 
            this.buttonCloseConnectionToBridge.Location = new System.Drawing.Point(230, 18);
            this.buttonCloseConnectionToBridge.Name = "buttonCloseConnectionToBridge";
            this.buttonCloseConnectionToBridge.Size = new System.Drawing.Size(48, 23);
            this.buttonCloseConnectionToBridge.TabIndex = 6;
            this.buttonCloseConnectionToBridge.Text = "Close";
            this.buttonCloseConnectionToBridge.UseVisualStyleBackColor = true;
            this.buttonCloseConnectionToBridge.Click += new System.EventHandler(this.buttonCloseConnectionToBridge_Click);
            // 
            // textBoxBridgeIpAddress
            // 
            this.textBoxBridgeIpAddress.Location = new System.Drawing.Point(77, 19);
            this.textBoxBridgeIpAddress.Name = "textBoxBridgeIpAddress";
            this.textBoxBridgeIpAddress.Size = new System.Drawing.Size(100, 20);
            this.textBoxBridgeIpAddress.TabIndex = 1;
            this.textBoxBridgeIpAddress.Text = "192.168.1.151";
            // 
            // textBoxBridgeIpPort
            // 
            this.textBoxBridgeIpPort.Location = new System.Drawing.Point(187, 19);
            this.textBoxBridgeIpPort.Name = "textBoxBridgeIpPort";
            this.textBoxBridgeIpPort.Size = new System.Drawing.Size(37, 20);
            this.textBoxBridgeIpPort.TabIndex = 2;
            this.textBoxBridgeIpPort.Text = "23";
            // 
            // timer1
            // 
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // labelMessageReceiveBufferCount
            // 
            this.labelMessageReceiveBufferCount.AutoSize = true;
            this.labelMessageReceiveBufferCount.Location = new System.Drawing.Point(377, 40);
            this.labelMessageReceiveBufferCount.Name = "labelMessageReceiveBufferCount";
            this.labelMessageReceiveBufferCount.Size = new System.Drawing.Size(168, 13);
            this.labelMessageReceiveBufferCount.TabIndex = 88;
            this.labelMessageReceiveBufferCount.Text = "labelMessageReceiveBufferCount";
            // 
            // labelDevicesPerSecond
            // 
            this.labelDevicesPerSecond.AutoSize = true;
            this.labelDevicesPerSecond.Location = new System.Drawing.Point(573, 40);
            this.labelDevicesPerSecond.Name = "labelDevicesPerSecond";
            this.labelDevicesPerSecond.Size = new System.Drawing.Size(35, 13);
            this.labelDevicesPerSecond.TabIndex = 89;
            this.labelDevicesPerSecond.Text = "label1";
            // 
            // buttonSendTestData
            // 
            this.buttonSendTestData.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonSendTestData.Location = new System.Drawing.Point(25, 635);
            this.buttonSendTestData.Name = "buttonSendTestData";
            this.buttonSendTestData.Size = new System.Drawing.Size(91, 23);
            this.buttonSendTestData.TabIndex = 90;
            this.buttonSendTestData.Text = "Send this ->";
            this.buttonSendTestData.UseVisualStyleBackColor = true;
            this.buttonSendTestData.Click += new System.EventHandler(this.buttonSendTestData_Click);
            // 
            // textBoxSendTestData
            // 
            this.textBoxSendTestData.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.textBoxSendTestData.Location = new System.Drawing.Point(122, 638);
            this.textBoxSendTestData.Name = "textBoxSendTestData";
            this.textBoxSendTestData.Size = new System.Drawing.Size(385, 20);
            this.textBoxSendTestData.TabIndex = 91;
            this.textBoxSendTestData.Text = "Hello";
            // 
            // dataGridViewReceiveBufferMessageList
            // 
            this.dataGridViewReceiveBufferMessageList.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridViewReceiveBufferMessageList.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewReceiveBufferMessageList.Location = new System.Drawing.Point(25, 229);
            this.dataGridViewReceiveBufferMessageList.Name = "dataGridViewReceiveBufferMessageList";
            this.dataGridViewReceiveBufferMessageList.Size = new System.Drawing.Size(1015, 372);
            this.dataGridViewReceiveBufferMessageList.TabIndex = 92;
            // 
            // timer2
            // 
            this.timer2.Enabled = true;
            this.timer2.Tick += new System.EventHandler(this.timer2_Tick);
            // 
            // checkBoxReceiveBufferGridViewPaused
            // 
            this.checkBoxReceiveBufferGridViewPaused.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.checkBoxReceiveBufferGridViewPaused.AutoSize = true;
            this.checkBoxReceiveBufferGridViewPaused.Checked = true;
            this.checkBoxReceiveBufferGridViewPaused.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxReceiveBufferGridViewPaused.Location = new System.Drawing.Point(634, 641);
            this.checkBoxReceiveBufferGridViewPaused.Name = "checkBoxReceiveBufferGridViewPaused";
            this.checkBoxReceiveBufferGridViewPaused.Size = new System.Drawing.Size(184, 17);
            this.checkBoxReceiveBufferGridViewPaused.TabIndex = 93;
            this.checkBoxReceiveBufferGridViewPaused.Text = "Paused Reveive Buffer Grid View";
            this.checkBoxReceiveBufferGridViewPaused.UseVisualStyleBackColor = true;
            // 
            // dataGridViewAllNetBoards
            // 
            this.dataGridViewAllNetBoards.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridViewAllNetBoards.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewAllNetBoards.Location = new System.Drawing.Point(25, 77);
            this.dataGridViewAllNetBoards.Name = "dataGridViewAllNetBoards";
            this.dataGridViewAllNetBoards.Size = new System.Drawing.Size(1015, 146);
            this.dataGridViewAllNetBoards.TabIndex = 8;
            // 
            // checkBoxSendInputToOutput
            // 
            this.checkBoxSendInputToOutput.AutoSize = true;
            this.checkBoxSendInputToOutput.Location = new System.Drawing.Point(871, 641);
            this.checkBoxSendInputToOutput.Name = "checkBoxSendInputToOutput";
            this.checkBoxSendInputToOutput.Size = new System.Drawing.Size(129, 17);
            this.checkBoxSendInputToOutput.TabIndex = 94;
            this.checkBoxSendInputToOutput.Text = "Send Input To Output";
            this.checkBoxSendInputToOutput.UseVisualStyleBackColor = true;
            // 
            // allNetMessageBufferBindingSource
            // 
            this.allNetMessageBufferBindingSource.DataSource = typeof(AllNetBridgeCS.AllNetMessageBuffer);
            // 
            // FormAllNetMonitor
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1075, 681);
            this.Controls.Add(this.checkBoxSendInputToOutput);
            this.Controls.Add(this.dataGridViewAllNetBoards);
            this.Controls.Add(this.checkBoxReceiveBufferGridViewPaused);
            this.Controls.Add(this.dataGridViewReceiveBufferMessageList);
            this.Controls.Add(this.textBoxSendTestData);
            this.Controls.Add(this.buttonSendTestData);
            this.Controls.Add(this.labelDevicesPerSecond);
            this.Controls.Add(this.labelMessageReceiveBufferCount);
            this.Controls.Add(this.groupBoxConnect);
            this.Name = "FormAllNetMonitor";
            this.Text = "AllNetMonitor V1.4B";
            this.Load += new System.EventHandler(this.FormAllNetMonitor_Load);
            this.groupBoxConnect.ResumeLayout(false);
            this.groupBoxConnect.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewReceiveBufferMessageList)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewAllNetBoards)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.allNetMessageBufferBindingSource)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBoxConnect;
        private System.Windows.Forms.Button buttonConnectToBridge;
        private System.Windows.Forms.Button buttonCloseConnectionToBridge;
        private System.Windows.Forms.TextBox textBoxBridgeIpAddress;
        private System.Windows.Forms.TextBox textBoxBridgeIpPort;
        private System.Windows.Forms.Timer timer1;
        private System.Windows.Forms.Label labelMessageReceiveBufferCount;
        private System.Windows.Forms.Label labelDevicesPerSecond;
        private System.Windows.Forms.Button buttonSendTestData;
        private System.Windows.Forms.TextBox textBoxSendTestData;
        private System.Windows.Forms.BindingSource allNetMessageBufferBindingSource;
        private System.Windows.Forms.DataGridView dataGridViewReceiveBufferMessageList;
        private System.Windows.Forms.Timer timer2;
        private System.Windows.Forms.CheckBox checkBoxReceiveBufferGridViewPaused;
        private System.Windows.Forms.DataGridView dataGridViewAllNetBoards;
        private System.Windows.Forms.CheckBox checkBoxSendInputToOutput;
    }
}

