using System;

namespace StudioTVPlayer.Model.Args
{
    public class RundownItemEventArgs : EventArgs
    {
        public RundownItemEventArgs(FileRundownItem rundownItem)
        {
            RundownItem = rundownItem;
        }

        public FileRundownItem RundownItem { get; }
    }
}
