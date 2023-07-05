using StudioTVPlayer.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model.Persistence
{
    public static class RundownPersister
    {
        public static void SaveRundown(Model.Rundown rundown, string fileName)
        {
            var items = rundown.Items
                .Select<Model.RundownItemBase, RundownItemBase>(item =>
                {
                    switch (item)
                    {
                        case Model.FileRundownItem fileRundownItem:
                            return new FileRundownItem { FileName = fileRundownItem.Media.FullPath, IsAutoStart = fileRundownItem.IsAutoStart };
                        case Model.LiveInputRundownItem liveInputRundownItem when liveInputRundownItem.Input is DecklinkInput decklinkInput:
                            return new DecklinkInputRundownItem { DeviceIndex = decklinkInput.DeviceIndex, IsAutoStart = item.IsAutoStart };
                        default:
                            throw new NotImplementedException();
                    }
                })
                .ToArray();
            new Rundown { RundownItems = items }.Save(fileName);
        }

        public static void LoadRundown(Model.Rundown rundown, string fileName)
        {
            var savedRundown = DataStore.Load<Rundown>(fileName);
            if (savedRundown is null)
                throw new ApplicationException($"file {fileName} not contain valid rundown information");
            rundown.ClearItems();
            foreach (var savedItem in savedRundown.RundownItems)
            {
                Model.RundownItemBase rundownItem = null;
                switch (savedItem)
                {
                    case FileRundownItem fileRundownItem:
                        var mediaFile = new MediaFile(fileRundownItem.FileName);
                        MediaVerifier.Current.Verify(mediaFile);
                        if (mediaFile.IsValid)
                            rundownItem = new Model.FileRundownItem(mediaFile);
                        break;
                    case DecklinkInputRundownItem decklinkInputRundownItem:
                        var input = Providers.InputList.Current.Inputs.OfType<Model.DecklinkInput>().FirstOrDefault(i => i.DeviceIndex == decklinkInputRundownItem.DeviceIndex);
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
