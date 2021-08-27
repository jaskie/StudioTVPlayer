#pragma once
namespace TVPlayR {
	namespace FFmpeg {
		struct FFStreamOutputParams
		{
			std::string Address;
			std::string VideoCodec;
			std::string AudioCodec;
			int VideoBitrate;
			int AudioVitrate;
			std::string OutputFilter;
			std::string OutputMetadata;
			std::string AudioMetadata;
			std::string VideoMetadata;
			std::string Options;
			int VideoStreamId;
			int AudioStreamId;
		};

	}
}