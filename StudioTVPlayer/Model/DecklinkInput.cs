﻿using System;
using System.Linq;
using System.Windows;
using System.Windows.Media;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class DecklinkInput : InputBase, IDisposable
    {
        private TVPlayR.DecklinkInput _input;
        private TVPlayR.InputPreview _preview;

        [XmlAttribute]
        public int DeviceIndex { get; set; }

        public override ImageSource Thumbnail => _preview?.PreviewSource;

        public override void Dispose()
        {
            if (_input is null)
                return;
            Uninitialize();
        }

        public TVPlayR.DecklinkInput GetInput() => _input;

        public override bool Initialize()
        {
            Uninitialize();
            var videoFormat = TVPlayR.VideoFormat.Formats.FirstOrDefault(f => f.Name == VideoFormat);
            var info = TVPlayR.DecklinkIterator.Devices.FirstOrDefault(i => i.Index == DeviceIndex);
            if (info is null || videoFormat is null)
                return false;
            try
            {
                _input = TVPlayR.DecklinkIterator.CreateInput(info, videoFormat, 2, TVPlayR.DecklinkTimecodeSource.None, true);
                _preview = new TVPlayR.InputPreview(Application.Current.Dispatcher, 160, 90);
                _input.AddPreview(_preview);
                return true;
            }
            catch
            {
                return false;
            }
        }

        public override void Uninitialize()
        {
            if (!(_preview is null))
                _input.RemovePreview(_preview);
            _input?.Dispose();
            _preview?.Dispose();
            _input = null;
            _preview = null;
        }

        public override bool IsRunning => !(_input is null);
    }
}
