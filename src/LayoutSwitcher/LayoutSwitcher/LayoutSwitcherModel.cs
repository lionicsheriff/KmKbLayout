using System;
using System.Windows;
using System.Windows.Input;
using System.ComponentModel;
using LayoutSwitcher;
using Settings = LayoutSwitcher.Properties.Settings;

namespace LayoutSwitcher
{
    public class LayoutSwitcherModel: IDisposable, INotifyPropertyChanged
    {
        private ICommand _exitCommand;
        public ICommand ExitCommand
        {
            get
            {
                return _exitCommand ?? (_exitCommand = new DelegateCommand { CommandAction = o => Application.Current.Shutdown() });
            }
        }

        public ICommand ToggleMainWindowCommand
        {
            get
            {
                return new DelegateCommand {
                    CommandAction = o => {
                        if  (Application.Current.MainWindow.Visibility != Visibility.Visible)
                        {
                            Application.Current.MainWindow.Show();
                        }
                        else
                        {
                            Application.Current.MainWindow.Hide();
                        }
                    }
                };
            }
        }
        public ICommand CloseMainWindowCommand
        {
            get
            {
                return new DelegateCommand {
                    Predicate = o => Application.Current.MainWindow != null,
                    CommandAction = o => Application.Current.MainWindow.Close()
                };
            }
        }

        public KeyboardConfig Config { get; set; }

        public LayoutSwitcherModel()
        {

            Config = new KeyboardConfig(Settings.Default.RegistryKey);
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public void Dispose()
        {
            Config.Dispose();
        }
    }
}


