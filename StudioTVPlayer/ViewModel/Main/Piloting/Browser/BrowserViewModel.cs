using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media.Imaging;
using StudioTVPlayer.Extensions;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Args;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.Properties;

namespace StudioTVPlayer.ViewModel.Main.Piloting.Browser
{
    public class BrowserViewModel : ViewModelBase
    {
        private ObservableCollection<BrowserTabViewModel> _browserTabs = new ObservableCollection<BrowserTabViewModel>();
        public ObservableCollection<BrowserTabViewModel> BrowserTabs
        {
            get => _browserTabs;
            set
            {
                Set(ref _browserTabs, value);
            }
        }

        private bool _isFocused;
        public bool IsFocused
        {
            get => _isFocused;
            set
            {
                Set(ref _isFocused, value);
            }
        }


        public BrowserViewModel(IMediaDataProvider mediaDataProvider)
        {
            BrowserTabs = mediaDataProvider.GetBrowserTabs();
        }
    }
}
