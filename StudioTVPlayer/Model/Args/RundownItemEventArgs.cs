using System;

namespace StudioTVPlayer.Model.Args
{
    public class RundownItemEventArgs : EventArgs
    {
        public RundownItemEventArgs(RundownItem rundownItem)
        {
            RundownItem = rundownItem;
        }

        public RundownItem RundownItem { get; }
    }
}
