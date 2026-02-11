using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace LucasAlias.NINA.NEK.Dockables.Converters
{
    class StringConverter : IValueConverter {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture) {
            if (value is SByte sbyteValue) {
                return sbyteValue.ToString();
            } else if (value is Byte byteValue) {
                return byteValue.ToString();
            } else if (value is Int16 int16Value) {
                return int16Value.ToString();
            } else if (value is UInt16 uint16Value) {
                return uint16Value.ToString();
            } else if (value is Int32 int32Value) {
                return int32Value.ToString();
            } else if (value is UInt32 uint32Value) {
                return uint32Value.ToString();
            } else if (value is Int64 int64Value) {
                return int64Value.ToString();
            } else if (value is UInt64 uint64Value) {
                return uint64Value.ToString();
            } else if (value is SByte[] sbyteArrayValue) {
                return JsonSerializer.Serialize(sbyteArrayValue);
            } else if (value is Byte[] byteArrayValue) {
                return JsonSerializer.Serialize(byteArrayValue);
            } else if (value is Int16[] int16ArrayValue) {
                return JsonSerializer.Serialize(int16ArrayValue);
            } else if (value is UInt16[] uint16ArrayValue) {
                return JsonSerializer.Serialize(uint16ArrayValue);
            } else if (value is Int32[] int32ArrayValue) {
                return JsonSerializer.Serialize(int32ArrayValue);
            } else if (value is UInt32[] uint32ArrayValue) {
                return JsonSerializer.Serialize(uint32ArrayValue);
            } else if (value is Int64[] int64ArrayValue) {
                return JsonSerializer.Serialize(int64ArrayValue);
            } else if (value is UInt64[] uint64ArrayValue) {
                return JsonSerializer.Serialize(uint64ArrayValue);
            } else if (value is String stringValue) {
                return stringValue;
            }

            return value?.ToString();
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) {
            if (targetType == typeof(SByte)) {
                return SByte.Parse(value as String);
            } else if (targetType == typeof(Byte)) {
                return Byte.Parse(value as String);
            } else if (targetType == typeof(Int16)) {
                return Int16.Parse(value as String);
            } else if (targetType == typeof(UInt16)) {
                return UInt16.Parse(value as String);
            } else if (targetType == typeof(Int32)) {
                return Int32.Parse(value as String);
            } else if (targetType == typeof(UInt32)) {
                return UInt32.Parse(value as String);
            } else if (targetType == typeof(Int64)) {
                return Int64.Parse(value as String);
            } else if (targetType == typeof(UInt64)) {
                return UInt64.Parse(value as String);
            } else if (targetType == typeof(SByte[])) {
                return JsonSerializer.Deserialize<SByte[]>(value as String);
            } else if (targetType == typeof(Byte[])) {
                return JsonSerializer.Deserialize<Byte[]>(value as String);
            } else if (targetType == typeof(Int16[])) {
                return JsonSerializer.Deserialize<Int16[]>(value as String);
            } else if (targetType == typeof(UInt16[])) {
                return JsonSerializer.Deserialize<UInt16[]>(value as String);
            } else if (targetType == typeof(Int32[])) {
                return JsonSerializer.Deserialize<Int32[]>(value as String);
            } else if (targetType == typeof(UInt32[])) {
                return JsonSerializer.Deserialize<UInt32[]>(value as String);
            } else if (targetType == typeof(Int64[])) {
                return JsonSerializer.Deserialize<Int64[]>(value as String);
            } else if (targetType == typeof(UInt64[])) {
                return JsonSerializer.Deserialize<UInt64[]>(value as String);
            } else if (targetType == typeof(String)) {
                return value as String;
            }

            return null;
        }
    }
}
