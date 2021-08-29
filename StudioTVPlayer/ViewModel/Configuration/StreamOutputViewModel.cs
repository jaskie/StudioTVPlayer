using StudioTVPlayer.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class StreamOutputViewModel : OutputViewModelBase
    {
        public StreamOutputViewModel(StreamOutput output): base(output)
        { }

        public override bool IsValid()
        {
            throw new NotImplementedException();
        }
    }
}
