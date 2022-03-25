using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class EncoderSettings
    {
        public static string[] VideoCodecs => TVPlayR.FFOutput.VideoCodecs;
        public static string[] AudioCodecs => TVPlayR.FFOutput.AudioCodecs;

        public string VideoCodec { get; set; }
        public string AudioCodec { get; set; }
        public int VideoBitrate { get; set; }
        public int AudioBitrate { get; set; }
        public string VideoFilter { get; set; }
        public string OutputMetadata { get; set; }
        public string AudioMetadata { get; set; }
        public string VideoMetadata { get; set; }
        public string Options { get; set; }
        public int VideoStreamId { get; set; }
        public int AudioStreamId { get; set; }
    }
}
