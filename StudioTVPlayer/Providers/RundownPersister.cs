using StudioTVPlayer.Helpers;
using System;
using System.Linq;

namespace StudioTVPlayer.Providers
{
    public static class RundownPersister
    {
        /// <summary>
        /// Saves <see cref="Model.Rundown"/> to file
        /// </summary>
        public static void SaveRundown(Model.Rundown rundown, string fileName)
        {
            var items = rundown.Items
                .Select<Model.RundownItemBase, Model.Persistence.RundownItemBase>(item => item switch
                    {
                        Model.FileRundownItem fileRundownItem => new Model.Persistence.FileRundownItem { FileName = fileRundownItem.Media.FullPath, IsAutoStart = fileRundownItem.IsAutoStart },
                        Model.LiveInputRundownItem liveInputRundownItem when liveInputRundownItem.Input is Model.DecklinkInput decklinkInput => new Model.Persistence.DecklinkInputRundownItem { DeviceIndex = decklinkInput.DeviceIndex, IsAutoStart = item.IsAutoStart },
                        _ => throw new NotImplementedException(),
                    });
            new Model.Persistence.Rundown { RundownItems = [.. items] }.Save(fileName);
        }

        /// <summary>
        /// Loads rundown from file into existing <see cref="Model.Rundown"/> instance
        /// </summary>
        public static void LoadRundown(Model.Rundown rundown, string fileName)
        {
            var savedRundown = DataStore.Load<Model.Persistence.Rundown>(fileName) ?? throw new ApplicationException($"File {fileName} can't be loaded");
            rundown.ClearItems();
            foreach (var savedItem in savedRundown.RundownItems)
            {
                Model.RundownItemBase rundownItem = null;
                switch (savedItem)
                {
                    case Model.Persistence.FileRundownItem fileRundownItem:
                        var mediaFile = new Model.MediaFile(fileRundownItem.FileName);
                        if (Model.MediaVerifier.Current.Verify(mediaFile) && mediaFile.IsValid)
                            rundownItem = new Model.FileRundownItem(mediaFile);
                        break;
                    case Model.Persistence.DecklinkInputRundownItem decklinkInputRundownItem:
                        var input = InputList.Current.Inputs.OfType<Model.DecklinkInput>().FirstOrDefault(i => i.DeviceIndex == decklinkInputRundownItem.DeviceIndex);
                        if (input != null)
                            rundownItem = new Model.LiveInputRundownItem(input);
                        break;
                }
                if (rundownItem is null)
                    continue;
                rundownItem.IsAutoStart = savedItem.IsAutoStart;
                rundown.Add(rundownItem);
            }
        }
    }
}
