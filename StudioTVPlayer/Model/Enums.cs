using System.ComponentModel;
using StudioTVPlayer.Converters;

namespace StudioTVPlayer.Model
{
    [TypeConverter(typeof(EnumDescriptionTypeConverter))]
    public enum Sorting
    {
        [Description("Name")]
        Name,
        [Description("Duration")]
        Duration,
        [Description("Creation time")]
        CreationTime
    }

    [TypeConverter(typeof(EnumDescriptionTypeConverter))]
    public enum ScanType
    {
        [Description("Progressive")]
        Progressive,
        [Description("Top field first")]
        TopFieldFirst,
        [Description("Bottom Field First")]
        BottomFieldFirst
    }

    public enum SortDirection
    {
        Ascending,
        Descending
    }

    public enum FFMeta
    {
        All,
        Duration,
        Thumbnail
    }

    public enum ThumbnailType
    {
        NoPreview,
        Loading
    }

    public enum MediaEventKind
    {
        Create,
        Delete,
        Change
    }
}
