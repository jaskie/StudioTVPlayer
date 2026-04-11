using System;
using System.Diagnostics;
using System.Windows.Input;

namespace StudioTVPlayer.Helpers
{
    public class UiCommand(Action<object> execute, Predicate<object> canExecute = null) : ICommand
    {
        private readonly Action<object> _execute = execute ?? throw new ArgumentNullException(nameof(execute));
        private readonly Predicate<object> _canExecute = canExecute;

        [DebuggerStepThrough]
        public bool CanExecute(object parameter) => _canExecute?.Invoke(parameter) ?? true;

        public void Execute(object parameter) => _execute(parameter);

        public event EventHandler CanExecuteChanged
        {
            add => CommandManager.RequerySuggested += value;
            remove => CommandManager.RequerySuggested -= value;
        }

    }
}
