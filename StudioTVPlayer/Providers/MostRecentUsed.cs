using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace StudioTVPlayer.Providers
{


    public class MostRecentUsed
    {
        private class MostRecentlyUsedList
        {
            private List<string> _items;
            private readonly string _fileName;
            private readonly int _count;

            public MostRecentlyUsedList(string fileName, int count)
            {
                _fileName = fileName;
                _count = count;
                if (File.Exists(fileName))
                {
                    try
                    {
                        _items = File.ReadAllLines(fileName).ToList();
                        return;
                    }
                    catch { } // ignore if unable to read
                }
                _items = new List<string>();
            }

            public bool Add(string folder)
            {
                var index = _items.IndexOf(folder);
                if (index >= 0 && index == _items.Count - 1)
                    return false;  // this is the already last item
                if (index >= 0)
                    _items.RemoveAt(index);
                else
                {
                    if (_items.Count > _count - 1)
                        _items.RemoveAt(0);
                }
                _items.Add(folder);
                return true;
            }

            public void Save()
            {
                try
                {
                    File.WriteAllLines(_fileName, _items);
                }
                catch { } // just ignore it
            }

            public List<string> Items { get { return _items; } }

        }

        private readonly MostRecentlyUsedList _folders = new MostRecentlyUsedList(Path.Combine(GlobalApplicationData.ApplicationDataDir, "MRU_Folders.txt"), 5);

        public IReadOnlyList<string> Folders => _folders.Items;

        public void AddMostRecentlyUsedFolder(string folder)
        {
            if (_folders.Add(folder))
                _folders.Save();
        }

        private MostRecentUsed() { }

        public static MostRecentUsed Current { get; } = new MostRecentUsed();
    }

}
