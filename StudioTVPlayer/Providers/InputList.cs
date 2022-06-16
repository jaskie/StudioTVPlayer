using StudioTVPlayer.Helpers;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml.Serialization;

namespace StudioTVPlayer.Providers
{
    public class InputList
    {
        private const string InputsFile = "inputs.xml";
        
        [XmlArray(nameof(Inputs))]
        [XmlArrayItem(typeof(Model.DecklinkInput))]
        public List<Model.InputBase> _inputs = new List<Model.InputBase>();

        [XmlIgnore]
        public IEnumerable<Model.InputBase> Inputs { get => _inputs; }
        
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
            if (!_inputs.Remove(input))
                return false;
            input.Dispose();
            Save();
            return true;
        }

        public Model.DecklinkInput AddDecklinkInput()
        {
            var input = new Model.DecklinkInput();
            _inputs.Add(input);
            input.Initialize();
            Save();
            return input;
        }

        public bool CanAddDecklinkInput => TVPlayR.DecklinkIterator.Devices.Any(d => d.HaveInput);

    }
}
