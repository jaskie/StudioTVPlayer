using System;
using System.IO;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using TVPlayR;
using StudioTVPlayer.Model;

namespace StudioTVPlayer.Extensions
{
    public static class MediaExtensions
    {        
        public static bool GetFFMeta(this Media media, FFMeta ffmeta = default,
            bool getAudioDuration = false, int height = 200)
        {
            if (!File.Exists(media.DirectoryName))
                return false;

            bool result = true;
            ImageSource tempThumb = null;

            //Debug.WriteLine("FFMeta: " + Thread.CurrentThread.ManagedThreadId);

            try
            {
                using (InputFile input = new InputFile(media.DirectoryName, 2))
                {
                    switch (ffmeta)
                    {
                        case FFMeta.Duration:
                            {
                                media.Duration =
                                    getAudioDuration ? input.AudioDuration : input.VideoDuration;
                                break;
                            }

                        case FFMeta.Thumbnail:
                            {
                                tempThumb = input.GetBitmapSource(height);                               
                                break;
                            }

                        default:
                            {
                                media.Duration =
                                    getAudioDuration ? input.AudioDuration : input.VideoDuration;
                                //Debug.WriteLine("FFMeta: " + Application.Current.Dispatcher.Thread.ManagedThreadId);                               
                                tempThumb = input.GetBitmapSource(height);
                                break;
                            }
                    }
                }
            }
            catch
            {
                result = false;
            }


            if (result && tempThumb != null)
            {
                tempThumb.Freeze();
                Application.Current?.Dispatcher?.BeginInvoke((Action)(() =>
                {
                    //Debug.WriteLine("FFMeta Invoke: " + Thread.CurrentThread.ManagedThreadId);
                    media.Thumbnail = tempThumb;
                }));
                return true;
            }
                
            return false;
        }

        public static void GetResourceThumbnail(this Media media, ThumbnailType thumbnailType = default(ThumbnailType))
        {           
            media.Thumbnail = thumbnailType == ThumbnailType.NoPreview ? 
                new BitmapImage(new Uri(@"pack://application:,,,/StudioTVPlayer;component/Resources/NoPreviewThumbnail.png")): 
                new BitmapImage(new Uri(@"pack://application:,,,/StudioTVPlayer;component/Resources/LoadingThumbnail.png")); 
        }
    }
}
