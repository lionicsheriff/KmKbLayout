using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Win32;
using System.ComponentModel;

namespace LayoutSwitcher
{
    public class KeyboardConfig: IDisposable, INotifyPropertyChanged
    {
        public IEnumerable<string> Layouts
        {
            get
            {
                return _key.OpenSubKey("Layouts").GetSubKeyNames();
            }
        }

        public string SelectedLayout
        {
            get
            {
                return _key.OpenSubKey("Layouts").GetValue("Selected Layout") as string;
            }

            set
            {
                _key.OpenSubKey("Layouts", writable: true).SetValue("Selected Layout", value);
            }
        }

        private RegistryKey _key;
        public KeyboardConfig(string path)
        {
            _key = Registry.LocalMachine.OpenSubKey(path);

        }



        public void Dispose()
        {
            if (_key != null)
            {
                _key.Close();
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        public void NotifyChange(PropertyChangedEventArgs e)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, e);
            }
        }
    }
}
