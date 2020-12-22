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

        public static void RegisterSingleton<T>(T instance) 
        {
            Instances.Add(typeof(T), instance);
        }

        public static T Get<T>(object[] parameters = null) where T : class
        {
            return (T)Get(typeof(T), parameters);
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

        private static object Get(Type requestedType, object[] parameters = null)
        {
            if (Instances.TryGetValue(requestedType, out var instance))
                return instance;

            if (!Bindings.TryGetValue(requestedType, out var registeredType))
            {
                if (requestedType.IsAbstract)
                    throw new ApplicationException($"Type {requestedType} not registered");
                else registeredType = requestedType;
            }
            var constructor = registeredType.GetConstructors().FirstOrDefault();
            if (constructor == null)
                throw new ApplicationException($"No default constructor found for type {registeredType}");
            var localParameters = constructor.GetParameters();
            if ((parameters?.Length ?? 0) == localParameters.Length)
                instance = constructor.Invoke(parameters);
            else
            {
                parameters = new object[localParameters.Length];
                for (int i = 0; i < localParameters.Length; ++i)
                    parameters[i] = Get(localParameters[i].ParameterType);
                instance = constructor.Invoke(parameters);
            }
            return instance;
        }
    }
}
