using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Concurrent;
using System.Collections.Generic;

namespace AllNetBridgeCS
{
    public class AllNetMessageBuffer
    {
        public ConcurrentQueue<AllNetMessage> AllMessageQueue { get; set; }
        public int MaxQueueLength = 2048;

        public AllNetMessageBuffer()
        {
            AllMessageQueue = new ConcurrentQueue<AllNetMessage>();
        }

        public int Length()
        {
            return AllMessageQueue.Count();
        }

        public bool IsEmpty()
        {
            if (Length() == 0) return false; else return true;
        }

        public AllNetMessage GetMessage()
        {
            AllNetMessage allMessage = new AllNetMessage();
            AllMessageQueue.TryDequeue(out allMessage);
            return allMessage;
        }

        public void PutMessage(AllNetMessage allMessage)
        {
            AllMessageQueue.Enqueue(allMessage);
            if(this.Length() > MaxQueueLength)
            {
                this.GetMessage();
            }
        }

        public List<AllNetMessage> GetMessageList()
        {
            return AllMessageQueue.ToList();
        }

    }
}
