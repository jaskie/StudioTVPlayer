#pragma once
namespace TVPlayR {
	namespace Core {
		struct AudioParameters
		{
			int SampleRate;
			int ChannelCount;
			AVSampleFormat SampleFormat;
		};
	}
}