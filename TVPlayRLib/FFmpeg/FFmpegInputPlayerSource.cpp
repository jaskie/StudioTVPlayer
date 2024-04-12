#include "../pch.h"
#include "FFmpegInputPlayerSource.h"
#include "../Core/Player.h"
#include "../Core/AVSync.h"

namespace TVPlayR {
	namespace FFmpeg {

FFmpegInputPlayerSource::FFmpegInputPlayerSource(const Core::Player& player, bool process_video, int audio_channels)
	: Core::PlayerSynchroSource(player, process_video, audio_channels)
	, Common::DebugTarget(Common::DebugSeverity::trace, "FFmpegInputPlayerSource for " + player.Name())
{
}

void FFmpegInputPlayerSource::Push(const Core::AVSync& sync, AVRational frame_rate)
{
	DebugPrintLine(Common::DebugSeverity::trace, "Push:  video " + std::to_string(static_cast<float>(FrameTime(sync.Video)) / AV_TIME_BASE) + ", audio: " + std::to_string(static_cast<float>(FrameTime(sync.Audio)) / AV_TIME_BASE) + ", delta:" + std::to_string((FrameTime(sync.Video) - FrameTime(sync.Audio)) / 1000) + " ms");
	PlayerSynchroSource::Push(sync, frame_rate);
}

Core::AVSync FFmpegInputPlayerSource::PullSync(int audio_samples_count)
{
	auto sync = PlayerSynchroSource::PullSync(audio_samples_count);
	DebugPrintLine(Common::DebugSeverity::trace, "PullSync:  video " + std::to_string(static_cast<float>(FrameTime(sync.Video)) / AV_TIME_BASE) + ", audio: " + std::to_string(static_cast<float>(FrameTime(sync.Audio)) / AV_TIME_BASE) + ", delta:" + std::to_string((FrameTime(sync.Video) - FrameTime(sync.Audio)) / 1000) + " ms");
	return sync;
}

}}