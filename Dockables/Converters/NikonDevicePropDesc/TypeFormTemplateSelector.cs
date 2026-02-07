using Google.Protobuf.WellKnownTypes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace LucasAlias.NINA.NEK.Dockables.Converters.NikonDevicePropDesc
{
    class TypeFormTemplateSelector : DataTemplateSelector {
        public DataTemplate IntegerTemplate { get; set; }
        public DataTemplate StringTemplate { get; set; }
        public DataTemplate ArrayTemplate { get; set; }
        public DataTemplate EnumTemplate { get; set; }
        public DataTemplate RangeTemplate { get; set; }

        public override DataTemplate SelectTemplate(object item, DependencyObject container) {
            if (item is NEKCS.NikonDevicePropDescDS_Variant desc) {
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
                            return IntegerTemplate;
                        case NEKCS.NikonMtpDatatypeCode.Int128:
                        case NEKCS.NikonMtpDatatypeCode.UInt128:
                            throw new NotImplementedException("128-bit integers are not supported in NEK at the moment");
                        case NEKCS.NikonMtpDatatypeCode.String:
                            return StringTemplate;
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt64:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt64:
                            return ArrayTemplate;
                    }
                }
                else if (desc.FormFlag == NEKCS.NikonMtpFormtypeCode.Enum) {
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
                        case NEKCS.NikonMtpDatatypeCode.String:
                            return EnumTemplate;
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt64:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt64:
                            throw new NotImplementedException("Enum of array are not implemented in NEK at the moment");
                    }
                }
                else if (desc.FormFlag == NEKCS.NikonMtpFormtypeCode.Range) {
                    switch (desc.DataType) {
                        case NEKCS.NikonMtpDatatypeCode.Int8:
                        case NEKCS.NikonMtpDatatypeCode.UInt8:
                        case NEKCS.NikonMtpDatatypeCode.Int16:
                        case NEKCS.NikonMtpDatatypeCode.UInt16:
                        case NEKCS.NikonMtpDatatypeCode.Int32:
                        case NEKCS.NikonMtpDatatypeCode.UInt32:
                        case NEKCS.NikonMtpDatatypeCode.Int64:
                        case NEKCS.NikonMtpDatatypeCode.UInt64:
                            return RangeTemplate;
                        case NEKCS.NikonMtpDatatypeCode.Int128:
                        case NEKCS.NikonMtpDatatypeCode.UInt128:
                            throw new NotImplementedException("128-bit integers are not supported in NEK at the moment");
                        case NEKCS.NikonMtpDatatypeCode.String:
                            throw new NotImplementedException("Range of string are not implemented in NEK at the moment");
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt64:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt64:
                            throw new NotImplementedException("Range of array are not implemented in NEK at the moment");
                    }
                }
            }
            else if (item.GetType().IsGenericType && item.GetType().GetGenericTypeDefinition() == typeof(NEKCS.NikonDevicePropDescDS<>)) {
                dynamic desc2 = item;
                if (desc2.FormFlag == NEKCS.NikonMtpFormtypeCode.Empty) {
                    switch (desc2.DataType) {
                        case NEKCS.NikonMtpDatatypeCode.Int8:
                        case NEKCS.NikonMtpDatatypeCode.UInt8:
                        case NEKCS.NikonMtpDatatypeCode.Int16:
                        case NEKCS.NikonMtpDatatypeCode.UInt16:
                        case NEKCS.NikonMtpDatatypeCode.Int32:
                        case NEKCS.NikonMtpDatatypeCode.UInt32:
                        case NEKCS.NikonMtpDatatypeCode.Int64:
                        case NEKCS.NikonMtpDatatypeCode.UInt64:
                            return IntegerTemplate;
                        case NEKCS.NikonMtpDatatypeCode.Int128:
                        case NEKCS.NikonMtpDatatypeCode.UInt128:
                            throw new NotImplementedException("128-bit integers are not supported in NEK at the moment");
                        case NEKCS.NikonMtpDatatypeCode.String:
                            return StringTemplate;
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt64:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt64:
                            return ArrayTemplate;
                    }
                } else if (desc2.FormFlag == NEKCS.NikonMtpFormtypeCode.Enum) {
                    switch (desc2.DataType) {
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
                        case NEKCS.NikonMtpDatatypeCode.String:
                            return EnumTemplate;
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt64:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt64:
                            throw new NotImplementedException("Enum of array are not implemented in NEK at the moment");
                    }
                } else if (desc2.FormFlag == NEKCS.NikonMtpFormtypeCode.Range) {
                    switch (desc2.DataType) {
                        case NEKCS.NikonMtpDatatypeCode.Int8:
                        case NEKCS.NikonMtpDatatypeCode.UInt8:
                        case NEKCS.NikonMtpDatatypeCode.Int16:
                        case NEKCS.NikonMtpDatatypeCode.UInt16:
                        case NEKCS.NikonMtpDatatypeCode.Int32:
                        case NEKCS.NikonMtpDatatypeCode.UInt32:
                        case NEKCS.NikonMtpDatatypeCode.Int64:
                        case NEKCS.NikonMtpDatatypeCode.UInt64:
                            return RangeTemplate;
                        case NEKCS.NikonMtpDatatypeCode.Int128:
                        case NEKCS.NikonMtpDatatypeCode.UInt128:
                            throw new NotImplementedException("128-bit integers are not supported in NEK at the moment");
                        case NEKCS.NikonMtpDatatypeCode.String:
                            throw new NotImplementedException("Range of string are not implemented in NEK at the moment");
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt8:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt16:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt32:
                        case NEKCS.NikonMtpDatatypeCode.ArrayInt64:
                        case NEKCS.NikonMtpDatatypeCode.ArrayUInt64:
                            throw new NotImplementedException("Range of array are not implemented in NEK at the moment");
                    }
                }
            }

            return StringTemplate; // Fallback
        }
    }
}
