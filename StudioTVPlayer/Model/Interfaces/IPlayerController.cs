using StudioTVPlayer.Model;

namespace StudioTVPlayer.Providers
{
    public interface IPlayerController
    {
        int Load(Media m);
        int Play();
        int Pause();
        int Stop();
        int Next();
    }
}