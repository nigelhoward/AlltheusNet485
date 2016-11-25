using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Diagnostics;

namespace AllNetBridgeCS
{
    public class AllNetSocketServer : IDisposable
    {

        public byte BoardId { get; set; }
        public IPAddress IpAddress { get; set; }
        public int Port { get; set; }
        public Socket SocketListener { get; set; }
        public IPHostEntry IpHost { get; set; }
        public IPEndPoint IpEndPoint { get; set; }
        public bool Connected { get; set; }

        public AsyncCallback m_pfnCallBack { get; set; }
        public IAsyncResult m_result;
        public bool StartMessageJsonReceived { get; set; }
        public bool EndMessageJsonReceived { get; set; }
        public int ReceivedDeviceCount = 0;
        public String DeviceJsonString { get; set; }

        public AllNetMessageBuffer ReceiveBuffer { get; set; }
        public AllNetMessageBuffer SendBuffer { get; set; }
        public AllNetMessageBuffer ConfirmationBuffer { get; set; }

        public List<string> ErrorMessages { get; set; }
        public int ErrorCounter = 0;
        public int SequenceErrorCounter = 0;
        private DateTime thisElapsedTime;
        private DateTime lastElapsedTime;
        public double DevicesPerSecond = 0;
        public double[] BusSpeedSamples = new double[] { 0, 0, 0, 0 };
        public int BusSpeedSampleAverageCounter = 0;

        public int MessageCounter = 0;

        private Stopwatch AllStopwatch { get; set; }

        public AllNetBoards AllNetBoards { get; set; }

        public AllNetSocketServer(IPAddress ipAddress, int port)
        {
            this.IpAddress = ipAddress;
            this.Port = port;
            this.Connected = false;

            ReceiveBuffer = new AllNetMessageBuffer();
        }

        public void Start()
        {
            try
            {
                AllStopwatch = new System.Diagnostics.Stopwatch();
                AllStopwatch.Start();
                //ReceivedDeviceTable = new AllDeviceTable();

                // Create the socket instance
                SocketListener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

                // Create the end point 
                IPEndPoint ipEnd = new IPEndPoint(IpAddress, Port);

                // Connect to the remote host
                SocketListener.Connect(ipEnd);

                if (SocketListener.Connected)
                {
                    this.Connected = true;
                }
            }
            catch (SocketException ex)
            {
                this.Connected = false;
                this.ErrorMessages.Add(ex.Message);
            }
        }

        public void WaitForData()
        {
            try
            {
                if (m_pfnCallBack == null)
                {
                    m_pfnCallBack = new AsyncCallback(OnDataReceived);
                }

                AllNetSocketPacket socketPacket = new AllNetSocketPacket();

                socketPacket.thisSocket = SocketListener;

                // Start listening to the data asynchronously

                m_result = SocketListener.BeginReceive(
                             socketPacket.dataBuffer,
                             0, socketPacket.dataBuffer.Length,
                             SocketFlags.None,
                             m_pfnCallBack,
                             socketPacket
                             );
            }
            catch (SocketException ex)
            {
                this.ErrorMessages.Add(ex.Message);

            }
        }

        public void OnDataReceived(IAsyncResult iAsyncResult)
        {
            AllNetSocketPacket theSockId = (AllNetSocketPacket)iAsyncResult.AsyncState;
            int iRx = theSockId.thisSocket.EndReceive(iAsyncResult);
            char[] chars = new char[iRx + 1];
            System.Text.Decoder d = System.Text.Encoding.UTF8.GetDecoder();
            int charLen = d.GetChars(theSockId.dataBuffer, 0, iRx, chars, 0);
            System.String szData = new System.String(chars);
            char dataChar = (char)chars[0];

            if (dataChar == '[')
            {
                StartMessageJsonReceived = true;
            }

            if (StartMessageJsonReceived)
            {
                if(dataChar!='[' && dataChar != ']' ) DeviceJsonString += dataChar;

                if (dataChar == ']')
                {
                    EndMessageJsonReceived = true;
                    StartMessageJsonReceived = false;
                    //deviceJsonStringList.Add(deviceJsonString);

                    MessageCounter++;

                    var allNetMessage = new AllNetMessage(DeviceJsonString);
                    if (allNetMessage != null)
                    {

                        CalculateBusSpeed();

                        ReceiveBuffer.PutMessage(allNetMessage);

                        if (this.AllNetBoards == null) this.AllNetBoards = new AllNetBoards();

                        this.AllNetBoards.AddAllNetBoard(new AllNetBoard(allNetMessage));
                        AllNetBoards.UpdateAllNetBoardInListWithAllNetMessage(new AllNetBoard(allNetMessage), allNetMessage);
                        DeviceJsonString = "";

                    }
                }
            }
            WaitForData();
        }
        private void CalculateBusSpeed()
        {
            int NoOfDevices = 20;

            if ((ReceivedDeviceCount / NoOfDevices) * NoOfDevices == ReceivedDeviceCount)
            {

                thisElapsedTime = DateTime.Now;

                double elapsedMilliseconds = (thisElapsedTime - lastElapsedTime).TotalMilliseconds;

                lastElapsedTime = thisElapsedTime;


                this.DevicesPerSecond = 1000 / (elapsedMilliseconds / NoOfDevices);

                //AllStopwatch.Start();
            }

            if (ReceivedDeviceCount < 20) DevicesPerSecond = 0;

        }
        public void Dispose()
        {
            if (SocketListener != null)
            {
                SocketListener.Dispose();
                SocketListener = null;
            }
        }
    }
}
