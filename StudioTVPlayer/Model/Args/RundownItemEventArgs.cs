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
}
