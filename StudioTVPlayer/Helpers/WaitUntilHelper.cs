using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
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
                await Task.Delay(delay, cancellationToken);
            }
        }

    }
}
