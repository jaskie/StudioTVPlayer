using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using TVPlayR;

namespace TestCSharp
{
    class Program
    {
        static void Main(string[] args)
        {
            Channel channel = new Channel("Channel 1", VideoFormat.EnumVideoFormats().FirstOrDefault(vf => vf.Name == "1080i50"), PixelFormat.yuv422, 2);
            channel.AddOutput(DecklinkIterator.CreateOutput(DecklinkIterator.Devices[0]), true);
            channel.AudioVolume += Channel_AudioVolume;
            Console.ReadKey();
        }

        private static void Channel_AudioVolume(object sender, AudioVolumeEventArgs e)
        {
            Debug.Write(e.AudioVolume.Length);
        }
    }
}
