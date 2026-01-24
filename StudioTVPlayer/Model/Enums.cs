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

    [TypeConverter(typeof(EnumDescriptionTypeConverter))]
    public enum ScheduleRepeatType
    {
        [Description("One time")]
        Single,
        [Description("Daily")]
        Daily
    }

    [TypeConverter(typeof(EnumDescriptionTypeConverter))]
    public enum FilenameCreationRule
    {
        [Description("Use provided name as filename directly")]
        None,
        [Description("Use name of the item with date/time at the beginning")]
        DateTimeAtBegin,
        [Description("Use name of the item with date/time at the end")]
        DateTimeAtEnd,
        [Description("Use name of the item with date at the beginning")]
        DateAtBegin,
        [Description("Use name of the item with date at the end")]
        DateAtEnd,
        [Description("Use name of the item with time at the beginning")]
        TimeAtBegin,
        [Description("Use name of the item with time at the end")]
        TimeAtEnd,
        [Description("Use name of the item to format date/time")]
        UseNameAsFormat
    }
}
