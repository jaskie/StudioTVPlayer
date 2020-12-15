using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static StudioTVPlayer.Model.Enums;

namespace StudioTVPlayer.Model.Interfaces
{
    public interface INavigationService
    {
        void SwitchView(ViewType view);
    }
}
