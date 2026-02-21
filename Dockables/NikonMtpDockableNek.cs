using CommunityToolkit.Mvvm.Input;
using LucasAlias.NINA.NEK.Dockables.VM;
using LucasAlias.NINA.NEK.Drivers;
using NEKCS;
using NINA.Core.Utility;
using NINA.Core.Utility.Notification;
using NINA.Equipment.Equipment.MyCamera;
using NINA.Equipment.Interfaces.Mediator;
using NINA.Equipment.Interfaces.ViewModel;
using NINA.Profile.Interfaces;
using NINA.WPF.Base.ViewModel;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.ComponentModel.Composition;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Threading;
using Xceed.Wpf.Toolkit.PropertyGrid.Attributes;

namespace LucasAlias.NINA.NEK.Dockables {
    [Export(typeof(IDockableVM))]
    class NikonMtpDockableNek : DockableVM {
        public const string sourceFile = @"NEKDockables\NikonMtpDockableNek.cs";

        private NikonCameraNek cameraNek { get => this.cameraMediator.GetDevice() != null && this.cameraMediator.GetDevice() is NikonCameraNek cam && cam.Connected ? cam : null; }

        private readonly ICameraMediator cameraMediator;

        private Boolean _connected = false;


        [ImportingConstructor]
        public NikonMtpDockableNek(IProfileService profileService, ICameraMediator cameraMediator) : base(profileService) {
            this.cameraMediator = cameraMediator;
            this.cameraMediator.Connected += CameraConnected;
            this.cameraMediator.Disconnected += CameraDisconnected;

            this.Title = "Nikon MTP Settings (NEK)";


            Application.Current.Dispatcher.Invoke(() => {
                DeviceProperties = new ObservableCollection<INikonDevicePropDescVM>();
                DevicePropertiesView = CollectionViewSource.GetDefaultView(DeviceProperties);
                //DevicePropertiesView.Filter = e => true;
            });

        }

        public void Dispose() {
            this.cameraMediator.Connected -= CameraConnected;
            this.cameraMediator.Disconnected -= CameraDisconnected;
        }



        public Boolean Connected { get => _connected && cameraNek != null; }
        public ObservableCollection<INikonDevicePropDescVM> DeviceProperties { get; private set; }
        public ICollectionView DevicePropertiesView { get; private set; }

        private async Task CameraConnected(object arg1, EventArgs arg2) {
            if (this.cameraNek != null) {
                //Notify UI immediately
                this._connected = true;
                RaiseAllPropertiesChanged();

                //Fill the array in background
                _ = Task.Run(() => {
                    var newDeviceProps = new List<INikonDevicePropDescVM>();

                    foreach (var k in this.cameraNek.cameraInfo.DevicePropertiesSupported) {
                        if (this.cameraNek == null) return;
                        try {
                            var desc = INikonDevicePropDescVM.Create(this.cameraNek.camera.GetDevicePropDesc(k));
                            desc.ValueChanged += SetDeviceProperty;
                            newDeviceProps.Add(desc);
                        } catch (NEKCS.MtpDeviceException e) {
                            Logger.Error("Error while trying to get Device Property Description: " + k.ToString(), e, sourceFile);
                        } catch (NEKCS.MtpException e) {
                            Logger.Error("Error while trying to get Device Property Description: " + k.ToString(), e, sourceFile);
                        }
                    }

                    //Copy the Result in the ObservableContainer in the DispatcherThread
                    Application.Current.Dispatcher.BeginInvoke(() => {
                        DeviceProperties.Clear();
                        foreach (var item in newDeviceProps) DeviceProperties.Add(item);
                        RaisePropertyChanged(nameof(DeviceProperties));

                        if (Connected) {
                            this.cameraNek.camera.OnMtpEvent += UpdateDeviceProperties;
                        }
                    });
                });
            }
        }

        private async Task CameraDisconnected(object arg1, EventArgs arg2) {
            if (this._connected) {
                if (this.cameraNek != null) {
                    this.cameraNek.camera.OnMtpEvent -= UpdateDeviceProperties;
                }
                //Notify UI immediately
                this._connected = false;
                RaiseAllPropertiesChanged();

                await Application.Current.Dispatcher.BeginInvoke(() => {
                    DeviceProperties.Clear();
                });
            }
        }


        private void UpdateDeviceProperties(NEKCS.NikonCamera camera, MtpEvent ev) {
            if (!Connected) return;
            if (ev.eventCode != NEKCS.NikonMtpEventCode.DevicePropChanged) return;

            var code = (NEKCS.NikonMtpDevicePropCode)ev.eventParams[0];
            UpdateDeviceProperty(code);
        }
        
        private void UpdateDeviceProperty(NEKCS.NikonMtpDevicePropCode code) {
            try {
                var desc = INikonDevicePropDescVM.Create(this.cameraNek.camera.GetDevicePropDesc(code));
                desc.ValueChanged += SetDeviceProperty;
                Application.Current.Dispatcher.BeginInvoke(() => {
                    if (!Connected) return;
                    try {
                        var i = DeviceProperties.Select((item, idx) => new { item, idx }).First(x => x.item.DevicePropertyCode == code).idx;
                        DeviceProperties[i] = desc;
                    } catch (InvalidOperationException) {
                        DeviceProperties.Add(desc);
                    }
                    RaisePropertyChanged(nameof(DeviceProperties));
                });
            } catch (NEKCS.MtpDeviceException e) {
                Logger.Error("Error while trying to get Device Property Description: " + code.ToString(), e, sourceFile);
            } catch (NEKCS.MtpException e) {
                Logger.Error("Error while trying to get Device Property Description: " + code.ToString(), e, sourceFile);
            }
        } 

        private async void SetDeviceProperty(INikonDevicePropDescVM desc) {
            if (!Connected) return;

            try {
                this.cameraNek.camera.SetDevicePropValueTypesafe(desc.DevicePropertyCode, desc.CurrentValueVariant);
            } catch (NEKCS.MtpDeviceException e) {
                Logger.Error($"Error while trying to Set Device Property: {desc.DevicePropertyCode.ToString()} to '{desc.CurrentValueUntyped.ToString()}'" , e, sourceFile);
                UpdateDeviceProperty(desc.DevicePropertyCode);
            } catch (NEKCS.MtpException e) {
                Logger.Error($"Error while trying to Set Device Property: {desc.DevicePropertyCode.ToString()} to '{desc.CurrentValueUntyped.ToString()}'", e, sourceFile);
                Notification.ShowError($"Failed to set {desc.DevicePropertyCode.ToString()} to: '{desc.CurrentValueUntyped.ToString()}'");
                UpdateDeviceProperty(desc.DevicePropertyCode);
            }
        }

    }
}
