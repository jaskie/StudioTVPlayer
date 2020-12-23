using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Helpers
{
    internal static class FolderHelper
    {
        public static bool Browse(ref string currentFolder, string description = "")
        {
            using (var dialog = new System.Windows.Forms.FolderBrowserDialog() { SelectedPath = currentFolder, Description = description, RootFolder = Environment.SpecialFolder.MyComputer })
            {
                System.Windows.Forms.DialogResult result = dialog.ShowDialog();
                if (result == System.Windows.Forms.DialogResult.OK)
                {
                    currentFolder = dialog.SelectedPath;
                    return true;
                }
                return false;
            }
        }

    }
}
