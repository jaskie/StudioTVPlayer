using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public abstract class PlayerControllerBase : IDisposable
    {
        public abstract void Dispose();
    }
}
