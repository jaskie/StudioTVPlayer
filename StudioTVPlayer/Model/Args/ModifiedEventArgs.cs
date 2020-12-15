using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class ModifiedEventArgs : EventArgs
    {
        public bool IsModified { get; }
        public ModifiedEventArgs(bool isModified)
        {
            IsModified = isModified;
        }
    }
}
