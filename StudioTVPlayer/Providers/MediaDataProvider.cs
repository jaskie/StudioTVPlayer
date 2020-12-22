using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using StudioTVPlayer.Extensions;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.Services;
using StudioTVPlayer.ViewModel.Main.Browser;

namespace StudioTVPlayer.Providers
{
    public class MediaDataProvider : IMediaDataProvider
    {
        public MediaWatcherService StartWatcher(string path)
        {
            return new MediaWatcherService(path);
        }

        public ObservableCollection<BrowserTabViewModel> GetBrowserTabs()
        {
            var browserTabs = new ObservableCollection<BrowserTabViewModel>();
            foreach (var item in SimpleIoc.Get<IGlobalApplicationData>().Configuration.WatchedFolders)
            {
                var tab = SimpleIoc.Get<BrowserTabViewModel>();
                tab.WatcherMeta = item;
                browserTabs.Add(tab);
            }
            return browserTabs;
        }    
        
        public BrowserTabItemViewModel GetNewBrowserTabItem(Media m)
        {
            var item = SimpleIoc.Get<BrowserTabItemViewModel>();
            item.Media = m;

            return item;
        }

        public List<BrowserTabItemViewModel> GetBrowserTabItems(string path)
        {
            List<BrowserTabItemViewModel> tabItems = new List<BrowserTabItemViewModel>();

            var mediaFiles = Directory.GetFiles(path)
                .Where(MediaExtensions.IsMediaFile)
                .Select(p => new Media
                {
                    Path = p,
                    Name = Path.GetFileName(p),
                    CreationDate = File.GetCreationTime(p)
                });

            foreach(var media in mediaFiles)
            {
                var tabItem = SimpleIoc.Get<BrowserTabItemViewModel>();
                tabItem.Media = media;
                tabItems.Add(tabItem);
            }

            return tabItems;               
        }

        public List<BrowserTabItemViewModel> ReloadBrowserTabItems()
        {
            var list = new List<BrowserTabItemViewModel>();

            return list;
        }

        public List<BrowserTabViewModel> ReloadBrowserTabs()
        {
            var list = new List<BrowserTabViewModel>();
            var oldTabs = SimpleIoc.GetInstances<BrowserTabViewModel>();

            foreach (var tab in oldTabs)
            {

            }

            return list;
        }

        public void LoadMediaFiles(ICollectionView mediaView, IMediaWatcherService mediaWatcher)
        {
            Task.Run(() =>
            {
                IEnumerable<BrowserTabItemViewModel> enumerable = mediaView.Cast<BrowserTabItemViewModel>().ToList();
                foreach (BrowserTabItemViewModel browserVm in enumerable)
                {
                    Application.Current?.Dispatcher.BeginInvoke((Action)(() =>
                    {
                        browserVm.GetResourceThumbnail(ThumbnailType.Loading);
                    }));

                    if (browserVm.GetFFMeta())
                        browserVm.IsVerified = true;
                    else
                    {
                        browserVm.IsVerified = false;
                        mediaWatcher.AddMediaToTrack(browserVm.Media);
                    }
                }
            });
        }

        public void UnloadMediaFiles(ICollectionView medias)
        { 
            foreach (BrowserTabItemViewModel browserVM in medias)
            {
                Application.Current.Dispatcher.BeginInvoke((Action)(() =>
                    {     
                        if(!browserVM.IsQueued)
                            browserVM.Thumbnail = null;
                    }));
            }
        }
    }
}
