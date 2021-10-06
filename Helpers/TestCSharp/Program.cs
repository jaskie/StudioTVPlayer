using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using TVPlayR;

namespace TestCSharp
{
    class Program
    {
        static void Main(string[] args)
        {
            int deviceIndex = 1;
            var videoFormat = VideoFormat.Formats.FirstOrDefault(vf => vf.Name == "1080i50");
            using (Channel channel = new Channel("Channel 1", videoFormat, PixelFormat.yuv422, 2))
            {
                using (DecklinkOutput output = DecklinkIterator.CreateOutput(DecklinkIterator.Devices[deviceIndex], false))
                using (DecklinkInput input = DecklinkIterator.CreateInput(DecklinkIterator.Devices[deviceIndex], videoFormat, 2, DecklinkTimecodeSource.VITC, true))
                {

                    channel.AddOutput(output, true);
                    var file = new FileInput(@"d:\temp\test5.mov");
                    channel.Load(file);
                    file.Play();
                    Console.ReadKey();
                    //channel.AudioVolume += Channel_AudioVolume;
                }
            }
        }

        private static void Channel_AudioVolume(object sender, AudioVolumeEventArgs e)
        {
            Debug.Write(e.AudioVolume.Length);
        }
    }
}
