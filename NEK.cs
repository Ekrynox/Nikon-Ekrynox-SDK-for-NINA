using LucasAlias.NINA.NEK.Properties;
using NINA.Core.Model;
using NINA.Core.Utility;
using NINA.Image.ImageData;
using NINA.Plugin;
using NINA.Plugin.Interfaces;
using NINA.Profile;
using NINA.Profile.Interfaces;
using NINA.WPF.Base.Interfaces.Mediator;
using NINA.WPF.Base.Interfaces.ViewModel;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.ComponentModel.Composition;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using Settings = LucasAlias.NINA.NEK.Properties.Settings;

namespace LucasAlias.NINA.NEK {
    [Export(typeof(IPluginManifest))]
    public class NEK : PluginBase, INotifyPropertyChanged {
        private readonly IPluginOptionsAccessor pluginSettings;
        private readonly IProfileService profileService;

        [ImportingConstructor]
        public NEK(IProfileService profileService, IOptionsVM options, IMessageBroker messageBroker) {
            if (Settings.Default.UpdateSettings) {
                Settings.Default.Upgrade();
                Settings.Default.UpdateSettings = false;
                CoreUtil.SaveSettings(Settings.Default);
            }

            this.pluginSettings = new PluginOptionsAccessor(profileService, Guid.Parse(this.Identifier));
            this.profileService = profileService;
            profileService.ProfileChanged += ProfileService_ProfileChanged;

            NEKMediator.InitMediator(this, messageBroker);
        }

        public override Task Teardown() {
            profileService.ProfileChanged -= ProfileService_ProfileChanged;

            return base.Teardown();
        }

        public event PropertyChangedEventHandler PropertyChanged;
        protected void RaisePropertyChanged([CallerMemberName] string propertyName = null) {
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        private void ProfileService_ProfileChanged(object sender, EventArgs e) {
        }


        //Plugin Settings
        public bool UpdateLensInfo {
            get {
                return pluginSettings.GetValueBoolean(nameof(UpdateLensInfo), true);
            }
            set {
                pluginSettings.SetValueBoolean(nameof(UpdateLensInfo), value);
                RaisePropertyChanged();
            }
        }
        public byte NbAdditionalThreads {
            get {
                return Settings.Default.NbAdditionalThreads;
            }
            set {
                Settings.Default.NbAdditionalThreads = value;
                CoreUtil.SaveSettings(Settings.Default);
                RaisePropertyChanged();
            }
        }
    }
}
