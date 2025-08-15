using NINA.Core.Utility;
using NINA.Equipment.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace LucasAlias.NINA.NEK.NEKDrivers {
    internal class NikonFocuserNek : BaseINPC, IFocuser {

        public NikonFocuserNek(NikonCameraNek cameraNek) {
            this.cameraNek = cameraNek;
        }

        NikonCameraNek cameraNek;



        public bool HasSetupDialog { get => false; }
        public string Id { get => cameraNek.Id + "LF"; }
        public string Name { get => cameraNek.Name; }
        public string DisplayName { get => cameraNek.Name + " Lens (NEK Experimental)"; }
        public string Category { get => "Nikon"; }
        public bool Connected { get => true; } //TODO
        public string Description { get => "The lens focus driver of your Nikon Camera !"; }
        public string DriverInfo { get => "Nikon Ekrynox SDK"; }
        public string DriverVersion { get => cameraNek.DriverVersion; }


        public Task<bool> Connect(CancellationToken token) { return Task.Run(() => { return true; }); } //TODO

        public void Disconnect() => throw new NotImplementedException();

        public void SetupDialog() => throw new NotImplementedException();

        public IList<string> SupportedActions { get => new List<string>(); }

        public string Action(string actionName, string actionParameters) => throw new NotImplementedException();

        public string SendCommandString(string command, bool raw = true) => throw new NotImplementedException();

        public bool SendCommandBool(string command, bool raw = true) => throw new NotImplementedException();

        public void SendCommandBlind(string command, bool raw = true) => throw new NotImplementedException();



        public bool IsMoving { get => false; } //TODO
        public int MaxIncrement { get => 0; } //TODO
        public int MaxStep { get => 0; } //TODO
        public int Position { get => 0; } //TODO
        public double StepSize { get => 0; } //TODO
        public bool TempCompAvailable { get => false; } //TOCHECK
        public bool TempComp { get => false; set {} } //TOCHECK
        public double Temperature { get => double.NaN; } //TOCHECK

        public Task Move(int position, CancellationToken ct, int waitInMs = 1000) => throw new NotImplementedException();

        public void Halt() => throw new NotImplementedException();

    }
}
