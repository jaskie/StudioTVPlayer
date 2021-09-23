using System;
using System.Configuration.Install;
using System.IO;
using System.Reflection;
using System.ServiceProcess;
using System.Xml.Serialization;

namespace TimecodeDecoderService
{
    [System.ComponentModel.DesignerCategory("Code")]
    public class Program : ServiceBase
    {
        private static Channels _channels;

        public Program()
        {
            ServiceName = "TimecodeDecoderService";
            CanStop = true;
            CanPauseAndContinue = false;
            AutoLog = true;
        }

        protected override void OnStart(string[] args)
        {
            base.OnStart(args);
            Execute(false);
        }

        protected override void OnStop()
        {
            Cleanup();
            base.OnStop();
        }

        private static void Execute(bool userInteractive)
        {
            var fileName = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "config.xml");
            using (var reader = new StreamReader(fileName))
            {
                var serializer = new XmlSerializer(typeof(Channels));
                _channels = (Channels)serializer.Deserialize(reader);
            }
            _channels?.StartAll();
            if (userInteractive)
            {
                ExecuteConsoleCommands();
                Cleanup();
            }
        }

        public static void ExecuteConsoleCommands()
        {
            while (true)
            {
                var line = Console.ReadLine().ToLower();
                switch (line)
                {
                    case "q":
                    case "quit":
                        return;
                }
            }
        }
        
        private static void Cleanup()
        {
            _channels.Dispose();
            _channels = null;
        }

        public static void Main(string[] args)
        {
            if (Environment.UserInteractive)
            {
                string parameter = string.Concat(args);
                switch (parameter)
                {
                    case "--install":
                        ManagedInstallerClass.InstallHelper(new[] { Assembly.GetExecutingAssembly().Location });
                        break;
                    case "--uninstall":
                        ManagedInstallerClass.InstallHelper(new[] { "/u", Assembly.GetExecutingAssembly().Location });
                        break;
                    default:
                        Execute(true);
                        Environment.Exit(0);
                        break;
                }
            }
            else
            {
                Run(new Program());
            }
        }
    }
}
