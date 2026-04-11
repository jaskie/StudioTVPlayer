using System;

namespace StudioTVPlayer.Model.Args
{
    public class RundownItemEventArgs(RundownItemBase rundownItem) : EventArgs
    {
        public RundownItemBase RundownItem { get; } = rundownItem;
    }

    public class RundownItemIndexedEventArgs(RundownItemBase item, int index) : RundownItemEventArgs(item)
    {
        public int Index { get; } = index;
    }

}
