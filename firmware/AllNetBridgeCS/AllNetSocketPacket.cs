using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AllNetBridgeCS
{
    public class AllNetSocketPacket : IDisposable
    {
        public void Dispose()
        {
            if (thisSocket != null)
            {
                thisSocket.Dispose();
                thisSocket = null;
            }
        }
        public System.Net.Sockets.Socket thisSocket;
        public byte[] dataBuffer = new byte[1];
    }
}
