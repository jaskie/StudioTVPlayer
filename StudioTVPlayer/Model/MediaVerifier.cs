using System;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;
using TVPlayR;

namespace StudioTVPlayer.Model
{
    internal class MediaVerifier : IDisposable
    {
        private struct MediaVerifyData
        {
            public MediaFile Media;
            public int Height;
            public int Width;
            public CancellationToken CancellationToken;
            public DateTime FirstVerification;
        }

        private bool _disposed;
        private readonly Task _verificationTask;
        private readonly BlockingCollection<MediaVerifyData> _mediaQueue = new BlockingCollection<MediaVerifyData>();
        private readonly CancellationTokenSource _cancellationTokenSource = new CancellationTokenSource();
        private const int DefaultThumbnailHeight = 126;
        private const int DefaultThumbnailWidth = 224;

        private MediaVerifier()
        {
            _verificationTask = Task.Factory.StartNew(MediaVerifierTask, _cancellationTokenSource.Token, TaskCreationOptions.LongRunning, TaskScheduler.Default);
        }

        public void Dispose()
        {
            if (_disposed)
                return;
            _disposed = true;
            _cancellationTokenSource.Cancel();
            try
            {
                _verificationTask.Wait(_cancellationTokenSource.Token);
            }
            catch (OperationCanceledException)
            { }
            _mediaQueue.Dispose();
        }

        public static MediaVerifier Current { get; } = new MediaVerifier();

        public void Queue(MediaFile media, CancellationToken cancellationToken)
        {
            if (_cancellationTokenSource.IsCancellationRequested)
                return;
            if (_mediaQueue.Any(vd => vd.Media == media && vd.Width == DefaultThumbnailWidth && vd.Height == DefaultThumbnailHeight))
                return;
            _mediaQueue.Add(new MediaVerifyData { Media = media, Width = DefaultThumbnailWidth, Height = DefaultThumbnailHeight, CancellationToken = cancellationToken });
        }

        public void Verify(MediaFile media, int thumbnailWidth, int thumbnailHeight)
        {
            try
            {
                using (var file = new TVPlayR.FileInfo(media.FullPath))
                {
                    media.StartTime = file.VideoStart;
                    media.Duration = file.VideoDuration;
                    media.Width = file.Width;
                    media.Height = file.Height;
                    switch (file.FieldOrder)
                    {
                        case FieldOrder.TopFieldFirst:
                            media.ScanType = ScanType.TopFieldFirst;
                            break;
                        case FieldOrder.BottomFieldFirst:
                            media.ScanType = ScanType.BottomFieldFirst;
                            break;
                    }
                    var frameRate = file.FrameRate;
                    media.FrameRate = $"{frameRate.Numerator}/{frameRate.Denominator}";
                    media.AudioChannelCount = file.AudioChannelCount;
                    media.HaveAlphaChannel = file.HaveAlphaChannel;
                    if (thumbnailHeight > 0)
                    {
                        var thumb = file.GetBitmapSource(file.VideoStart, thumbnailWidth, thumbnailHeight) ?? new BitmapImage();
                        thumb.Freeze();
                        media.Thumbnail = thumb;
                    }
                }
                media.IsValid = media.Duration > TimeSpan.Zero;
            }
            catch 
            {
                media.IsValid = false;
                throw;
            }
            media.IsVerified = true;
        }

        public void Verify(MediaFile media)
        {
            Verify(media, DefaultThumbnailWidth, DefaultThumbnailHeight);
        }

        private void MediaVerifierTask()
        {
            while (!_cancellationTokenSource.IsCancellationRequested)
            {
                try
                {
                    MediaVerifyData vd = _mediaQueue.Take(_cancellationTokenSource.Token);

                    if (vd.CancellationToken.IsCancellationRequested)
                        continue;
                    if (!File.Exists(vd.Media.FullPath))
                        continue;
                    if (vd.FirstVerification == default)
                        vd.FirstVerification = DateTime.Now;
                    try
                    {
                        Verify(vd.Media, vd.Width, vd.Height);
                    }
                    catch (Exception e)
                    {
                        if (DateTime.Now > vd.FirstVerification + TimeSpan.FromSeconds(30))
                            Debug.WriteLine("Verification of {0} unsuccessfull in 30 seconds. Error: {1}", vd.Media.FullPath, e);
                        else
                        {
                            Task.Run(async () =>
                            {
                                await Task.Delay(TimeSpan.FromSeconds(5), vd.CancellationToken);
                                _mediaQueue.Add(vd);
                            }, _cancellationTokenSource.Token);
                        }
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
