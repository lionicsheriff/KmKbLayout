using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;

using Hardcodet.Wpf.TaskbarNotification;

namespace LayoutSwitcher
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private TaskbarIcon _taskbarIcon;
        private void Application_Startup(object sender, StartupEventArgs e)
        {
            _taskbarIcon = (TaskbarIcon)FindResource("TaskbarIcon");
            _taskbarIcon.PreviewTrayToolTipOpen += _taskbarIcon_PreviewTrayToolTipOpen;
            Application.Current.MainWindow = new MainWindow();
        }

        void _taskbarIcon_PreviewTrayToolTipOpen(object sender, RoutedEventArgs e)
        {
            // bit brutal, but until I get a real registry monitor running: it's easier to just
            // raise the property change event for all registry properties, and trigger refreshing
            // all the bindings
            var model = (LayoutSwitcherModel)_taskbarIcon.DataContext;
            model.Config.NotifyChange(new System.ComponentModel.PropertyChangedEventArgs(""));
        }

    }
}
