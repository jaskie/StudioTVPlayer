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
            int deviceIndex = 0;
            var videoFormat = VideoFormat.Formats.FirstOrDefault(vf => vf.Name == "1080i50");
            using (Player player = new Player("Channel 1", videoFormat, PixelFormat.yuv422, 2))
            {
                using (DecklinkOutput output = DecklinkIterator.CreateOutput(DecklinkIterator.Devices[deviceIndex], DecklinkKeyerType.Default, TimecodeOutputSource.Timecode))
                {
                    output.Initialize(videoFormat, PixelFormat.yuv422, 2, 48000);
                    player.SetFrameClockSource(output);
                    player.AddOutputSink(output);
                    var file1 = new FileInput(@"d:\temp\test1.mov");
                    var file2 = new FileInput(@"d:\temp\test (1).mov");
                    player.Load(file1);
                    file1.Play();
                    player.PlayNext(file2);
                    file2.Loaded += File2_Loaded;
                    Console.ReadKey();
                }
            }
        }

        private static void File2_Loaded(object sender, EventArgs e)
        {
            var input = sender as FileInput;
            Debug.Write($"Started: {input.FileName}");
        }

        private static void Player_AudioVolume(object sender, AudioVolumeEventArgs e)
        {
            Debug.Write(e.AudioVolume.Length);
        }
    }
}
