using System;
using System.Threading;
using System.Threading.Tasks;

namespace StudioTVPlayer.Helpers
{
    public static class WaitUntilHelper
    {
        public static async Task WaitUntilAsync(DateTime dateTime, CancellationToken cancellationToken)
        {
            var delay = dateTime - DateTime.Now;
            if (delay > TimeSpan.Zero)
            {
                try
                {
                    await Task.Delay(delay, cancellationToken);
                }
                catch (TaskCanceledException) { }
            }
        }

        public static async Task WaitForAsync(TimeSpan timeSpan)
        {
            if (timeSpan > TimeSpan.Zero)
            {
                await Task.Delay(timeSpan);
            }
        }

    }
}
