using StudioTVPlayer.Extensions;
using StudioTVPlayer.Model.Args;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class WatchedFolder
    {
        private bool _needsInitialization;
        private string _path;
        private bool _isFilteredByDate;
        private string _filter;
        private FileSystemWatcher _fs;

        private List<Media> _medias;

        [XmlAttribute]
        public string Name { get; set; }

        [XmlAttribute]
        public string Path { get => _path; set => Set(ref _path, value); }

        [XmlAttribute]
        public bool IsFilteredByDate { get => _isFilteredByDate; set => Set(ref _isFilteredByDate, value); }

        [XmlAttribute]
        public string Filter { get => _filter; set => Set(ref _filter, value); }

        public event EventHandler<MediaEventArgs> MediaChanged;
        public event EventHandler<MediaEventArgs> MediaAdd;
        public event EventHandler<MediaEventArgs> MediaVerified;
        public event EventHandler<MediaEventArgs> MediaDelete;

        public void Initialize()
        {
            if (!_needsInitialization)
                return;
            _fs = new FileSystemWatcher(Path);
            _fs.NotifyFilter = NotifyFilters.FileName | NotifyFilters.Size | NotifyFilters.LastWrite | NotifyFilters.CreationTime;
            _fs.InternalBufferSize = 64000; //zalecany max
            _fs.Error += Fs_Error;
            _fs.Created += Fs_MediaCreated;
            _fs.Deleted += Fs_MediaDeleted;
            _fs.Renamed += Fs_MediaRenamed;
            _fs.Changed += Fs_MediaChanged;
            _fs.EnableRaisingEvents = true;
            _medias.AddRange(Directory.EnumerateFiles(Path)
                .Where(Accept)
                .Select(path => new Media(path)));
        }

        public IList<Media> Medias => _medias.AsReadOnly();

        private bool Accept(string m)
        {
            throw new NotImplementedException();
        }

        private void Fs_MediaCreated(object sender, FileSystemEventArgs e)
        {
            Debug.WriteLine("Media Created Notified");
            if (!MediaExtensions.IsMediaFile(e.FullPath))
                return;

            Media m = new Media(e.FullPath);

            MediaAdd?.Invoke(this, new MediaEventArgs(m));
        }

        private void Fs_MediaChanged(object sender, FileSystemEventArgs e)
        {
            Debug.WriteLine("Media Changed Notified");
            if (e.ChangeType != WatcherChangeTypes.Changed)
                return;

            //MediaChanged?.Invoke(this, new MediaEventArgs(new Media { DirectoryName = e.FullPath }));
        }

        private void Fs_MediaRenamed(object sender, RenamedEventArgs e)
        {
            Debug.WriteLine("Media Renamed Notified");
            //MediaRenamed?.Invoke(this, new MediaEventArgs(new Media { DirectoryName = e.FullPath }, e.OldFullPath));
        }

        private void Fs_MediaDeleted(object sender, FileSystemEventArgs e)
        {
            //MediaDelete?.Invoke(this, new MediaEventArgs(new Media { DirectoryName = e.FullPath }));
        }

        private void Fs_Error(object sender, ErrorEventArgs e)
        {
            Debug.WriteLine($"Watcher error: {e.ToString()}");
        }

        private void Set<T>(ref T field, T value)
        {
            if (EqualityComparer<T>.Default.Equals(field, value))
                return;
            _needsInitialization = true;
        }

    }
}
