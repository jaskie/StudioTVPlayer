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
            base.OnStop();
        }

        private static void Execute(bool userInteractive)
        {
            Channels channels;
            using (var reader = new StreamReader("config.xml"))
            {
                var serializer = new XmlSerializer(typeof(Channels));
                channels = (Channels)serializer.Deserialize(reader);
            }
            if (userInteractive)
            {
                var line = Console.ReadLine().ToLower();
                switch(line)
                {
                    case "q":
                    case "quit":
                        break;
                }
            }
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
