using System;
using System.Diagnostics;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace StudioTVPlayer.Controls
{
    public class Timecode : TextBox
    {
        public static readonly DependencyProperty TimeProperty =
            DependencyProperty.Register(
            "Time",
            typeof(TimeSpan),
            typeof(Timecode),
            new FrameworkPropertyMetadata(TimeSpan.Zero, OnTimeChanged));

        public static readonly DependencyProperty VideoFormatProperty =
            DependencyProperty.Register(
                "VideoFormat",
                typeof(TVPlayR.VideoFormat),
                typeof(Timecode),
                new PropertyMetadata(OnTimeChanged));

        public static readonly DependencyProperty EnterPressedCommandProperty =
           DependencyProperty.Register(
           "EnterPressedCommand",
           typeof(ICommand),
           typeof(Timecode),
           new FrameworkPropertyMetadata(OnEnterPressedChanged));


        private static void OnEnterPressedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is TextBox element)                 
                element.KeyDown += EnterPressed;                                  
        }

        private static void EnterPressed(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter && sender is Timecode element)
            {
                var vf = GetVideoFormat(element);
                SetTime(element, vf.FrameNumberToTime(vf.StringToFrameNumber(element.Text)));

                ICommand command = GetEnterPressedCommand((TextBox)sender);
                object[] param = { sender, e };

                command.Execute(param);
            }
        }

        private static void OnTimeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (!(d is Timecode element)) return;
            var vf = GetVideoFormat(d);
            element.Text = vf.FrameNumberToString(vf.TimeToFrameNumber(GetTime(d)), false);
        }

        public static void SetEnterPressedCommand(UIElement element, ICommand value) => element.SetValue(EnterPressedCommandProperty, value);

        public static ICommand GetEnterPressedCommand(UIElement element) => (ICommand)element.GetValue(EnterPressedCommandProperty);

        public static TimeSpan GetTime(DependencyObject d) => (TimeSpan)d.GetValue(TimeProperty);

        public static void SetTime(DependencyObject d, TimeSpan value) => d.SetValue(TimeProperty, value);

        public static TVPlayR.VideoFormat GetVideoFormat(DependencyObject d) => (TVPlayR.VideoFormat)d.GetValue(VideoFormatProperty);

        public static void SetVideoFormat(DependencyObject d, TVPlayR.VideoFormat value) => d.SetValue(VideoFormatProperty, value);
    }
}
