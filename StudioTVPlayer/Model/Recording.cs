using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    /// <summary>
    /// describes recording in progress
    /// </summary>
    public class Recording: IDisposable
    {
        private bool _disposed;

        public Recording(InputBase input)
        {
            Input = input;
        }

        public InputBase Input { get; }

        public string FullPath { get; private set; }
        public EncoderPreset EncoderPreset { get; private set; }

        public void StartRecording(string fullPath, EncoderPreset preset)
        {
            FullPath = fullPath;
            EncoderPreset = preset;
            Providers.GlobalApplicationData.Current.AddRecording(this);
        }

        public void StopRecording() 
        {
            Finished?.Invoke(this, EventArgs.Empty);
        }

        public event EventHandler Finished;

        public void Dispose()
        {
            if (!_disposed)
            {
                StopRecording();
                _disposed = true;
            }
        }
    }
}
