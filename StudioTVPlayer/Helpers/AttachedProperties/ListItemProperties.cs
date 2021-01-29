using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using StudioTVPlayer.ViewModel.Main.MediaBrowser;
using StudioTVPlayer.ViewModel.Main.Player;

namespace StudioTVPlayer.Helpers.AttachedProperties
{
    public class ListItemProperties
    {
        public static readonly DependencyProperty ItemSelectedProperty =
           DependencyProperty.RegisterAttached(
               "ItemSelected",
               typeof(MediaViewModel),
               typeof(ListItemProperties),
               new FrameworkPropertyMetadata(ItemSelectedChanged));

        public static readonly DependencyProperty DragEnterProperty =
           DependencyProperty.RegisterAttached(
               "DragEnter",
               typeof(bool),
               typeof(ListItemProperties),
               new FrameworkPropertyMetadata(true, DragEnterChanged));

        public static readonly DependencyProperty DragLeaveProperty =
           DependencyProperty.RegisterAttached(
               "DragLeave",
               typeof(bool),
               typeof(ListItemProperties),
               new FrameworkPropertyMetadata(true, DragLeaveChanged));

        public static readonly DependencyProperty IsDroppedProperty =
            DependencyProperty.RegisterAttached(
                "IsDropped",
                typeof(bool),
                typeof(ListItemProperties),
                new FrameworkPropertyMetadata(false, IsDroppedChanged));


        private static void IsDroppedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var element = (FrameworkElement)d;
            element.Drop += IsDropped;
        }

        private static void IsDropped(object sender, DragEventArgs e)
        {
            if (sender is FrameworkElement element)
                SetIsDropped(element, false);
            e.Handled = true;
        }

        public static void SetIsDropped(UIElement element, bool value) => element.SetValue(IsDroppedProperty, value);
        public static bool GetIsDropped(UIElement element) => (bool)element.GetValue(IsDroppedProperty);


    
        private static void DragEnterChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var element = (FrameworkElement)d;
            element.DragEnter += DragEnter_Occurs;
        }

        private static void DragEnter_Occurs(object sender, DragEventArgs e)
        {
            if (sender is FrameworkElement element && (RundownItemViewModel)element.DataContext != e.Data.GetData(typeof(RundownItemViewModel)))
                SetDragEnter(element, true);
            e.Handled = true;
        }

        public static void SetDragEnter(UIElement element, bool value) => element.SetValue(DragEnterProperty, value);
        public static bool GetDragEnter(UIElement element) => (bool)element.GetValue(DragEnterProperty);    

        private static void DragLeaveChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var element = (FrameworkElement)d;
            element.DragLeave += DragLeave_Occurs;
        }

        private static void DragLeave_Occurs(object sender, DragEventArgs e)
        {
            if (sender is FrameworkElement element)
                SetDragLeave(element, false);
            e.Handled = true;
        }

        public static void SetDragLeave(UIElement element, bool value) => element.SetValue(DragLeaveProperty, value);
        public static bool GetDragLeave(UIElement element) => (bool)element.GetValue(DragLeaveProperty);


       

        private static void ItemSelectedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var element = (FrameworkElement)d;
            element.PreviewMouseLeftButtonDown += ItemSelect;
            element.PreviewMouseRightButtonDown += ItemSelect;
            element.GotFocus += ItemSelectByFocus;
            element.MouseEnter += SelectItemOnce;
        }

        private static void SelectItemOnce(object sender, MouseEventArgs e)
        {
            ((Control)sender).Focus();
        }

        private static void ItemSelectByFocus(object sender, RoutedEventArgs e)
        {
            if (sender is Control element)
                SetItemSelected(element, (MediaViewModel)element.DataContext);
            e.Handled = true;
        }

        private static void ItemSelect(object sender, MouseButtonEventArgs e)
        {
            if (sender is Control element)
                SetItemSelected(element, (MediaViewModel)element.DataContext);
            e.Handled = true;
        }

        public static void SetItemSelected(UIElement element, MediaViewModel value) => element.SetValue(ItemSelectedProperty, value);
        public static MediaViewModel GetItemSelected(UIElement element) => (MediaViewModel)element.GetValue(ItemSelectedProperty);


    }
}
