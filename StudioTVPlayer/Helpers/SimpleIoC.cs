using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using StudioTVPlayer.ViewModel;

namespace StudioTVPlayer.Helpers
{
    public static class SimpleIoc
    {
        private static Dictionary<Type, Type> Bindings = new Dictionary<Type, Type>();
        private static Dictionary<Type, object> Instances = new Dictionary<Type, object>();

        public static void Register<T>() => Bindings.Add(typeof(T), typeof(T));
        public static void Register<T, R>() => Bindings.Add(typeof(T), typeof(R));

        //public static void RegisterInstance<T>(bool multipleInstancesMember, T instance) //do wywalenia, powinno unikać się parametrów w konstruktorach
        //{
        //    Bindings.TryGetValue(typeof(T), out var registeredType);

        //    if (multipleInstancesMember)
        //        if ((!(Instances.TryGetValue(registeredType, out _))) && multipleInstancesMember == false)
        //            Instances.Add(typeof(T), instance);
        //        else { }
        //    else
        //    {
        //        if (!Instances.TryGetValue(typeof(List<T>), out var list))
        //            Instances.Add(typeof(List<T>), Activator.CreateInstance(typeof(List<T>)));

        //        Instances.TryGetValue(typeof(List<T>), out list);

        //        List<T> tempList = list as List<T>;
        //        tempList.Add((T)instance);
        //    }
        //}

        public static T GetInstance<T>(bool multipleInstancesMember = false, object[] parameters = null) where T : class
        {
            Bindings.TryGetValue(typeof(T), out var registeredType);
            var constructors = typeof(T).GetConstructors();

            var constructor = constructors.FirstOrDefault((c) => {
                var localParameters = c.GetParameters();
                if (localParameters.Length == 0)
                    return false;
                else
                    return true;                               
            });

            if (constructor != null)
            {
                bool parametersFilled = default(bool);
                var localParameters = constructor.GetParameters();

                if (parameters?.Length == localParameters.Length)
                    parametersFilled = true;

                if (!parametersFilled)
                {
                    parameters = new object[localParameters.Length];
                    for (int i = 0; i < localParameters.Length; ++i)
                    {
                        if (Instances.TryGetValue(localParameters[i].ParameterType, out var localInstance))
                            parameters[i] = localInstance;
                        else
                        {
                            Bindings.TryGetValue(localParameters[i].ParameterType, out var type);
                            Instances.Add(localParameters[i].ParameterType, Activator.CreateInstance(type));
                            Instances.TryGetValue(localParameters[i].ParameterType, out parameters[i]);
                        }
                    }
                }
                
            }

            if ((!(Instances.TryGetValue(registeredType, out var instance))) && multipleInstancesMember == false)
            {
                Instances.Add(typeof(T), (T)Activator.CreateInstance(registeredType, parameters));
                Instances.TryGetValue(registeredType, out instance);
            }

            if (multipleInstancesMember == true)
            {
                if (!Instances.TryGetValue(typeof(List<T>), out var list))
                    Instances.Add(typeof(List<T>), Activator.CreateInstance(typeof(List<T>)));

                Instances.TryGetValue(typeof(List<T>), out list);
                instance = (T)Activator.CreateInstance(registeredType,parameters);

                List<T> tempList = list as List<T>;
                tempList.Add((T)instance);
            }
            

            return (T)instance;                      
        }

        public static void Destroy<T>()
        {
            if (!Instances.TryGetValue(typeof(T), out _))
                return;
            Instances.Remove(typeof(T));
        }

        public static List<T> GetInstances<T>()
        {
            Bindings.TryGetValue(typeof(List<T>), out var registeredType);
            if (!Instances.TryGetValue(typeof(List<T>), out var list))
                Instances.Add(typeof(List<T>), Activator.CreateInstance(typeof(List<T>)));

            if (list == null)
                Instances.TryGetValue(typeof(List<T>), out list);

            return list as List<T>;
        }        
    }
}
