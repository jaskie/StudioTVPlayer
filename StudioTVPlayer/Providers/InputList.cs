using StudioTVPlayer.Helpers;
using System.Collections.Generic;
using System.IO;
using System.Xml.Serialization;

namespace StudioTVPlayer.Providers
{
    public class InputList
    {
        private const string InputsFile = "inputs.xml";

        [XmlArray]
        [XmlArrayItem(typeof(Model.DecklinkInput))]
        public List<Model.InputBase> Inputs { get; set; } = new List<Model.InputBase>();

        public static InputList Current { get; } = Load();

        private static InputList Load()
        {
            var inputsFile = Path.Combine(GlobalApplicationData.ApplicationDataDir, InputsFile);
            try
            {
                return DataStore.Load<InputList>(inputsFile) ?? new InputList();
            }
            catch
            {
                return new InputList();
            }
        }

        public void Save()
        {
            var inputsFile = Path.Combine(GlobalApplicationData.ApplicationDataDir, InputsFile);
            DataStore.Save(this, inputsFile);
        }

        public bool RemoveInput(Model.InputBase input)
        {
            if (!Inputs.Remove(input))
                return false;
            Save();
            return true;
        }

    }
}
