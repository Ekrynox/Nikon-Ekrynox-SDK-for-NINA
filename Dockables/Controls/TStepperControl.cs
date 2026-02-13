using NINA.CustomControlLibrary;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace LucasAlias.NINA.NEK.Dockables.Controls {
    [TemplatePart(Name = "PART_Textbox", Type = typeof(UnitTextBox))]
    [TemplatePart(Name = "PART_Decrement", Type = typeof(Button))]
    [TemplatePart(Name = "PART_Increment", Type = typeof(Button))]
    public abstract class StepperControlBase : UserControl {

        public static readonly DependencyProperty ButtonForegroundBrushProperty =
           DependencyProperty.Register(nameof(ButtonForegroundBrush), typeof(Brush), typeof(StepperControlBase), new UIPropertyMetadata(new SolidColorBrush(Colors.White)));

        public Brush ButtonForegroundBrush {
            get => (Brush)GetValue(ButtonForegroundBrushProperty);
            set => SetValue(ButtonForegroundBrushProperty, value);
        }

        public static readonly DependencyProperty AddSVGProperty =
           DependencyProperty.Register(nameof(AddSVG), typeof(Geometry), typeof(StepperControlBase), new UIPropertyMetadata(null));

        public Geometry AddSVG {
            get => (Geometry)GetValue(AddSVGProperty);
            set => SetValue(AddSVGProperty, value);
        }

        public static readonly DependencyProperty SubstractSVGProperty =
           DependencyProperty.Register(nameof(SubstractSVG), typeof(Geometry), typeof(StepperControlBase), new UIPropertyMetadata(null));

        public Geometry SubstractSVG {
            get => (Geometry)GetValue(SubstractSVGProperty);
            set => SetValue(SubstractSVGProperty, value);
        }
    }

    public abstract class TStepperControl<T> : StepperControlBase
        where T: INumber<T>, IMinMaxValue<T> {

        public static readonly DependencyProperty ValueProperty =
           DependencyProperty.Register(nameof(Value), typeof(T), typeof(TStepperControl<T>), new UIPropertyMetadata(T.Zero));

        public T Value {
            get => (T)GetValue(ValueProperty);
            set => SetValue(ValueProperty, value);
        }

        public static readonly DependencyProperty MinValueProperty =
           DependencyProperty.Register(nameof(MinValue), typeof(T), typeof(TStepperControl<T>), new UIPropertyMetadata(T.MinValue));

        public T MinValue {
            get => (T)GetValue(MinValueProperty);
            set => SetValue(MinValueProperty, value);
        }

        public static readonly DependencyProperty MaxValueProperty =
           DependencyProperty.Register(nameof(MaxValue), typeof(T), typeof(TStepperControl<T>), new UIPropertyMetadata(T.MaxValue));

        public T MaxValue {
            get => (T)GetValue(MaxValueProperty);
            set => SetValue(MaxValueProperty, value);
        }

        public static readonly DependencyProperty StepSizeProperty =
           DependencyProperty.Register(nameof(StepSize), typeof(T), typeof(TStepperControl<T>), new UIPropertyMetadata(T.One));

        public T StepSize {
            get => (T)GetValue(StepSizeProperty);
            set => SetValue(StepSizeProperty, value);
        }

        public static readonly DependencyProperty UnitProperty =
           DependencyProperty.Register(nameof(Unit), typeof(string), typeof(TStepperControl<T>), new UIPropertyMetadata(string.Empty));

        public string Unit {
            get => (string)GetValue(UnitProperty);
            set => SetValue(UnitProperty, value);
        }

        public override void OnApplyTemplate() {
            base.OnApplyTemplate();
            var button = GetTemplateChild("PART_Increment") as Button;
            if (button != null) {
                button.Click += Button_PART_Increment_Click;
            }

            button = GetTemplateChild("PART_Decrement") as Button;
            if (button != null) {
                button.Click += Button_PART_Decrement_Click;
            }

            var tb = GetTemplateChild("PART_Textbox") as UnitTextBox;
            if (tb != null) {
                tb.LostFocus += PART_TextBox_LostFocus;
            }
        }

        private void Button_PART_Increment_Click(object sender, RoutedEventArgs e) {
            if (Value + StepSize <= MaxValue) {
                Value += StepSize;
            }
        }

        private void Button_PART_Decrement_Click(object sender, RoutedEventArgs e) {
            if (Value - StepSize >= MinValue) {
                Value -= StepSize;
            }
        }

        private void PART_TextBox_LostFocus(object sender, RoutedEventArgs e) {
            if (Value < MinValue) {
                Value = MinValue;
            }
            if (Value > MaxValue) {
                Value = MaxValue;
            }
        }
    }


    public class Int64StepperControl : TStepperControl<Int64> {
        static Int64StepperControl() {
            DefaultStyleKeyProperty.OverrideMetadata(typeof(Int64StepperControl), new FrameworkPropertyMetadata(typeof(Int64StepperControl)));
        }
    }
    public class UInt64StepperControl : TStepperControl<UInt64> {
        static UInt64StepperControl() {
            DefaultStyleKeyProperty.OverrideMetadata(typeof(UInt64StepperControl), new FrameworkPropertyMetadata(typeof(UInt64StepperControl)));
        }
    }
}
