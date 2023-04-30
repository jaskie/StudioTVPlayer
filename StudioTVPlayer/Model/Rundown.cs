using StudioTVPlayer.Model.Args;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class Rundown : IDisposable
    {

        private RundownItemBase _loadedItem;
        private RundownItemBase _nextAutoPlayItem;
        private readonly List<RundownItemBase> _rundown = new List<RundownItemBase>();

        public List<RundownItemBase> Items => _rundown;
        public RundownItemBase NextAutoPlayItem => _nextAutoPlayItem;
        public bool IsLoop { get; set; }

        public event EventHandler<RundownItemEventArgs> Loaded;
        public event EventHandler<RundownItemEventArgs> Removed;

        public bool Contains(RundownItemBase item) => _rundown.Contains(item);

        public void MoveItem(int srcIndex, int destIndex)
        {
            var item = _rundown[srcIndex];
            _rundown.RemoveAt(srcIndex);
            _rundown.Insert(destIndex, item);
            RundownChanged();
        }

        private void RundownItem_Loaded(object sender, EventArgs _)
        {
            var item = sender as RundownItemBase ?? throw new ArgumentException(nameof(sender));
            Loaded?.Invoke(this, new RundownItemEventArgs(item));
            _loadedItem = item;
            RundownChanged();
        }

        private void RundownItem_RemoveRequested(object sender, EventArgs _)
        {
            var rundownItem = sender as RundownItemBase ?? throw new ArgumentException(nameof(sender));
            if (!Remove(rundownItem))
                return;
            Removed?.Invoke(this, new RundownItemEventArgs(rundownItem));
        }

        private void RundownItem_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            var rundownItem = sender as RundownItemBase ?? throw new ArgumentException(nameof(sender));
            switch (e.PropertyName)
            {
                case nameof(RundownItemBase.IsAutoStart):
                case nameof(RundownItemBase.IsDisabled):
                    RundownChanged();
                    break;
            }
        }

        public void Add(RundownItemBase rundownItem, int index = -1)
        {
            if (index < _rundown.Count)
                _rundown.Insert(index, rundownItem);
            else if (index == -1 || index == _rundown.Count)
                _rundown.Add(rundownItem);
            else
                throw new ArgumentException(nameof(index));
            rundownItem.PropertyChanged += RundownItem_PropertyChanged;
            rundownItem.RemoveRequested += RundownItem_RemoveRequested;
            rundownItem.Loaded += RundownItem_Loaded;
            RundownChanged();
        }

        public bool Remove(RundownItemBase rundownItem)
        {
            if (!_rundown.Remove(rundownItem))
                return false;
            rundownItem.PropertyChanged -= RundownItem_PropertyChanged;
            rundownItem.RemoveRequested -= RundownItem_RemoveRequested;
            rundownItem.Loaded -= RundownItem_Loaded;
            RundownChanged();
            return true;
        }


        public void DeleteDisabled()
        {
            RundownItemBase item = null;
            while (true)
            {
                item = _rundown.FirstOrDefault(i => i.IsDisabled);
                if (item is null)
                    break;
                Remove(item);
            }
        }

        private RundownItemBase FindNextAutoPlayItem(RundownItemBase currentItem)
        {
            using (var iterator = _rundown.GetEnumerator())
            {
                bool found = false;
                while (iterator.MoveNext())
                {
                    if (found)
                    {
                        if (iterator.Current != null && iterator.Current.IsAutoStart && !iterator.Current.IsDisabled)
                            return iterator.Current;
                    }
                    if (iterator.Current == currentItem)
                        found = true;
                }
            }
            if (IsLoop)
                return _rundown.FirstOrDefault(i => i != currentItem && i.IsAutoStart && !i.IsDisabled);
            else
                return null;
        }

        private void RundownChanged()
        {
            var loadedItem = _loadedItem;
            if (loadedItem == null)
                return;
            Task.Run(() =>
            {
                _nextAutoPlayItem = FindNextAutoPlayItem(loadedItem);
            });
        }


        public void Dispose()
        {
            foreach (var item in _rundown.ToList())
                Remove(item);
            _rundown.Clear();
        }
    }
}
