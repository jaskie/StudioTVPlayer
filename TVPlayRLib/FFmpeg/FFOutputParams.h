#pragma once
namespace TVPlayR {
	namespace FFmpeg {
		struct FFOutputParams
		{
			std::string Url;
			std::string VideoCodec, AudioCodec;
			int VideoBitrate, AudioBitrate;
			std::string Options;
			std::string VideoFilter;
			std::string OutputMetadata, VideoMetadata, AudioMetadata;
			int VideoStreamId, AudioStreamId;
		};

	}
}