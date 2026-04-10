using System;
using System.Collections.Generic;
using System.Linq;

namespace StudioTVPlayer.Providers
{
    public class CommandLineProcessor
    {
        private readonly string[] _commandLineArgs;
        private readonly Dictionary<string, object> _commandLineParams = [];
        private CommandLineProcessor()
        {
            _commandLineArgs = Environment.GetCommandLineArgs();
        }

        public static CommandLineProcessor Instance { get; } = new CommandLineProcessor();

        public string RundownToLoad(string playerName) => GetParam<string>($"LoadRundown:{playerName}");
        
        private T GetParam<T>(string paramName)
        {
            if (_commandLineParams.TryGetValue(paramName, out var value))
                return (T)value;
            var arg = _commandLineArgs.FirstOrDefault(a => a.TrimStart(' ', '\"').StartsWith(paramName, StringComparison.OrdinalIgnoreCase));
            if (arg is null)
                value = default(T);
            else
            {
                var splitted = arg.Split(['='], 2, StringSplitOptions.RemoveEmptyEntries);
                if (splitted.Length != 2)
                    value = default(T);
                else
                {
                    var trimmed = splitted[1].Trim(' ', '\"');
                    value = Convert.ChangeType(trimmed, typeof(T));
                }
            }
            _commandLineParams[paramName] = value;
            return (T)value;
        }
    }
}
