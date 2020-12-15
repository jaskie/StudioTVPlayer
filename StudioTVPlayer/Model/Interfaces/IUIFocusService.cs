using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model.Interfaces
{
    public interface IUIFocusService
    {
        void FocusPlayer(int index);
        void FocusBrowser();
    }
}
