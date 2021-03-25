using System;
using System.Diagnostics;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace StudioTVPlayer.Controls
{
    public class TimecodeEdit : TextBox
    {
        public static readonly DependencyProperty TimeProperty =
            DependencyProperty.Register(
                nameof(Time),
                typeof(TimeSpan),
                typeof(TimecodeEdit),
                new FrameworkPropertyMetadata(TimeSpan.Zero, OnTimeChanged));

        public static readonly DependencyProperty VideoFormatProperty =
            DependencyProperty.Register(
                nameof(VideoFormat),
                typeof(TVPlayR.VideoFormat),
                typeof(TimecodeEdit),
                new PropertyMetadata(OnTimeChanged));

        public static readonly DependencyProperty IsDropFrameProperty =
            DependencyProperty.Register(
                nameof(IsDropFrame),
                typeof(bool),
                typeof(TimecodeEdit),
                new PropertyMetadata(OnTimeChanged));

        public static readonly DependencyProperty EnterPressedCommandProperty =
           DependencyProperty.Register(
               "EnterPressedCommand",
               typeof(ICommand),
               typeof(TimecodeEdit),
               new FrameworkPropertyMetadata(OnEnterPressedChanged));


        ///<summary>
        /// Default  constructor
        ///</summary>
        public TimecodeEdit()
        {
            this.DefaultStyleKey = typeof(TextBox);
            //cancel the paste and cut command
            CommandBindings.Add(new CommandBinding(ApplicationCommands.Paste, null, DoNothingCommand));
            CommandBindings.Add(new CommandBinding(ApplicationCommands.Cut, null, DoNothingCommand));
        }


        private void DoNothingCommand(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = false;
            e.Handled = true;
        }

        private static void OnEnterPressedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is TextBox element)                 
                element.KeyDown += EnterPressed;                                  
        }

        private static void EnterPressed(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter && sender is TimecodeEdit element && element.VideoFormat is TVPlayR.VideoFormat vf)
            {
                element.Time = vf.FrameNumberToTime(vf.StringToFrameNumber(element.Text));
                ICommand command = GetEnterPressedCommand((TextBox)sender);
                object[] param = { sender, e };
                command?.Execute(param);
            }
        }

        private static void OnTimeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (!(d is TimecodeEdit element && element.VideoFormat is TVPlayR.VideoFormat vf)) return;
            element.Text = vf.FrameNumberToString(vf.TimeToFrameNumber(element.Time), element.IsDropFrame);
        }

        public static void SetEnterPressedCommand(UIElement element, ICommand value) => element.SetValue(EnterPressedCommandProperty, value);

        public static ICommand GetEnterPressedCommand(UIElement element) => (ICommand)element.GetValue(EnterPressedCommandProperty);

        public TimeSpan Time
        {
            get => (TimeSpan)GetValue(TimeProperty);
            set => SetValue(TimeProperty, value);
        }

        public TVPlayR.VideoFormat VideoFormat
        {
            get => (TVPlayR.VideoFormat)GetValue(VideoFormatProperty);
            set => SetValue(VideoFormatProperty, value);
        }

        public bool IsDropFrame
        {
            get => (bool)GetValue(IsDropFrameProperty);
            set => SetValue(IsDropFrameProperty, value);
        }


        /// <summary>
        /// override this method to replace the characters enetered with the mask
        /// </summary>
        protected override void OnPreviewTextInput(TextCompositionEventArgs e)
        {
            //if the text is readonly do not add the text
            if (!IsReadOnly)
            {
                int selectionStart = SelectionStart;
                int position = selectionStart;
                if (selectionStart < Text.Length)
                {
                    var sb = new StringBuilder(Text);
                    Debug.Assert(sb.Length == 11);
                    for (int i = 0; i < e.Text.Length; i++)
                    {
                        if (IsValidCharAt(e.Text[i], position))
                        {
                            sb.Remove(position, 1);
                            sb.Insert(position++, e.Text[i]);
                        }
                        else if ((position == 2 || position == 5 || position == 8) && IsValidCharAt(e.Text[i], position + 1)) // separators
                        {
                            position++;
                            sb.Remove(position, 1);
                            sb.Insert(position++, e.Text[i]);
                        }
                        else if (position == 8 || position == 9 || position == 10)
                        {
                            char tens = (position == 8 || position == 9) ? e.Text[i] : sb[9];
                            char ones = position == 10 ? e.Text[i] : (e.Text.Length > i + 1) ? e.Text[i + 1] : sb[10];
                            if (IsValidFrames(tens, ones))
                            {
                                if (position == 8)
                                    position++;
                                if (position == 9 || position == 10)
                                {
                                    sb.Remove(9, 2);
                                    sb.Insert(9, tens);
                                    sb.Insert(10, ones);
                                    position++;
                                }
                                if (e.Text.Length > i + 1)
                                {
                                    i++;
                                    position++;
                                }
                            }
                            else
                            {
                                e.Handled = true;
                                return;
                            }
                        }
                        else
                        {
                            e.Handled = true;
                            return;
                        }
                    }
                    Text = sb.ToString();
                    SelectionStart = position;
                }
            }

            e.Handled = true;
            base.OnPreviewTextInput(e);
        }

        /// <summary>
        /// override the key down to handle delete of a character
        /// </summary>
        protected override void OnPreviewKeyDown(KeyEventArgs e)
        {
            base.OnPreviewKeyDown(e);
            int position = SelectionStart;
            int selectionLength = SelectionLength;

            if (e.Key == Key.Delete && position < Text.Length)//handle the delete key
            {
                if (selectionLength == 0)
                    selectionLength = 1;
                var text = new StringBuilder(Text);
                text.Remove(position, selectionLength);
                text.Insert(position, Zero().Substring(position, selectionLength));
                Text = text.ToString();
                SelectionStart = position;
                SelectionLength = 0;
                e.Handled = true;
            }

            else if (e.Key == Key.Back && (position > 0 || selectionLength > 0))//handle the back space
            {
                if (selectionLength == 0)
                {
                    position--;
                    selectionLength = 1;
                }
                var text = new StringBuilder(Text);
                text.Remove(position, selectionLength);
                text.Insert(position, Zero().Substring(position, selectionLength));
                Text = text.ToString();
                if (position == 3 || position == 6 || position == 9)
                    position--; // omit separators
                SelectionStart = position;
                SelectionLength = 0;
                e.Handled = true;
            }
        }

        /// <summary>
        /// Determines if a char can be entered at hours, minutes and seconds place
        /// </summary>
        private bool IsValidCharAt(char c, int position)
        {
            switch (position)
            {
                case 0:
                case 1:
                case 4:
                case 7:
                    return char.IsDigit(c);
                case 2:
                case 5:
                    return c == ':';
                case 3:
                case 6:
                    return c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5';
                case 8:
                    return IsDropFrame ? c == ';' : c == ':';
                default:
                    return false;
            }
        }

        /// <summary>
        /// Determines if a char pair can be entered at frames (9-10) place
        /// </summary>
        private bool IsValidFrames(char tens, char ones)
        {
            if (!(char.IsDigit(tens) && char.IsDigit(ones)))
                return false;
            int frames = (tens - '0') * 10 + (ones - '0');
            return frames < VideoFormat.TimeToFrameNumber(TimeSpan.FromTicks(TimeSpan.TicksPerSecond));
        }

        private string Zero()
        {
            return VideoFormat.FrameNumberToString(0, IsDropFrame);
        }
    }
}
