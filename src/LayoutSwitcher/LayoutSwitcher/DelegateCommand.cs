using System;
using System.Windows;
using System.Windows.Input;

namespace LayoutSwitcher
{
    public class DelegateCommand: ICommand
    {
        public Action<object> CommandAction { get; set; }
        public Func<object, bool> Predicate { get; set; }

        public DelegateCommand():this(null, null) { }
        public DelegateCommand(Action<object> action):this(action, null) { }
        public DelegateCommand(Action<object> action, Func<object, bool> predicate)
        {
            CommandAction = action;
            Predicate = predicate;
        }

        public void Execute(object parameter)
        {
            CommandAction(parameter);
        }

        public bool CanExecute(object parameter)
        {
            return Predicate == null  || Predicate(parameter);
        }

        public event EventHandler CanExecuteChanged
        {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }
    }
}
