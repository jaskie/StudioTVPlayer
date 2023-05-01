namespace StudioTVPlayer.Helpers
{
    internal static class FolderHelper
    {
        public static bool Browse(ref string currentFolder, string description)
        {            
            using (var dialog = new Microsoft.WindowsAPICodePack.Dialogs.CommonOpenFileDialog(description) 
            { 
                IsFolderPicker = true,
                ShowPlacesList = true,
                Multiselect = false,
                DefaultDirectory = currentFolder, 

            })
            {
                var result = dialog.ShowDialog();
                if (result == Microsoft.WindowsAPICodePack.Dialogs.CommonFileDialogResult.Ok)
                {
                    currentFolder = dialog.FileName;
                    return true;
                }
                return false;
            }
        }

    }
}
