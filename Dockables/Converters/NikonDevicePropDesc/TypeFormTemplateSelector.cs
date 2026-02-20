using Google.Protobuf.WellKnownTypes;
using LucasAlias.NINA.NEK.Dockables.VM;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace LucasAlias.NINA.NEK.Dockables.Converters.NikonDevicePropDesc {
    class TypeFormTemplateSelector : DataTemplateSelector {
        public DataTemplate StringTemplate { get; set; }
        public DataTemplate EnumTemplate { get; set; }
        public DataTemplate IntRangeTemplate { get; set; }
        public DataTemplate UIntRangeTemplate { get; set; }

        public override DataTemplate SelectTemplate(object item, DependencyObject container) {
            if (item == null) return null;

            if (item is INikonDevicePropDescVM desc) {
                if (desc.FormFlag == NEKCS.NikonMtpFormtypeCode.Empty) {
                    switch (desc.DataType) {
                        case NEKCS.NikonMtpDatatypeCode.Int8:
                        case NEKCS.NikonMtpDatatypeCode.UInt8:
                        case NEKCS.NikonMtpDatatypeCode.Int16:
                        case NEKCS.NikonMtpDatatypeCode.UInt16:
                        case NEKCS.NikonMtpDatatypeCode.Int32:
                        case NEKCS.NikonMtpDatatypeCode.UInt32:
                        case NEKCS.NikonMtpDatatypeCode.Int64:
                        case NEKCS.NikonMtpDatatypeCode.UInt64:
                            return StringTemplate;
                        case NEKCS.NikonMtpDatatypeCode.Int128:
                        case NEKCS.NikonMtpDatatypeCode.UInt128:
                            throw new NotImplementedException("128-bit integers are not supported in NEK at the moment");
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt64:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt64:
                            return StringTemplate;
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt128:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt128:
                            throw new NotImplementedException("128-bit integers are not supported in NEK at the moment");
                        case NEKCS.NikonMtpDatatypeCode.String:
                            return StringTemplate;
                    }
                } else if (desc.FormFlag == NEKCS.NikonMtpFormtypeCode.Enum) {
                    switch (desc.DataType) {
                        case NEKCS.NikonMtpDatatypeCode.Int8:
                        case NEKCS.NikonMtpDatatypeCode.UInt8:
                        case NEKCS.NikonMtpDatatypeCode.Int16:
                        case NEKCS.NikonMtpDatatypeCode.UInt16:
                        case NEKCS.NikonMtpDatatypeCode.Int32:
                        case NEKCS.NikonMtpDatatypeCode.UInt32:
                        case NEKCS.NikonMtpDatatypeCode.Int64:
                        case NEKCS.NikonMtpDatatypeCode.UInt64:
                            return EnumTemplate;
                        case NEKCS.NikonMtpDatatypeCode.Int128:
                        case NEKCS.NikonMtpDatatypeCode.UInt128:
                            throw new NotImplementedException("128-bit integers are not supported in NEK at the moment");
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt64:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt64:
                            throw new NotImplementedException("Enum of array are not implemented in NEK at the moment");
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt128:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt128:
                            throw new NotImplementedException("128-bit integers are not supported in NEK at the moment");
                        case NEKCS.NikonMtpDatatypeCode.String:
                            return EnumTemplate;
                    }
                } else if (desc.FormFlag == NEKCS.NikonMtpFormtypeCode.Range) {
                    switch (desc.DataType) {
                        case NEKCS.NikonMtpDatatypeCode.Int8:
                        case NEKCS.NikonMtpDatatypeCode.Int16:
                        case NEKCS.NikonMtpDatatypeCode.Int32:
                        case NEKCS.NikonMtpDatatypeCode.Int64:
                            return IntRangeTemplate;
                        case NEKCS.NikonMtpDatatypeCode.UInt8:
                        case NEKCS.NikonMtpDatatypeCode.UInt16:
                        case NEKCS.NikonMtpDatatypeCode.UInt32:
                        case NEKCS.NikonMtpDatatypeCode.UInt64:
                            return UIntRangeTemplate;
                        case NEKCS.NikonMtpDatatypeCode.Int128:
                        case NEKCS.NikonMtpDatatypeCode.UInt128:
                            throw new NotImplementedException("128-bit integers are not supported in NEK at the moment");
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt64:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt64:
                            return StringTemplate; // Range of array is not implemented in NEK at the moment, but we can still show it as a string
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt128:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt128:
                            throw new NotImplementedException("128-bit integers are not supported in NEK at the moment");
                        case NEKCS.NikonMtpDatatypeCode.String:
                            return StringTemplate; // Range of string is not implemented in NEK at the moment, but we can still show it as a string
                    }
                }
            }

            return null;
        }
    }
}
