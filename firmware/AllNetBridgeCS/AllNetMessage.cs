using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using System.IO;
using System.Web;

namespace AllNetBridgeCS
{
    public enum MessageTypes

    {
        MESSAGE_BOARDCAST,  // Message sent to all boards on the bus
        MESSAGE_MESSAGE,        // For a specific device Id with user given data - Processed manually 
        MESSAGE_DEVICE,     // For a specific device Id with pre-formatted data - Processed automatically by AllDevice
        MESSAGE_CONFIRMATION    // Confirmation that message received
    };

    public class AllNetMessageShort
    {
        public long Id { get; set; }                   // Sequence number from sending board
        public int SId { get; set; }                   // HEX Id of sending board
        public int RId { get; set; }                   // HEX If of receiving board
        public MessageTypes Typ { get; set; }          // Type of message
        public bool Cnf { get; set; }                  // Requires a response
        public long Rcv { get; set; }                  // When the message was received in millis
        public string Dat { get; set; }                // "{AD0=280}{AD1=268}{AD2=254}{AD3=244}"}}

        public AllNetMessageShort() { }
        public AllNetMessageShort(AllNetMessage allnetMessage)
        {
            this.Id = allnetMessage.Id;
            this.SId = allnetMessage.SenderId;
            this.RId = allnetMessage.ReceiverId;
            this.Typ = allnetMessage.MessageType;
            this.Cnf = allnetMessage.RequireConfirmation;
            this.Rcv = allnetMessage.WhenReceived;
            this.Dat = allnetMessage.Data;
        }

        // Makes a key value string compatible with existing RS485NB Code
        // The RS485NB Class can extract values from key=value strings wrapped in curly brackets so
        // sending a string back to the bridge board makes it easier to decode it into a device.
        // Could have used a JSON decoder / encoder buy concluded that this is overkill for our use
        // especially since RS485NB Class already has the functionality.
         
        public string MakeKeyValueAllNetString(Dictionary<string, string> dataDictionary = null)
        {
            string keyValueAllNetString = "";
            keyValueAllNetString += makeKeyValuePairSyntax("SId", this.SId.ToString());
            keyValueAllNetString += makeKeyValuePairSyntax("RId", this.RId.ToString());
            keyValueAllNetString += makeKeyValuePairSyntax("Cnf", this.Cnf.ToString());
            keyValueAllNetString += makeKeyValuePairSyntax("Typ", ((int)this.Typ).ToString());

            string data = "";
            // Yet another pair of delimeters used up - Can't use curly brackets
            if (dataDictionary!=null)
            {
                foreach (var item in dataDictionary)
                {
                    data += "<" + item.Key + "=" + item.Value + ">";
                }
            }


            keyValueAllNetString += makeKeyValuePairSyntax("Dat", data);
            
            return keyValueAllNetString;

        }
        private static string makeKeyValuePairSyntax(string key, string value)
        {
            return "{" + key + "=" + value + "}";
        }
    }

    public class AllNetMessage
    {


        public long Id { get; set; }                        // Sequence number from sending board
        public int SenderId { get; set; }                   // HEX Id of sending board
        public int ReceiverId { get; set; }                 // HEX If of receiving board
        public MessageTypes MessageType { get; set; }       // Type of message
        public bool RequireConfirmation { get; set; }       // Requires a response
        public long WhenReceived { get; set; }              // When the message was received in millis
        public string Data { get; set; }                    // "{AD0=280}{AD1=268}{AD2=254}{AD3=244}"}}
        
        public Dictionary<string, string> DataDictionary;



        public AllNetMessage() { }
        public AllNetMessage(string jsonData)
        {
            var serializer = new System.Web.Script.Serialization.JavaScriptSerializer();

            var allMessageAlias = serializer.Deserialize<AllNetMessageShort>(jsonData);


            this.Data = allMessageAlias.Dat.Replace("\0","");

            this.Data = this.Data.Replace("}{", ",");
            this.Data = this.Data.Replace("{", "");
            this.Data = this.Data.Replace("}", "");

            var splitKeyPairs = this.Data.Split(',');

            if(splitKeyPairs.Length>0)
            {
                foreach (var splitKeyPair in splitKeyPairs)
                {
                    var splitKeyPairValue = splitKeyPair.Split('=');
                    if(splitKeyPairValue.Length>1)
                    {
                        if (this.DataDictionary == null) this.DataDictionary = new Dictionary<string, string>();
                        this.DataDictionary.Add(splitKeyPairValue[0], splitKeyPairValue[1]);
                    }
                }

            }


            this.Id = allMessageAlias.Id;
            this.ReceiverId = allMessageAlias.RId;
            this.MessageType = allMessageAlias.Typ;
            this.RequireConfirmation = allMessageAlias.Cnf;
            this.SenderId = allMessageAlias.SId;
            this.WhenReceived = allMessageAlias.Rcv;
                

        }
    }
}
