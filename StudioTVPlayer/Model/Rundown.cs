using StudioTVPlayer.Model.Args;
using StudioTVPlayer.Model.Persistence;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class Rundown : IDisposable
    {

        private RundownItemBase _loadedItem;
        private RundownItemBase _nextAutoPlayItem;
        private bool _isLoop;
        private int _isDisposed;
        private readonly List<RundownItemBase> _items = new List<RundownItemBase>();
        private readonly object _rundownLock = new object();

        public List<RundownItemBase> Items
        {
            get
            {
                lock (_rundownLock)
                {
                    return _items.ToList();
                }
            }
        }

        public RundownItemBase NextAutoPlayItem => _nextAutoPlayItem;
        public bool IsLoop
        {
            get => _isLoop; 
            set
            {
                if (_isLoop == value)
                    return;
                _isLoop = value;
                RundownChanged();
            }
        }

        public event EventHandler<RundownItemEventArgs> ItemLoaded;
        public event EventHandler<RundownItemIndexedEventArgs> ItemAdded;
        public event EventHandler<RundownItemIndexedEventArgs> ItemRemoved;

        public bool Contains(RundownItemBase item)
        {
            lock (_rundownLock)
            {
                return _items.Contains(item);
            }
        }

        public int IndexOf(RundownItemBase item)
        {
            lock (_rundownLock)
            {
                return _items.IndexOf(item);
            }
        }

        public void MoveItem(int srcIndex, int destIndex)
        {
            lock (_rundownLock)
            {
                var item = _items[srcIndex];
                _items.RemoveAt(srcIndex);
                _items.Insert(destIndex, item);
            }
            RundownChanged();
        }

        private void RundownItem_Loaded(object sender, EventArgs _)
        {
            var item = sender as RundownItemBase ?? throw new ArgumentException(nameof(sender));
            ItemLoaded?.Invoke(this, new RundownItemEventArgs(item));
            _loadedItem = item;
            RundownChanged();
        }

        private void RundownItem_RemoveRequested(object sender, EventArgs _)
        {
            var rundownItem = sender as RundownItemBase ?? throw new ArgumentException(nameof(sender));
            Remove(rundownItem);
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
            lock (_rundownLock)
            {
                if (index == -1 || index == _items.Count)
                    _items.Add(rundownItem);
                else if (index < _items.Count)
                    _items.Insert(index, rundownItem);
                else
                    throw new ArgumentException(nameof(index));
            }
            rundownItem.PropertyChanged += RundownItem_PropertyChanged;
            rundownItem.RemoveRequested += RundownItem_RemoveRequested;
            rundownItem.Loaded += RundownItem_Loaded;
            RundownChanged();
            ItemAdded?.Invoke(this, new RundownItemIndexedEventArgs(rundownItem, index));
        }

        public void Remove(RundownItemBase rundownItem)
        {
            int index;
            lock (_rundownLock)
            {
                index = _items.IndexOf(rundownItem);
                if (index >= 0)
                    _items.RemoveAt(index);
            }
            if (index == -1)
                return;
            rundownItem.PropertyChanged -= RundownItem_PropertyChanged;
            rundownItem.RemoveRequested -= RundownItem_RemoveRequested;
            rundownItem.Loaded -= RundownItem_Loaded;
            RundownChanged();
            ItemRemoved?.Invoke(this, new RundownItemIndexedEventArgs(rundownItem, index));
        }


        public void DeleteDisabled()
        {
            RundownItemBase item = null;
            while (true)
            {
                lock (_rundownLock)
                    item = _items.FirstOrDefault(i => i.IsDisabled);
                if (item is null)
                    break;
                Remove(item);
            }
        }

        private RundownItemBase FindNextAutoPlayItem(RundownItemBase currentItem)
        {
            lock (_rundownLock)
                using (var iterator = _items.GetEnumerator())
                {
                    bool foundCurrentItem = false;
                    while (iterator.MoveNext())
                    {
                        if (foundCurrentItem)
                        {
                            if (iterator.Current != null && iterator.Current.IsAutoStart && !iterator.Current.IsDisabled)
                                return iterator.Current;
                        }
                        if (iterator.Current == currentItem)
                            foundCurrentItem = true;
                    }
                }
            if (IsLoop)
                return _items.FirstOrDefault(i => i != currentItem && i.IsAutoStart && !i.IsDisabled);
            else
                return null;
        }

        private void RundownChanged()
        {
            if (_isDisposed != default)
                return;
            var loadedItem = _loadedItem;
            if (loadedItem == null)
                return;
            Task.Run(() =>
            {
                _nextAutoPlayItem = FindNextAutoPlayItem(loadedItem);
            });
        }

        public void ClearItems()
        {
            foreach (var item in Items)
                Remove(item);
        }

        public bool CanLoadNextItem(RundownItemBase fromRundownItem)
        {
            lock (_rundownLock)
            {
                var currentIndex = _items.IndexOf(fromRundownItem);
                while (++currentIndex < _items.Count)
                {
                    if (_items[currentIndex].IsDisabled)
                        continue;
                    return true;
                }
                return false;
            }
        }

        public RundownItemBase FindNextItemToLoad(RundownItemBase fromRundownItem)
        {
            lock (_rundownLock)
            {
                var currentIndex = _items.IndexOf(fromRundownItem);
                if (currentIndex >= _items.Count - 1)
                    return null;
                while (++currentIndex < _items.Count)
                {
                    if (_items[currentIndex].IsDisabled)
                        continue;
                    return _items[currentIndex];
                }
            }
            return null;
        }

        public void Dispose()
        {
            if (Interlocked.Exchange(ref _isDisposed, 1) == default)
                return;
            foreach (var item in _items.ToList())
                Remove(item);
            _items.Clear();
        }
    }
}
