#pragma once
namespace TVPlayR {
	namespace FFmpeg {
		struct FFOutputParams
		{
			std::string Url;
			std::string VideoCodec;
			std::string AudioCodec;
			int VideoBitrate;
			int AudioBitrate;
			std::string Options;
			std::string OutputFilter;
			std::string OutputMetadata;
			std::string VideoMetadata;
			std::string AudioMetadata;
			int VideoStreamId;
			int AudioStreamId;
		};

	}
}