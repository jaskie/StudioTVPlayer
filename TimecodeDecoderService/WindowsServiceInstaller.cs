using System.ComponentModel;
using System.Configuration.Install;
using System.ServiceProcess;

namespace TimecodeDecoderService
{
    [RunInstaller(true), DesignerCategory("Code")]
    public class WindowsServiceInstaller : Installer
    {
        private ServiceProcessInstaller processInstaller;
        private ServiceInstaller serviceInstaller;

        public WindowsServiceInstaller()
        {
            processInstaller = new ServiceProcessInstaller();
            serviceInstaller = new ServiceInstaller();

            processInstaller.Account = ServiceAccount.LocalSystem;
            serviceInstaller.StartType = ServiceStartMode.Manual;
            serviceInstaller.ServiceName = "TimecodeDecoderService";
            serviceInstaller.Description = "Timecode decoder";
            foreach (var installer in serviceInstaller.Installers)
                if (installer is System.Diagnostics.EventLogInstaller eventLogInstaller)
                {
                    serviceInstaller.Installers.Remove(eventLogInstaller);
                    break;
                }
            Installers.Add(serviceInstaller);
            Installers.Add(processInstaller);
        }
    }
}
