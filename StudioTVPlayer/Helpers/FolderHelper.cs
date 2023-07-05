namespace StudioTVPlayer.Helpers
{
    internal static class FolderHelper
    {
        public static bool BrowseForFolder(ref string folder, string description)
        {            
            using (var dialog = new Microsoft.WindowsAPICodePack.Dialogs.CommonOpenFileDialog(description) 
            { 
                IsFolderPicker = true,
                ShowPlacesList = true,
                Multiselect = false,
                DefaultDirectory = folder, 

            })
            {
                var result = dialog.ShowDialog();
                if (result == Microsoft.WindowsAPICodePack.Dialogs.CommonFileDialogResult.Ok)
                {
                    folder = dialog.FileName;
                    return true;
                }
                return false;
            }
        }

        public static string SaveFileDialog(string description, string filterName, string fileExtension)
        {
            using (var dialog = new Microsoft.WindowsAPICodePack.Dialogs.CommonSaveFileDialog(description)
            {
                DefaultExtension = fileExtension,
                Filters = { new Microsoft.WindowsAPICodePack.Dialogs.CommonFileDialogFilter(filterName, fileExtension) }
            })
            {
                if (dialog.ShowDialog() == Microsoft.WindowsAPICodePack.Dialogs.CommonFileDialogResult.Ok)
                    return dialog.FileName;
                return null;
            }
        }

        public static string OpenFileDialog(string description, string filterName, string fileExtension)
        {
            using (var dialog = new Microsoft.WindowsAPICodePack.Dialogs.CommonOpenFileDialog(description)
            {
                DefaultExtension = fileExtension,
                Filters = { new Microsoft.WindowsAPICodePack.Dialogs.CommonFileDialogFilter(filterName, fileExtension) }
            })
            {
                if (dialog.ShowDialog() == Microsoft.WindowsAPICodePack.Dialogs.CommonFileDialogResult.Ok)
                    return dialog.FileName;
                return null;
            }
        }

    }
}
