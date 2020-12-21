using System.Collections.Generic;

namespace StudioTVPlayer.Model.Interfaces
{
    public interface IGlobalApplicationData
    {
        Configuration Configuration { get; }
        void Save();
    }
}
