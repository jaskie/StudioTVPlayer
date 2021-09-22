using System;
using System.Xml.Serialization;

namespace TimecodeDecoderService
{
    [XmlRoot("channels")]
    public class Channels : IDisposable
    {
        [XmlElement("channel")]
        public Channel[] ChannelList { get; set; }

        public void Dispose()
        {
            foreach (Channel channel in ChannelList)
                channel.Dispose();
        }

        public void StartAll()
        {
            foreach (Channel channel in ChannelList)
                channel.Start();
        }


    }
}
