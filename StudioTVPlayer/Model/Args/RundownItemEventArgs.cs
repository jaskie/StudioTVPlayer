using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
