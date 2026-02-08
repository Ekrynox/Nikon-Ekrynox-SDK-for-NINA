using CommunityToolkit.Mvvm.Input;
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
using System.ComponentModel.Composition;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using System.Windows.Threading;

namespace LucasAlias.NINA.NEK.Dockables {
    [Export(typeof(IDockableVM))]
    class NikonMtpDockableNek : DockableVM, ICameraConsumer {
        public const string sourceFile = @"NEKDockables\NikonMtpDockableNek.cs";

        private NikonCameraNek cameraNek { get => this.cameraMediator.GetDevice() != null && this.cameraMediator.GetDevice() is NikonCameraNek cam && cam.Connected ? cam : null; }
        private ObservableCollection<KeyValuePair<NEKCS.NikonMtpDevicePropCode, NEKCS.NikonDevicePropDescDS_Variant>> _deviceProperties;
        public ICommand UpdateDevicePropertyCommand { get; }

        private readonly ICameraMediator cameraMediator;

        private Boolean _connected = false;


        [ImportingConstructor]
        public NikonMtpDockableNek(IProfileService profileService, ICameraMediator cameraMediator) : base(profileService) {
            this.cameraMediator = cameraMediator;
            this.cameraMediator.RegisterConsumer(this);
            this.cameraMediator.Connected += CameraConnected;
            this.cameraMediator.Disconnected += CameraDisconnected;

            this.Title = "Nikon MTP Settings (NEK)";

            _deviceProperties = new();
            UpdateDevicePropertyCommand = new AsyncRelayCommand<KeyValuePair<NikonMtpDevicePropCode, NikonDevicePropDescDS_Variant>>(UpdateDeviceProperty);
        }

        public void Dispose() {
            this.cameraMediator.RemoveConsumer(this);
            this.cameraMediator.Connected -= CameraConnected;
            this.cameraMediator.Disconnected -= CameraDisconnected;
        }



        public Boolean Connected { get => _connected && cameraNek != null; }
        public ObservableCollection<KeyValuePair<NEKCS.NikonMtpDevicePropCode, NEKCS.NikonDevicePropDescDS_Variant>> DeviceProperties { get => _deviceProperties; }
        public void UpdateDeviceInfo(CameraInfo deviceInfo) {}

        private async Task CameraConnected(object arg1, EventArgs arg2) {
            if (this.cameraNek != null) {
                //Notify UI immediately
                this._connected = true;
                RaiseAllPropertiesChanged();

                //Fill the array in background
                _ = Task.Run(() => {
                    var newDeviceProps = new List<KeyValuePair<NEKCS.NikonMtpDevicePropCode, NEKCS.NikonDevicePropDescDS_Variant>>();

                    foreach (var k in this.cameraNek.cameraInfo.DevicePropertiesSupported) {
                        if (this.cameraNek == null) return;
                        try {
                            newDeviceProps.Add(new KeyValuePair<NEKCS.NikonMtpDevicePropCode, NEKCS.NikonDevicePropDescDS_Variant>(k, this.cameraNek.camera.GetDevicePropDesc(k)));
                        } catch (MtpDeviceException e) {
                            Logger.Error("Error while trying to get Device Property Description: " + k.ToString(), e, sourceFile);
                        } catch (MtpException e) {
                            Logger.Error("Error while trying to get Device Property Description: " + k.ToString(), e, sourceFile);
                        }
                    }

                    //Copy the Result in the ObservableContainer in the DispatcherThread
                    System.Windows.Application.Current.Dispatcher.Invoke(() => {
                        this._deviceProperties = new(newDeviceProps);
                        RaisePropertyChanged(nameof(DeviceProperties));
                    });
                });
            }
        }

        private async Task CameraDisconnected(object arg1, EventArgs arg2) {
            if (this._connected) {
                this._deviceProperties.Clear();

                RaiseAllPropertiesChanged();
            }
            this._connected = false;
        }


        private async Task UpdateDeviceProperty(KeyValuePair<NikonMtpDevicePropCode, NikonDevicePropDescDS_Variant> kvp) {
            Notification.ShowSuccess("LostFocus");
        }


        }
    }
