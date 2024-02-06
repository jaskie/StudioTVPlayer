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
        CreationTime,
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

    public enum TimeDisplaySource
    {
        TimeFromBegin,
        Timecode,
    }

    public enum MediaEventKind
    {
        Create,
        Delete,
        Change,
    }

    public enum PlayerState
    {
        Disabled,
        Unloaded,
        Cue,
        Playing,
        Paused,
        Finished
    }

    [TypeConverter(typeof(EnumDescriptionTypeConverter))]
    public enum PlayerMethodKind
    {
        [Description("Cue loaded clip")]
        Cue,
        [Description("Load next clip")]
        LoadNext,
        [Description("Play loaded clip")]
        Play,
        [Description("Pause playing clip")]
        Pause,
        [Description("Toggle playing clip")]
        Toggle,
        [Description("Clear player")]
        Clear,
    }

    [TypeConverter(typeof(EnumDescriptionTypeConverter))]
    public enum BlackmagicDesignAtemCommand
    {
        [Description("Program - input change")]
        PrgI,
        [Description("Preview - input change")]
        PrvI,
    }
}
