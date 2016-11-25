using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AllNetBridgeCS
{
    public enum BoardTypes
    {
        ArduinoLeonardo,
        ArduinoYun,
        ArduinoMega256,
        ArduinoMiniPro5,
        ArduinoMiniPro3,
        ArduinoEthernet,
        ArduinoDue,
        ArduinoMICRO,
        ArduinoNano,
        ArduinoZERO,
        AruinoAnother1,
        AruinoAnother2,
        AruinoAnother3,
        AruinoAnother4,
        AruinoAnother5,
        AruinoAnother6,
        ParticlePhoton,
        ParticleElectron,
        ParticleRaspberryPi,
        RaspberryPi1AP,
        RaspberryPi1BP,
        RaspberryPi2B,
        RaspberryPi3B,
        RaspberryPiZERO

    }

    public class AllNetBoard
    {
        public int Id { get; set; }

        public string Code { get; set; }

        public string Name { get; set; }
        public BoardTypes BoardType { get; set; }

        public long SequenceId { get; set; }
        public long SequenceIdLast { get; set; }
        public int SequenceErrors { get; set; }

        public DateTime LastSeen { get; set; }

        public string Data { get; set; }
        public string DataLast { get; set; }

        public Dictionary<string,string> DataDictionary { get; set; }
        public Dictionary<string,string> DataDictionaryLast { get; set; }

        public AllNetBoard() { }
        public AllNetBoard(AllNetMessage allNetMessage)
        {
            this.Id = allNetMessage.SenderId;
            this.LastSeen = DateTime.Now;
            this.SequenceId = allNetMessage.Id;
            this.SequenceIdLast = 0;

            this.Data = allNetMessage.Data;
            this.DataLast = "";

            this.DataDictionary = allNetMessage.DataDictionary;
        }

        public void UpdateWithMessage(AllNetMessage allNetMessage)
        {
            if (this.SequenceId + 1 != allNetMessage.Id) this.SequenceErrors++;

            this.SequenceIdLast = this.SequenceId;
            this.SequenceId = allNetMessage.Id;
            this.DataDictionaryLast = this.DataDictionary;
            this.DataDictionaryLast = allNetMessage.DataDictionary;
            this.DataLast = this.Data;
            this.Data = allNetMessage.Data;

        }
    }
}
