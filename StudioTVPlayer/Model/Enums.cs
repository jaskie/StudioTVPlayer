using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using StudioTVPlayer.Converters;

namespace StudioTVPlayer.Model
{
    public class Enums
    {
        public enum ViewType
        {
            Piloting,
            Configuration
        }

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
    }
}
