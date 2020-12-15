using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Helpers
{
    public static class ObjectOperations
    {
        public static T Clone<T>(this T source)
        {
            var target = (T)Activator.CreateInstance(typeof(T));
            var sourceProps = typeof(T).GetProperties().Where(x => x.CanRead).ToList();
            var targetProps = typeof(T).GetProperties().Where(x => x.CanWrite).ToList();

            foreach (var sourceProp in sourceProps)
                targetProps.First(x => x.Name == sourceProp.Name).SetValue(target, sourceProp.GetValue(source, null), null);

            return (T)target;
        }
    }
}
