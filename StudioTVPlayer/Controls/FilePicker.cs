// --------------------------------------------------------------------------------------------------------------------
// <copyright file="FilePicker.cs" company="PropertyTools">
//   Copyright (c) 2014 PropertyTools contributors
// </copyright>
// <summary>
//   Represents a control that allows the user to pick a file.
// </summary>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

using Microsoft.Win32;
using StudioTVPlayer.Helpers;

namespace StudioTVPlayer.Controls
{
    /// <summary>
    /// Represents a control that allows the user to pick a file.
    /// </summary>
    public class FilePicker : Control
    {
        /// <summary>
        /// Identifies the <see cref="BasePath"/> dependency property.
        /// </summary>
        public static readonly DependencyProperty BasePathProperty = DependencyProperty.Register(
            nameof(BasePath), typeof(string), typeof(FilePicker), new UIPropertyMetadata(null));

        /// <summary>
        /// Identifies the <see cref="DefaultExtension"/> dependency property.
        /// </summary>
        public static readonly DependencyProperty DefaultExtensionProperty = DependencyProperty.Register(
            nameof(DefaultExtension), typeof(string), typeof(FilePicker), new UIPropertyMetadata(null));

        /// <summary>
        /// Identifies the <see cref="Multiselect"/> dependency property.
        /// </summary>
        public static readonly DependencyProperty MultiselectProperty = DependencyProperty.Register(
            nameof(Multiselect), typeof(bool), typeof(FilePicker), new UIPropertyMetadata(false));

        /// <summary>
        /// Identifies the <see cref="FilePath"/> dependency property.
        /// </summary>
        public static readonly DependencyProperty FilePathProperty = DependencyProperty.Register(
            nameof(FilePath),
            typeof(string),
            typeof(FilePicker),
            new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));

