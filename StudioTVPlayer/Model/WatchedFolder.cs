using StudioTVPlayer.Model.Args;
using StudioTVPlayer.Providers;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Management.Automation;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class WatchedFolder
    {
        private bool _needsInitialization;
        private string _path;
        private bool _isFilteredByDate;
        private string _filter = "*.mp4;*.mov;*.mxf";
        private FileSystemWatcher _fs;
        private WildcardPattern _wildcardPattern;
        private readonly List<Media> _medias = new List<Media>();


        [XmlAttribute]
        public string Name { get; set; }

        [XmlAttribute]
        public string Path { get => _path; set => Set(ref _path, value); }

        [XmlAttribute]
        public bool IsFilteredByDate { get => _isFilteredByDate; set => Set(ref _isFilteredByDate, value); }

        [XmlAttribute]
        public string Filter { get => _filter; set => Set(ref _filter, value); } 

        public event EventHandler<MediaEventArgs> MediaChanged;

        public void Initialize()
        {
            if (!_needsInitialization)
                return;
            _needsInitialization = false;
            _wildcardPattern = new WildcardPattern(Filter, WildcardOptions.IgnoreCase);
            _fs = new FileSystemWatcher(Path)
            {
                NotifyFilter = NotifyFilters.FileName | NotifyFilters.Size | NotifyFilters.LastWrite | NotifyFilters.CreationTime,
                InternalBufferSize = 64000 //zalecany max
            };
            _fs.Error += Fs_Error;
            _fs.Created += Fs_MediaCreated;
            _fs.Deleted += Fs_MediaDeleted;
            _fs.Renamed += Fs_MediaRenamed;
            _fs.Changed += Fs_MediaChanged;
            _fs.EnableRaisingEvents = true;
            lock (((IList)_medias).SyncRoot)
            {
                _medias.Clear();
                foreach (var media in Directory.EnumerateFiles(Path)
                    .Where(Accept)
                    .Select(path => new Media(path)))
                {
                    _medias.Add(media);
                    MediaVerifier.Current.Queue(media, 90);
                }
            }
        }

        public IList<Media> Medias
        {
            get
            {
                lock (((IList)_medias).SyncRoot)
                    return _medias.AsReadOnly();
            }
        }

        private bool Accept(string fullPath)
        {
            return _wildcardPattern.IsMatch(fullPath);
        }

        private void Fs_MediaCreated(object sender, FileSystemEventArgs e)
        {
            Debug.WriteLine("Media Created Notified");
            if (!Accept(e.FullPath))
                return;
            MediaChanged?.Invoke(this, new MediaEventArgs(AddMediaFromPath(e.FullPath), MediaEventKind.Create));
        }

        private void Fs_MediaChanged(object sender, FileSystemEventArgs e)
        {
            Debug.WriteLine("Media Changed Notified");
            Media media;
            lock (((IList)_medias).SyncRoot)
                media = _medias.FirstOrDefault(m => m.FullPath == e.FullPath);
            if (media == null)
                return;
            MediaChanged?.Invoke(this, new MediaEventArgs(media, MediaEventKind.Change));
        }

        private void Fs_MediaRenamed(object sender, RenamedEventArgs e)
        {
            Debug.WriteLine("Media Renamed Notified");
            Media media;
            lock (((IList)_medias).SyncRoot)
                media = _medias.FirstOrDefault(m => m.FullPath == e.OldFullPath);
            if (media == null && Accept(e.FullPath))
            {
                MediaChanged?.Invoke(this, new MediaEventArgs(AddMediaFromPath(e.FullPath), MediaEventKind.Create));
            }
            else if (media != null && !Accept(e.FullPath))
            {
                lock (((IList)_medias).SyncRoot)
                    _medias.Remove(media);
                MediaChanged?.Invoke(this, new MediaEventArgs(media, MediaEventKind.Delete));
            }
            else if (media != null)
            {
                media.Name = e.Name;
                media.DirectoryName = System.IO.Path.GetDirectoryName(e.FullPath);
                MediaChanged?.Invoke(this, new MediaEventArgs(media, MediaEventKind.Change));
            }
        }

        private Media AddMediaFromPath(string fullPath)
        {
            var media = new Media(fullPath);
            lock (((IList)_medias).SyncRoot)
                _medias.Add(media);
            return media;
        }

        private void Fs_MediaDeleted(object sender, FileSystemEventArgs e)
        {
            Debug.WriteLine("Media Delete Notified");
            Media media;
            lock (((IList)_medias).SyncRoot)
                media = _medias.FirstOrDefault(m => m.FullPath == e.FullPath);
            if (media != null)
                MediaChanged?.Invoke(this, new MediaEventArgs(media, MediaEventKind.Change));
        }

        private void Fs_Error(object sender, ErrorEventArgs e)
        {
            Debug.WriteLine($"Watcher error: {e}");
        }

        private void Set<T>(ref T field, T value)
        {
            if (EqualityComparer<T>.Default.Equals(field, value))
                return;
            field = value;
            _needsInitialization = true;
        }

    }
}
