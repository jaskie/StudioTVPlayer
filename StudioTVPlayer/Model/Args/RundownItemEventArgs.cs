using System;

namespace StudioTVPlayer.Model.Args
{
    public class RundownItemEventArgs : EventArgs
    {
        public RundownItemEventArgs(RundownItemBase rundownItem)
        {
            RundownItem = rundownItem;
        }

        public RundownItemBase RundownItem { get; }
    }

    public class RundownItemIndexedEventArgs: RundownItemEventArgs
    {
        public RundownItemIndexedEventArgs(RundownItemBase item, int index) : base(item)
        {
            Index = index;
        }

        public int Index { get; }
    }

}