        /// <summary>
        /// Identifies the <see cref="FilePaths"/> dependency property.
        /// </summary>
        public static readonly DependencyProperty FilePathsProperty = DependencyProperty.Register(
            nameof(FilePaths),
            typeof(string[]),
            typeof(FilePicker),
            new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, OnFilePathsChanged));

        /// <summary>
        /// Identifies the <see cref="Filter"/> dependency property.
        /// </summary>
        public static readonly DependencyProperty FilterProperty = DependencyProperty.Register(
            nameof(Filter),
            typeof(string),
            typeof(FilePicker),
            new UIPropertyMetadata(null));

        /// <summary>
        /// Identifies the <see cref="UseOpenDialog"/> dependency property.
        /// </summary>
        public static readonly DependencyProperty UseOpenDialogProperty = DependencyProperty.Register(
            nameof(UseOpenDialog),
            typeof(bool),
            typeof(FilePicker),
            new UIPropertyMetadata(true));

        /// <summary>
        /// Identifies the <see cref="BrowseButtonContent"/> dependency property.
        /// </summary>
        public static readonly DependencyProperty BrowseButtonContentProperty = DependencyProperty.Register(
            nameof(BrowseButtonContent),
            typeof(object),
            typeof(FilePicker),
            new PropertyMetadata("..."));

        /// <summary>
        /// Identifies the <see cref="ExploreButtonContent"/> dependency property.
        /// </summary>
        public static readonly DependencyProperty ExploreButtonContentProperty = DependencyProperty.Register(
            nameof(ExploreButtonContent),
            typeof(object),
            typeof(FilePicker),
            new PropertyMetadata(null));

        /// <summary>
        /// Identifies the <see cref="OpenButtonContent"/> dependency property.
        /// </summary>
        public static readonly DependencyProperty OpenButtonContentProperty = DependencyProperty.Register(
            nameof(OpenButtonContent),
            typeof(object),
            typeof(FilePicker),
            new PropertyMetadata(null));

        /// <summary>
        /// Identifies the <see cref="BrowseButtonToolTip"/> dependency property.
        /// </summary>
        public static readonly DependencyProperty BrowseButtonToolTipProperty = DependencyProperty.Register(
            nameof(BrowseButtonToolTip),
            typeof(object),
            typeof(FilePicker),
            new PropertyMetadata(null));

        /// <summary>
        /// Identifies the <see cref="ExploreButtonToolTip"/> dependency property.
        /// </summary>
        public static readonly DependencyProperty ExploreButtonToolTipProperty = DependencyProperty.Register(
            nameof(ExploreButtonToolTip),
            typeof(object),
            typeof(FilePicker),
            new PropertyMetadata(null));

        /// <summary>
        /// Identifies the <see cref="OpenButtonToolTip"/> dependency property.
        /// </summary>
        public static readonly DependencyProperty OpenButtonToolTipProperty = DependencyProperty.Register(
            nameof(OpenButtonToolTip),
            typeof(object),
            typeof(FilePicker),
            new PropertyMetadata(null));

        public static readonly DependencyProperty CornerRadiusProperty = DependencyProperty.Register(
            nameof(CornerRadius),
            typeof(CornerRadius),
            typeof(FilePicker),
            new PropertyMetadata(default(CornerRadius)));


        /// <summary>
        /// Initializes static members of the <see cref="FilePicker" /> class.
        /// </summary>
        static FilePicker()
        {
            DefaultStyleKeyProperty.OverrideMetadata(
                typeof(FilePicker), new FrameworkPropertyMetadata(typeof(FilePicker)));
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="FilePicker" /> class.
        /// </summary>
        public FilePicker()
        {
            BrowseCommand = new UiCommand(Browse);
            ExploreCommand = new UiCommand(Explore, CanExplore);
            OpenCommand = new UiCommand(Open, CanOpen);
        }

        /// <summary>
        /// Gets or sets the browse command.
        /// </summary>
        /// <value>The browse command.</value>
        public ICommand BrowseCommand { get; set; }

        /// <summary>
        /// Gets or sets the explore command.
        /// </summary>
        /// <value>The explore command.</value>
        public ICommand ExploreCommand { get; set; }

        /// <summary>
        /// Gets or sets the open command.
        /// </summary>
        /// <value>The open command.</value>
        public ICommand OpenCommand { get; set; }

        /// <summary>
        /// Gets or sets the default extension.
        /// </summary>
        /// <value>The default extension.</value>
        public string DefaultExtension
        {
            get => (string)GetValue(DefaultExtensionProperty);
            set => SetValue(DefaultExtensionProperty, value);
        }

        /// <summary>
        /// Gets or sets a value indicating whether the <see cref="FilePicker" /> allows users to select multiple files.
        /// </summary>
        /// <value><c>true</c> if multiple selections are allowed; otherwise, false. The default is <c>false</c>.</value>
        /// <remarks>When this feature is enabled, use the <see cref="FilePaths" /> property to get/set the filenames.</remarks>
        public bool Multiselect
        {
            get => (bool)GetValue(MultiselectProperty);
            set => SetValue(MultiselectProperty, value);
        }

        /// <summary>
        /// Gets a value indicating whether input is enabled.
        /// </summary>
        /// <remarks>If the <see cref="FilePicker" /> is in multi-select mode, disable free form text input.</remarks>
        public bool IsInputEnabled => !Multiselect;

        /// <summary>
        /// Gets or sets the base path.
        /// </summary>
        /// <value>The base path.</value>
        public string BasePath
        {
            get => (string)GetValue(BasePathProperty);
            set => SetValue(BasePathProperty, value);
        }

        /// <summary>
        /// Gets or sets the file path.
        /// </summary>
        /// <value>The file path.</value>
        public string FilePath
        {
            get => (string)GetValue(FilePathProperty);
            set => SetValue(FilePathProperty, value);
        }

        /// <summary>
        /// Gets or sets the file paths.
        /// </summary>
        /// <value>The file paths.</value>
        public string[] FilePaths
        {
            get => (string[])GetValue(FilePathsProperty);
            set => SetValue(FilePathsProperty, value);
        }

        /// <summary>
        /// Gets or sets the filter.
        /// </summary>
        /// <value>The filter.</value>
        public string Filter
        {
            get => (string)GetValue(FilterProperty);
            set => SetValue(FilterProperty, value);
        }

        /// <summary>
        /// Gets or sets a value indicating whether to use the File Open Dialog.
        /// </summary>
        /// <value>The "File Open" dialog is used if the property is set to <c>true</c>; otherwise, the File Save dialog is used.</value>
        public bool UseOpenDialog
        {
            get => (bool)GetValue(UseOpenDialogProperty);
            set => SetValue(UseOpenDialogProperty, value);
        }

        /// <summary>
        /// Gets or sets the content on the "browse" button.
        /// </summary>
        public object BrowseButtonContent
        {
            get => GetValue(BrowseButtonContentProperty);
            set => SetValue(BrowseButtonContentProperty, value);
        }

        /// <summary>
        /// Gets or sets the content on the "explore" button.
        /// </summary>
        public object ExploreButtonContent
        {
            get => GetValue(ExploreButtonContentProperty);
            set => SetValue(ExploreButtonContentProperty, value);
        }

        /// <summary>
        /// Gets or sets the content on the "open" button.
        /// </summary>
        public object OpenButtonContent
        {
            get => GetValue(OpenButtonContentProperty);
            set => SetValue(OpenButtonContentProperty, value);
        }

        /// <summary>
        /// Gets or sets the ToolTip on the "browse" button.
        /// </summary>
        public object BrowseButtonToolTip
        {
            get => GetValue(BrowseButtonToolTipProperty);
            set => SetValue(BrowseButtonToolTipProperty, value);
        }

        /// <summary>
        /// Gets or sets the ToolTip on the "explore" button.
        /// </summary>
        public object ExploreButtonToolTip
        {
            get => GetValue(ExploreButtonToolTipProperty);
            set => SetValue(ExploreButtonToolTipProperty, value);
        }

        /// <summary>
        /// Gets or sets the ToolTip on the "open" button.
        /// </summary>
        public object OpenButtonToolTip
        {
            get => GetValue(OpenButtonToolTipProperty);
            set => SetValue(OpenButtonToolTipProperty, value);
        }

        /// <summary>
        /// Gets or sets corner radius value
        /// </summary>
        public CornerRadius CornerRadius
        {
            get => (CornerRadius)GetValue(CornerRadiusProperty);
            set => SetValue(CornerRadiusProperty, value);
        }

        /// <summary>
        /// Gets the selected file paths.
        /// </summary>
        /// <value>
        /// A sequence of file paths.
        /// </value>
        private IEnumerable<string> SelectedFilePaths
        {
            get
            {
                if (Multiselect)
                {
                    return FilePaths ?? new string[0];
                }

                return FilePath != null ? new[] { FilePath } : new string[0];
            }
        }

        /// <summary>
        /// Ensures synchronization between <see cref="FilePaths" /> and <see cref="FilePath" /> properties when <see cref="Multiselect" /> is enabled
        /// </summary>
        /// <param name="dependencyObject">The <see cref="FilePicker" />.</param>
        /// <param name="ea">The <see cref="DependencyPropertyChangedEventArgs"/> instance containing the event data.</param>
        private static void OnFilePathsChanged(DependencyObject dependencyObject, DependencyPropertyChangedEventArgs ea)
        {
            var instance = (FilePicker)dependencyObject;
            if (instance.Multiselect && instance.FilePaths != null && instance.FilePaths.Length > 0)
            {
                instance.FilePath = string.Join(", ", instance.FilePaths);
            }
        }

        /// <summary>
        /// Shows the open or save file dialog.
        /// </summary>
        private void Browse(object _)
        {
            string filename = null;
            string[] filenames = null;

            if (!Multiselect)
            {
                filename = GetAbsolutePath(FilePath);
            }
            else
            {
                filenames = GetAbsolutePaths(FilePaths);
            }

            var ok = false;
            // use Microsoft.Win32 dialogs
            if (UseOpenDialog)
            {
                var d = new OpenFileDialog
                {
                    FileName = FilePath,
                    Filter = Filter,
                    DefaultExt = DefaultExtension,
                    Multiselect = Multiselect
                };
                if (true == d.ShowDialog())
                {
                    if (Multiselect)
                    {
                        filenames = d.FileNames;
                    }
                    else
                    {
                        filename = d.FileName;
                    }

                    ok = true;
                }
            }
            else
            {
                var d = new SaveFileDialog
                {
                    FileName = FilePath,
                    Filter = Filter,
                    DefaultExt = DefaultExtension
                };
                if (true == d.ShowDialog())
                {
                    filename = d.FileName;
                    ok = true;
                }
            }

            if (ok)
            {
                if (Multiselect)
                {
                    FilePaths = GetRelativePaths(filenames);
                }
                else
                {
                    FilePath = GetRelativePath(filename);
                }
            }
        }

        /// <summary>
        /// Starts Windows Explorer with the current file.
        /// </summary>
        private void Explore(object _)
        {
            System.Diagnostics.Process.Start("explorer.exe", "/select," + FilePath);
        }

        /// <summary>
        /// Opens the current file.
        /// </summary>
        private void Open(object _)
        {
            var filePath = SelectedFilePaths.FirstOrDefault();
            if (filePath != null)
            {
                System.Diagnostics.Process.Start(filePath);
            }
        }

        /// <summary>
        /// Determines whether the file can be opened.
        /// </summary>
        /// <returns>
        /// <c>true</c> if the file exists; otherwise, <c>false</c>.
        /// </returns>
        private bool CanOpen(object _)
        {
            var filePath = SelectedFilePaths.FirstOrDefault();
            return filePath != null && File.Exists(filePath);
        }

        /// <summary>
        /// Determines whether the file can be explored.
        /// </summary>
        /// <returns>
        /// <c>true</c> if the file exists; otherwise, <c>false</c>.
        /// </returns>
        private bool CanExplore(object o)
        {
            // same logic as for open file
            return CanOpen(o);
        }

        /// <summary>
        /// Gets the absolute path.
        /// </summary>
        /// <param name="filePath">The file path.</param>
        /// <returns>
        /// The get absolute path.
        /// </returns>
        private string GetAbsolutePath(string filePath)
        {
            if (filePath == null)
            {
                return null;
            }

            if (BasePath != null && !Path.IsPathRooted(filePath))
            {
                return Path.Combine(BasePath, filePath);
            }

            return filePath;
        }

        /// <summary>
        /// Gets the absolute paths.
        /// </summary>
        /// <param name="filePaths">The file paths.</param>
        /// <returns>
        /// The get absolute paths.
        /// </returns>
        private string[] GetAbsolutePaths(string[] filePaths)
        {
            if (filePaths == null || filePaths.Length == 0)
            {
                return filePaths;
            }

            return filePaths.Select(GetAbsolutePath).ToArray();
        }

        /// <summary>
        /// Gets the relative path.
        /// </summary>
        /// <param name="filePath">The file path.</param>
        /// <returns>
        /// The get relative path.
        /// </returns>
        private string GetRelativePath(string filePath)
        {
            if (BasePath == null)
            {
                return filePath;
            }

            if (filePath == null)
            {
                return null;
            }

            var uri1 = new Uri(filePath);
            var bp = Path.GetFullPath(BasePath);
            if (!bp.EndsWith("\\"))
            {
                bp += "\\";
            }

            var uri2 = new Uri(bp);
            var relativeUri = uri2.MakeRelativeUri(uri1);
            var relativePath = Uri.UnescapeDataString(relativeUri.OriginalString);
            return relativePath.Replace('/', '\\');
        }

        /// <summary>
        /// Gets the relative paths.
        /// </summary>
        /// <param name="filePaths">The file paths.</param>
        /// <returns>
        /// The get relative paths.
        /// </returns>
        private string[] GetRelativePaths(string[] filePaths)
        {
            if (BasePath == null)
            {
                return filePaths;
            }

            if (filePaths == null || filePaths.Length == 0)
            {
                return filePaths;
            }

            return filePaths.Select(GetRelativePath).ToArray();
        }
    }
}