#pragma once
namespace TVPlayR {
	namespace FFmpeg {
		struct FFStreamOutputParams
		{
			std::string Address;
			std::string VideoCodec;
			std::string AudioCodec;
			int VideoBitrate;
			int AudioBitrate;
			std::string OutputFilter;
			std::string OutputMetadata;
			std::string VideoMetadata;
			std::string AudioMetadata;
			std::string Options;
			int VideoStreamId;
			int AudioStreamId;
		};

	}
}