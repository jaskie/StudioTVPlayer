using System.ComponentModel;
using StudioTVPlayer.Converters;

namespace StudioTVPlayer.Model
{
    [TypeConverter(typeof(EnumDescriptionTypeConverter))]
    public enum Sortings
    {
        [Description("Nazwa")]
        Name,
        [Description("Czas trwania")]
        Duration,
        [Description("Data modyfikacji")]
        CreationDate
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
