using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace LucasAlias.NINA.NEK.Dockables {
    [Export(typeof(ResourceDictionary))]
    public partial class NikonMtpDockableNekTemplates : ResourceDictionary {
        public NikonMtpDockableNekTemplates() {
            InitializeComponent();
        }
    }
}