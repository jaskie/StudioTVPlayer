using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using StudioTVPlayer.Extensions;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Args;
using StudioTVPlayer.Model.Interfaces;

namespace StudioTVPlayer.Services
{
    public class MediaWatcherService : IMediaWatcherService
    {
        private FileSystemWatcher _fs = null;
        public event EventHandler<MediaEventArgs> NotifyOnMediaChanged;
        public event EventHandler<MediaEventArgs> NotifyOnMediaAdd;
        public event EventHandler<MediaEventArgs> NotifyOnMediaVerified;
        public event EventHandler<MediaEventArgs> NotifyOnMediaDelete;
        public event EventHandler<MediaEventArgs> NotifyOnMediaRenamed;
        private readonly List<Media> _mediaListToVerify;
        private readonly List<Media> _mediaListToTrack;

        private bool IsVerificating;
        private bool IsTracking;       
      
        public MediaWatcherService(string path)
        {
            _mediaListToTrack = new List<Media>();
            _mediaListToVerify = new List<Media>();

            _fs = new FileSystemWatcher(path);
            _fs.NotifyFilter = NotifyFilters.FileName | NotifyFilters.Size | NotifyFilters.LastWrite | NotifyFilters.CreationTime;
            _fs.InternalBufferSize = 64000; //zalecany max
            _fs.Error += _fs_Error;
            _fs.Created += MediaCreated;
            _fs.Deleted += MediaDeleted;
            _fs.Renamed += MediaRenamed;
            _fs.Changed += MediaChanged;
            _fs.EnableRaisingEvents = true;
        }           
        
        public string GetPath()
        {
            return _fs.Path;
        }

        private void MediaCreated(object sender, FileSystemEventArgs e)
        {
            Debug.WriteLine("Media Created Notified");
            if (!MediaExtensions.IsMediaFile(e.FullPath))
                return;

            Media m = new Media
            {
                Path = e.FullPath,
                Name = Path.GetFileName(e.FullPath),
                CreationDate = File.GetCreationTime(e.FullPath),
            };

            NotifyOnMediaAdd?.Invoke(this, new MediaEventArgs(m));
        }

        private void MediaChanged(object sender, FileSystemEventArgs e)
        {
            Debug.WriteLine("Media Changed Notified");
            if (e.ChangeType != WatcherChangeTypes.Changed)
                return;

            NotifyOnMediaChanged?.Invoke(this, new MediaEventArgs(new Media { Path = e.FullPath }));            
        }

        private void MediaRenamed(object sender, RenamedEventArgs e)
        {
            Debug.WriteLine("Media Renamed Notified");
            NotifyOnMediaRenamed?.Invoke(this, new MediaEventArgs(new Media { Path = e.FullPath }, e.OldFullPath));            
        }

        private void MediaDeleted(object sender, FileSystemEventArgs e)
        {
            NotifyOnMediaDelete?.Invoke(this, new MediaEventArgs(new Media { Path=e.FullPath }));
        }
      
        private void _fs_Error(object sender, ErrorEventArgs e)
        {
            Debug.WriteLine($"Watcher error: {e.ToString()}");
        }

        public void AddMediaToTrack(Media m)
        {
            lock (_mediaListToTrack)
            {
                if (_mediaListToTrack.FirstOrDefault(param => param.Path == m.Path) != null)
                    return;
                Debug.WriteLine($"Media {m.Name} added to track list");
                _mediaListToTrack.Add(m);
            }

            if (!IsTracking)
                TrackMedia();
        }

        public void AddMediaToVerify(Media m)
        {
            lock (_mediaListToVerify)
            {
                if (_mediaListToVerify.FirstOrDefault(param => param.Path == m.Path) != null)
                    return;

                _mediaListToVerify.Add(m);
            }

            if (!IsVerificating)
                VerifyMedia();
        }

        private void TrackMedia()
        {
            IsTracking = true;
            Task.Run(async () =>
            {
                Debug.WriteLine("Tracking Task started");
                while (true)
                {
                    lock (_mediaListToTrack)
                        if (_mediaListToTrack.Count < 1)
                        {
                            IsTracking = false;
                            break;
                        }
                    await Task.Delay(5000);
                    List<Media> mediaListToTrack;
                    lock (_mediaListToTrack)
                        mediaListToTrack = _mediaListToTrack.ToList();

                    foreach (var media in mediaListToTrack)
                    {
                        Debug.WriteLine($"Tracking {media.Path}");
                        if (!File.Exists(media.Path))
                            continue;

                        try
                        {
                            var time = MediaExtensions.TryGetDuration(media.Path);
                            if (time == media.Duration)
                            {
                                Debug.WriteLine($"Media {media.Name} deleted from track list");
                                lock (_mediaListToTrack)
                                    _mediaListToTrack.Remove(media);
                            }
                                
                            else
                            {
                                media.Duration = time;
                                NotifyOnMediaChanged?.Invoke(this, new MediaEventArgs(new Media { Path = media.Path, Duration = time }));
                            }
                        }
                        catch
                        {
                            continue;
                        }
                    }
                }
                Debug.WriteLine("Tracking task Exit");
            });
        }

        private void VerifyMedia()
        {
            IsVerificating = true;
            Task.Run(async () =>
            {
                Debug.WriteLine("Verify Task started");
                while (true)
                {
                    lock (_mediaListToVerify)
                        if (_mediaListToVerify.Count() < 1)
                        {
                            IsVerificating = false;
                            break;
                        }

                    List<Media> mediaListToVerify;
                    lock (_mediaListToVerify)
                        mediaListToVerify = _mediaListToVerify.ToList();

                    foreach (var media in mediaListToVerify)
                    {
                        if (!File.Exists(media.Path))
                            continue;

                        try
                        {
                            var time = MediaExtensions.TryGetDuration(media.Path);
                            lock (_mediaListToVerify)
                                _mediaListToVerify.Remove(media);
                            NotifyOnMediaVerified?.Invoke(this, new MediaEventArgs(new Media { Path = media.Path, Duration = time }));
                        }
                        catch
                        {
                            continue;
                        }
                    }
                    await Task.Delay(500);
                }
                Debug.WriteLine("Verify task Exit");
            });
        }       
    }
}
