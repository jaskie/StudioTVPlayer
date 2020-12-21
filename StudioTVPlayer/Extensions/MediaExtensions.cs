using System;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using TVPlayR;
using StudioTVPlayer.ViewModel.Main.Piloting.Browser;
using static StudioTVPlayer.Model.Enums;

namespace StudioTVPlayer.Extensions
{
    public static class MediaExtensions
    {        
        public static TimeSpan TryGetDuration(string path, bool getAudioDuration = false)
        {
            //Debug.WriteLine("GetDuration thread ID " + Thread.CurrentThread.ManagedThreadId);
            using (InputFile input = new InputFile(path))
            {               
                if (getAudioDuration)
                    return input.AudioDuration;
                else
                    return input.VideoDuration;
            }
        }

        public static bool IsMediaFile(string fileName)
        {
            //TODO: add checking
            return true;
        }

        public static bool GetFFMeta(this BrowserTabItemViewModel browserVM, FFMeta ffmeta = default(FFMeta),
            bool getAudioDuration = false, int height = 200)
        {
            if (!File.Exists(browserVM.Media.Path))
                return false;

            bool result = true;
            ImageSource tempThumb = null;

            //Debug.WriteLine("FFMeta: " + Thread.CurrentThread.ManagedThreadId);

            try
            {
                using (InputFile input = new InputFile(browserVM.Media.Path))
                {
                    switch (ffmeta)
                    {
                        case FFMeta.Duration:
                            {
                                browserVM.Media.Duration =
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
                                browserVM.Media.Duration =
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
                    browserVM.Thumbnail = tempThumb;
                }));
                return true;
            }
                
            return false;
        }

        public static void GetResourceThumbnail(this BrowserTabItemViewModel browserVM, ThumbnailType thumbnailType = default(ThumbnailType))
        {           
            browserVM.Thumbnail = thumbnailType == ThumbnailType.NoPreview ? 
                new BitmapImage(new Uri(@"pack://application:,,,/StudioTVPlayer;component/Resources/NoPreviewThumbnail.png")): 
                new BitmapImage(new Uri(@"pack://application:,,,/StudioTVPlayer;component/Resources/LoadingThumbnail.png")); 
        }
    }
}
