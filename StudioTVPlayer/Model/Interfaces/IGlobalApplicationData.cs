using TVPlayR;

namespace StudioTVPlayer.Model.Interfaces
{
    public interface IGlobalApplicationData
    {
        Configuration Configuration { get; }
        DecklinkDevice[] DecklinkDevices { get; }
        VideoFormat[] VideoFormats { get; }
        PixelFormat[] PixelFormats { get; }

        void SaveConfiguration();
    }
}
