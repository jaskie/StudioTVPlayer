using System.Threading.Tasks;

namespace StudioTVPlayer.Helpers
{
    public interface IConfirmClose
    {
        Task<bool> ConfirmCloseAsync();
    }
}
