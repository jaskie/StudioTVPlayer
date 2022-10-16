using StudioTVPlayer.Providers;
using System;
using System.Collections.Generic;

namespace StudioTVPlayer.ViewModel.Main
{
    public class RecorderViewModel : RemovableViewModelBase
    {
        private Model.InputBase _input;

        public RecorderViewModel(Model.InputBase input)
        {
            _input = input;
        }

        public string Folder { get; set; }

        public string FileName { get; set; }

        public Model.EncoderPreset EncoderPreset { get; set; }

        public static IEnumerable<Model.EncoderPreset> EncoderPresets => GlobalApplicationData.Current.EncoderPresets;

        public override void Apply()
        {
            throw new NotImplementedException();
        }

        public override bool IsValid()
        {
            return true;
        }

    }
}
