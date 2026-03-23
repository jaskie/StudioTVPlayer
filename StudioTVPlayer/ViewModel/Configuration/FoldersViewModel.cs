using StudioTVPlayer.Helpers;
using System;
using System.Collections.ObjectModel;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class FoldersViewModel : ModifyableViewModelBase
    {
        private readonly MahApps.Metro.Controls.Dialogs.IDialogCoordinator _dialogCoordinator = MahApps.Metro.Controls.Dialogs.DialogCoordinator.Instance;

        public FoldersViewModel()
        {
            WatchedFolders = new(Providers.Configuration.Current.WatchedFolders.Select(f =>
                {
                    var vm = new WatchedFolderViewModel(f);
                    vm.Modified += (o, e) => IsModified = true;
                    vm.RemoveRequested += WatchedFolder_RemoveRequested;
                    return vm;
                }));
            AddWatchedFolderCommand = new UiCommand(AddWatchedFolder);
        }

        public UiCommand AddWatchedFolderCommand { get; }

        public ObservableCollection<WatchedFolderViewModel> WatchedFolders { get; }

        private void AddWatchedFolder(object obj)
        {
            string path = Environment.GetFolderPath(Environment.SpecialFolder.MyComputer);
            if (FolderHelper.BrowseForFolder(ref path, "Select path for new watched folder"))
            {
                var name = path.Split(new[] { System.IO.Path.DirectorySeparatorChar }, StringSplitOptions.RemoveEmptyEntries).LastOrDefault() ?? "New watched folder";
                var vm = new WatchedFolderViewModel(new Model.WatchedFolder() { Path = path, Name = name, Filter = "*.mov;*.mp4;*.mxf" });
                vm.RemoveRequested += WatchedFolder_RemoveRequested;
                WatchedFolders.Add(vm);
                IsModified = true;
            }
        }

        private async void WatchedFolder_RemoveRequested(object sender, EventArgs e)
        {
            var folder = sender as WatchedFolderViewModel ?? throw new ArgumentException(nameof(sender));
            if (await _dialogCoordinator.ShowMessageAsync(ShellViewModel.Instance, "Confirmation", $"Really remove watched folder \"{folder.Name}\"?", MahApps.Metro.Controls.Dialogs.MessageDialogStyle.AffirmativeAndNegative) != MahApps.Metro.Controls.Dialogs.MessageDialogResult.Affirmative)
                return;
            WatchedFolders.Remove(folder);
            IsModified = true;
        }

        public override void Apply()
        {
            Providers.Configuration.Current.WatchedFolders = [.. WatchedFolders.Select(f => {
                f.Apply();
                return f.WatchedFolder;
            })];
            base.Apply();
        }

        public override bool IsValid()
        {
            return WatchedFolders.All(f => f.IsValid());
        }
    }
}
