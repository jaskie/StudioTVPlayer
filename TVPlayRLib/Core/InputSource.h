#pragma once

namespace TVPlayR {
	enum class FieldOrder;

	namespace Core {
		class Player;
		class OutputSink;
		struct FrameTimeInfo;
		struct AVSync;

class InputSource : Common::NonCopyable
{
public:
	typedef std::function<void(FrameTimeInfo&)> TIME_CALLBACK;
	typedef std::function<void()> LOADED_CALLBACK;
	virtual Core::AVSync PullSync(const Core::Player &player, int audio_samples_count) = 0;
	virtual bool IsAddedToPlayer(const Player &player) = 0;
	virtual void AddToPlayer(const Player &player) = 0;
	virtual void RemoveFromPlayer(const Core::Player &player) = 0;
	virtual void AddOutputSink(std::shared_ptr<Core::OutputSink> &output_sink) = 0;
	virtual void RemoveOutputSink(std::shared_ptr<Core::OutputSink> &output_sink) = 0;
	virtual void Play() = 0;
	virtual void Pause() = 0;
	virtual bool IsPlaying() const = 0;
	virtual bool IsEof() const = 0;
	virtual std::int64_t GetVideoStart() const { return 0LL; }
	virtual std::int64_t GetVideoDuration() const { return 0LL; }
	virtual std::int64_t GetAudioDuration() const { return 0LL; }
	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;
	virtual TVPlayR::FieldOrder GetFieldOrder() = 0;
	virtual int GetAudioChannelCount() = 0;
	virtual bool HaveAlphaChannel() const = 0;
	virtual void SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) = 0;
	void RaiseLoaded();
	void SetLoadedCallback(LOADED_CALLBACK callback);
private:
	LOADED_CALLBACK _loadedCallback = nullptr;
};

}}