using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace LucasAlias.NINA.NEK.Dockables.VM {
    internal interface INikonDevicePropDescVM : INotifyPropertyChanged {
        public static INikonDevicePropDescVM Create(NEKCS.NikonDevicePropDescDS_Variant devicePropDescDS) {
            INikonDevicePropDescVM o = devicePropDescDS.DataType switch {
                NEKCS.NikonMtpDatatypeCode.Int8 => devicePropDescDS.TryGetInt8(out var value) ? new NikonDevicePropDescVM<SByte>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.UInt8 => devicePropDescDS.TryGetUInt8(out var value) ? new NikonDevicePropDescVM<Byte>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.Int16 => devicePropDescDS.TryGetInt16(out var value) ? new NikonDevicePropDescVM<Int16>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.UInt16 => devicePropDescDS.TryGetUInt16(out var value) ? new NikonDevicePropDescVM<UInt16>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.Int32 => devicePropDescDS.TryGetInt32(out var value) ? new NikonDevicePropDescVM<Int32>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.UInt32 => devicePropDescDS.TryGetUInt32(out var value) ? new NikonDevicePropDescVM<UInt32>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.Int64 => devicePropDescDS.TryGetInt64(out var value) ? new NikonDevicePropDescVM<Int64>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.UInt64 => devicePropDescDS.TryGetUInt64(out var value) ? new NikonDevicePropDescVM<UInt64>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.Int128 => throw new NotImplementedException("128-bit integers are not supported in NEK at the moment"),
                NEKCS.NikonMtpDatatypeCode.UInt128 => throw new NotImplementedException("128-bit integers are not supported in NEK at the moment"),
                NEKCS.NikonMtpDatatypeCode.ArrayInt8 => devicePropDescDS.TryGetArrayInt8(out var value) ? new NikonDevicePropDescVM<SByte[]>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.ArrayUInt8 => devicePropDescDS.TryGetArrayUInt8(out var value) ? new NikonDevicePropDescVM<Byte[]>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.ArrayInt16 => devicePropDescDS.TryGetArrayInt16(out var value) ? new NikonDevicePropDescVM<Int16[]>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.ArrayUInt16 => devicePropDescDS.TryGetArrayUInt16(out var value) ? new NikonDevicePropDescVM<UInt16[]>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.ArrayInt32 => devicePropDescDS.TryGetArrayInt32(out var value) ? new NikonDevicePropDescVM<Int32[]>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.ArrayUInt32 => devicePropDescDS.TryGetArrayUInt32(out var value) ? new NikonDevicePropDescVM<UInt32[]>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.ArrayInt64 => devicePropDescDS.TryGetArrayInt64(out var value) ? new NikonDevicePropDescVM<Int64[]>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.ArrayUInt64 => devicePropDescDS.TryGetArrayUInt64(out var value) ? new NikonDevicePropDescVM<UInt64[]>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                NEKCS.NikonMtpDatatypeCode.ArrayInt128 => throw new NotImplementedException("128-bit integers are not supported in NEK at the moment"),
                NEKCS.NikonMtpDatatypeCode.ArrayUInt128 => throw new NotImplementedException("128-bit integers are not supported in NEK at the moment"),
                NEKCS.NikonMtpDatatypeCode.String => devicePropDescDS.TryGetString(out var value) ? new NikonDevicePropDescVM<String>(value) : throw new InvalidCastException("The Variant do not correspond to the Datatype."),
                _ => throw new InvalidCastException("Unkown Type")
            };

            return o;
        }

        public event Action<INikonDevicePropDescVM> ValueChanged;

        public NEKCS.NikonMtpDevicePropCode DevicePropertyCode { get; }
        public NEKCS.NikonMtpDatatypeCode DataType { get; }
        public Byte GetSet { get; }
        public object CurrentValueUntyped { get; }
        public NEKCS.MtpDatatypeVariant CurrentValueVariant { get; }
        public NEKCS.NikonMtpFormtypeCode FormFlag { get; }
    }

    internal class NikonDevicePropDescVM<T> : INikonDevicePropDescVM {
        private NEKCS.NikonDevicePropDescDS<T> _devicePropDescDS;
        public event Action<INikonDevicePropDescVM> ValueChanged;

        private NikonDevicePropDescVM() {}
        internal NikonDevicePropDescVM(NEKCS.NikonDevicePropDescDS<T> devicePropDescDS) {
            this._devicePropDescDS = devicePropDescDS;
        }

        public NEKCS.NikonMtpDevicePropCode DevicePropertyCode { get => _devicePropDescDS.DevicePropertyCode; }
        public NEKCS.NikonMtpDatatypeCode DataType { get => _devicePropDescDS.DataType; }
        public Byte GetSet { get => _devicePropDescDS.GetSet; }
        public T FactoryDefaultValue { get => _devicePropDescDS.FactoryDefaultValue; }
        public T CurrentValue {
            get => _devicePropDescDS.CurrentValue;
            set {
                _devicePropDescDS.CurrentValue = value;

                RaisePropertyChanged();
                ValueChanged?.Invoke(this);
            }
        }
        public object CurrentValueUntyped { get => _devicePropDescDS.CurrentValue; }
        public NEKCS.MtpDatatypeVariant CurrentValueVariant {
            get {
                if (typeof(T) == typeof(SByte)) return new NEKCS.MtpDatatypeVariant((SByte)(object)CurrentValue);
                else if (typeof(T) == typeof(Byte)) return new NEKCS.MtpDatatypeVariant((Byte)(object)CurrentValue);
                else if (typeof(T) == typeof(Int16)) return new NEKCS.MtpDatatypeVariant((Int16)(object)CurrentValue);
                else if (typeof(T) == typeof(UInt16)) return new NEKCS.MtpDatatypeVariant((UInt16)(object)CurrentValue);
                else if (typeof(T) == typeof(Int32)) return new NEKCS.MtpDatatypeVariant((Int32)(object)CurrentValue);
                else if (typeof(T) == typeof(UInt32)) return new NEKCS.MtpDatatypeVariant((UInt32)(object)CurrentValue);
                else if (typeof(T) == typeof(Int64)) return new NEKCS.MtpDatatypeVariant((Int64)(object)CurrentValue);
                else if (typeof(T) == typeof(UInt64)) return new NEKCS.MtpDatatypeVariant((UInt64)(object)CurrentValue);
                else if (typeof(T) == typeof(Int128)) throw new NotImplementedException("128-bit integers are not supported in NEK at the moment");
                else if (typeof(T) == typeof(UInt128)) throw new NotImplementedException("128-bit integers are not supported in NEK at the moment");
                else if (typeof(T) == typeof(SByte[])) return new NEKCS.MtpDatatypeVariant((SByte[])(object)CurrentValue);
                else if (typeof(T) == typeof(Byte[])) return new NEKCS.MtpDatatypeVariant((Byte[])(object)CurrentValue);
                else if (typeof(T) == typeof(Int16[])) return new NEKCS.MtpDatatypeVariant((Int16[])(object)CurrentValue);
                else if (typeof(T) == typeof(UInt16[])) return new NEKCS.MtpDatatypeVariant((UInt16[])(object)CurrentValue);
                else if (typeof(T) == typeof(Int32[])) return new NEKCS.MtpDatatypeVariant((Int32[])(object)CurrentValue);
                else if (typeof(T) == typeof(UInt32[])) return new NEKCS.MtpDatatypeVariant((UInt32[])(object)CurrentValue);
                else if (typeof(T) == typeof(Int64[])) return new NEKCS.MtpDatatypeVariant((Int64[])(object)CurrentValue);
                else if (typeof(T) == typeof(UInt64[])) return new NEKCS.MtpDatatypeVariant((UInt64[])(object)CurrentValue);
                else if (typeof(T) == typeof(Int128[])) throw new NotImplementedException("128-bit integers are not supported in NEK at the moment");
                else if (typeof(T) == typeof(UInt128[])) throw new NotImplementedException("128-bit integers are not supported in NEK at the moment");
                else if (typeof(T) == typeof(String)) return new NEKCS.MtpDatatypeVariant((String)(object)CurrentValue);

                throw new InvalidCastException("This type is not supported by the Variant.");
            }
        }
        public NEKCS.NikonMtpFormtypeCode FormFlag { get => _devicePropDescDS.FormFlag; }
        public NEKCS.NikonMtpRangeForm<T> RangeFORM { get => _devicePropDescDS.RangeFORM; }
        public T[] EnumFORM { get => _devicePropDescDS.EnumFORM; }


        public event PropertyChangedEventHandler PropertyChanged;
        protected void RaisePropertyChanged([CallerMemberName] string propertyName = null) {
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
