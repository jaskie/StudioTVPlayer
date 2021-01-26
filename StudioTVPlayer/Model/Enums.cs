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
