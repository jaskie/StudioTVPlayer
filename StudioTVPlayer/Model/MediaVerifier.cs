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
            _cancellationTokenSource.Dispose();
        }

        public static MediaVerifier Current { get; } = new MediaVerifier();

        public void Queue(MediaFile media)
        {
            if (_cancellationTokenSource.IsCancellationRequested)
                return;
            if (_mediaQueue.Any(vd => vd.Media == media && vd.Width == DefaultThumbnailWidth && vd.Height == DefaultThumbnailHeight))
                return;
            _mediaQueue.Add(new MediaVerifyData { Media = media, Width = DefaultThumbnailWidth, Height = DefaultThumbnailHeight });
        }

        /// <summary>
        /// Method sets IsVerified and IsValid properties for <see cref="MediaFile"/>. Returns true is verification was successful, even when file is not valid.
        /// </summary>
        public bool Verify(MediaFile media, int thumbnailWidth = DefaultThumbnailWidth, int thumbnailHeight = DefaultThumbnailHeight)
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
                media.IsVerified = true;
                return true;
            }
            catch 
            {
                media.IsValid = false;
                media.IsVerified = true;
                return false;
            }
        }

        private void MediaVerifierTask()
        {
            var token = _cancellationTokenSource.Token;
            while (true)
            {
                try
                {
                    MediaVerifyData vd = _mediaQueue.Take(token);

                    if (!File.Exists(vd.Media.FullPath))
                        continue;
                    if (vd.FirstVerification == default)
                        vd.FirstVerification = DateTime.Now;
                    if (!Verify(vd.Media, vd.Width, vd.Height))
                    {
                        if (DateTime.Now > vd.FirstVerification + TimeSpan.FromSeconds(30))
                            Debug.WriteLine("Verification of {0} unsuccessfull in 30 seconds.", vd.Media.FullPath);
                        else
                        {
                            Task.Run(async () =>
                            {
                                await Task.Delay(TimeSpan.FromSeconds(5), token)
                                    .ContinueWith(_ => _mediaQueue.Add(vd));
                            });
                        }
                    }
                }
                catch (Exception e) when (e is OperationCanceledException || e is ObjectDisposedException)
                {
                    return;
                }
            }
        }
    }
}
