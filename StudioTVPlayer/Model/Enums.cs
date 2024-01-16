﻿using System.ComponentModel;
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

    [TypeConverter(typeof(EnumDescriptionTypeConverter))]
    public enum PlayerMethodKind
    {
        Cue,
        [Description("Load next clip")]
        LoadNext,
        Play,
        Pause,
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
