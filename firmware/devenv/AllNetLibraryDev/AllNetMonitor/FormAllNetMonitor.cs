using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using AllNetBridgeCS;
using System.Net;
using System.Net.Sockets;

namespace AllNetMonitor
{
    public partial class FormAllNetMonitor : Form
    {
        public AllNetSocketServer allNetSocketServer{ get; set; }

        public FormAllNetMonitor()
        {
            InitializeComponent();
        }

        private void buttonConnectToBridge_Click(object sender, EventArgs e)
        {
            this.allNetSocketServer = new AllNetSocketServer(IPAddress.Parse(textBoxBridgeIpAddress.Text), Convert.ToInt32(textBoxBridgeIpPort.Text));
            this.allNetSocketServer.Start();
            this.allNetSocketServer.WaitForData();
            timer1.Enabled = true;

            byte[] byteData = System.Text.Encoding.ASCII.GetBytes("Hello");
            this.allNetSocketServer.SocketListener.Send(byteData);

        }

        private void buttonCloseConnectionToBridge_Click(object sender, EventArgs e)
        {
            this.allNetSocketServer.Dispose();
        }

        private void FormAllNetMonitor_Load(object sender, EventArgs e)
        {

        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            labelMessageReceiveBufferCount.Text = allNetSocketServer.ReceiveBuffer.Length().ToString();
            labelDevicesPerSecond.Text = " Messages / Second :" + Convert.ToInt16(allNetSocketServer.DevicesPerSecond).ToString() + "   Messages:" + allNetSocketServer.MessageCounter.ToString();

            sendInputToOutput();
        }

        private void updateReceiveBufferDataGridView()
        {
            if (this.allNetSocketServer != null)
            {
                if (!checkBoxReceiveBufferGridViewPaused.Checked)
                {
                    var receiveBufferMessageList = this.allNetSocketServer.ReceiveBuffer.GetMessageList();
                    receiveBufferMessageList = (from rb in receiveBufferMessageList orderby rb.WhenReceived descending select rb).ToList();
                    this.dataGridViewReceiveBufferMessageList.DataSource = receiveBufferMessageList;

                }
                    
            }

        }
        private void sendInputToOutput()
        {
            if (this.allNetSocketServer != null)
            {
                if (checkBoxSendInputToOutput.Checked)
                {
                    var messageFromBuffer = this.allNetSocketServer.ReceiveBuffer.GetMessage();

                    var allNetMessageShort = new AllNetMessageShort(messageFromBuffer);
                    allNetMessageShort.SId = 240;

                    if (allNetMessageShort != null)
                    {
                        var message = allNetMessageShort.MakeKeyValueAllNetString(messageFromBuffer.DataDictionary);
                        message = "[" + message + "]";
                        byte[] byteData = System.Text.Encoding.ASCII.GetBytes(message);
                        this.allNetSocketServer.SocketListener.Send(byteData);
                    }

                }
            }
        }

        private void updateAllnetrBoardsGridView()
        {
            if (this.allNetSocketServer != null)
            {
                if (!checkBoxReceiveBufferGridViewPaused.Checked)
                {
                    dataGridViewAllNetBoards.DataSource = this.allNetSocketServer.AllNetBoards.AllNetBoardList.ToList();
                }
            }
        }



        private void buttonSendTestData_Click(object sender, EventArgs e)
        {
            string dataToSend = textBoxSendTestData.Text + "\n";
            byte[] byteData = System.Text.Encoding.ASCII.GetBytes(dataToSend);
            this.allNetSocketServer.SocketListener.Send(byteData);
        }

        private void timer2_Tick(object sender, EventArgs e)
        {
            updateReceiveBufferDataGridView();
            updateAllnetrBoardsGridView();
        }
    }
}
