using System;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using TVPlayR;

namespace StudioTVPlayer.Model
{
    internal class MediaVerifier: IDisposable
    {
        private struct MediaVerifyData
        {
            public Media Media;
            public int Height;
            public CancellationToken CancellationToken;
            public DateTime FirstVerification;
        }

        private readonly Task _verificationTask;
        private readonly BlockingCollection<MediaVerifyData> _medias = new BlockingCollection<MediaVerifyData>();
        private CancellationTokenSource _cancellationTokenSource = new CancellationTokenSource();
        private MediaVerifier()
        {
            _verificationTask = Task.Factory.StartNew(MediaVerifierTask, _cancellationTokenSource.Token, TaskCreationOptions.LongRunning, TaskScheduler.Default);
        }

        public void Dispose()
        {
            _cancellationTokenSource.Cancel();
            try
            {
                _verificationTask.Wait();
            }
            catch (OperationCanceledException) 
            { }
            _medias.Dispose();
        }

        public static MediaVerifier Current { get; } = new MediaVerifier();

        public void Queue(Media media, int thumbnailHeight, CancellationToken cancellationToken)
        {
            _medias.Add(new MediaVerifyData { Media = media, Height = thumbnailHeight, CancellationToken = cancellationToken });
        }

        private void MediaVerifierTask()
        {
            while (!_verificationTask.IsCanceled)
            {
                try
                {
                    var vd = _medias.Take(_cancellationTokenSource.Token);

                    if (vd.CancellationToken.IsCancellationRequested)
                        continue;
                    if (!File.Exists(vd.Media.FullPath))
                        continue;
                    if (vd.FirstVerification == default(DateTime))
                        vd.FirstVerification = DateTime.Now;
                    try
                    {
                        using (var file = new InputFile(vd.Media.FullPath, 0))
                        {
                            vd.Media.Duration = file.VideoDuration;
                            if (vd.Height > 0)
                            {
                                var thumb = file.GetBitmapSource(vd.Height);
                                thumb.Freeze();
                                vd.Media.Thumbnail = thumb;
                            }
                        }
                    }
                    catch (Exception e)
                    {
                        if (DateTime.Now > vd.FirstVerification + TimeSpan.FromMinutes(30))
                            Debug.WriteLine("Verification of {0} unsuccessfull in 30 minutes. Error: {1}", vd.Media.FullPath, e);
                        else
                            Task.Run(async () =>
                            {
                                await Task.Delay(TimeSpan.FromSeconds(5), vd.CancellationToken);
                                _medias.Add(vd);
                            }, _cancellationTokenSource.Token);
                    }
                }
                catch (OperationCanceledException)
                {
                    return;
                }
            }
        }
    }
}
