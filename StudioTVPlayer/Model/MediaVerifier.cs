using System;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Threading;
using TVPlayR;

namespace StudioTVPlayer.Model
{
    internal class MediaVerifier: IDisposable
    {
        private struct MediaVerifyData
        {
            public Media Media;
            public int Height;
        }

        private readonly Thread _thread;
        private readonly BlockingCollection<MediaVerifyData> _medias = new BlockingCollection<MediaVerifyData>();
        private MediaVerifier()
        {
            _thread = new Thread(MediaVerifierThread) { Name = "Media verifier thread" };
            _thread.Start();
        }

        public void Dispose()
        {
            while (_medias.TryTake(out var _));
            _medias.Add(new MediaVerifyData { Media = null }); // to exit thread
            _thread.Join();
            _medias.Dispose();
        }

        public static MediaVerifier Current { get; } = new MediaVerifier();

        public void Queue(Media media, int thumbnailHeight)
        {
            _medias.Add(new MediaVerifyData { Media = media, Height = thumbnailHeight });
        }

        private void MediaVerifierThread()
        {
            while (true) 
            {
                var vd = _medias.Take();
                if (vd.Media == null)
                    return;
                try
                {
                    using (var file = new InputFile(vd.Media.FullPath))
                    {
                        vd.Media.Duration = file.VideoDuration;
                        if (vd.Height > 0)
                            vd.Media.Thumbnail = file.GetBitmapSource(vd.Height);
                    }
                }
                catch (Exception e)
                {
                    Debug.WriteLine("Verification of {0} failed with {1}", vd.Media.FullPath, e);
                }                
            }
        }

    }
}
