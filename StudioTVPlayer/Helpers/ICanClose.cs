using System.Threading.Tasks;

namespace StudioTVPlayer.Helpers
{
    public interface ICanClose
    {
        Task<bool> ConfirmCloseAsync();
    }
}
