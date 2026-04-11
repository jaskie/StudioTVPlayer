#pragma once
#include "../FieldOrder.h"
namespace TVPlayR {
	namespace FFmpeg {

class PauseBuffer: public Common::NonCopyable
{
public:
	PauseBuffer(FieldOrder field_order, bool is_playing);
	void SetFrame(std::shared_ptr<const AVFrame>& frame);
	std::shared_ptr<const AVFrame> GetFrame();
	void SetIsPlaying(bool is_playing);
	bool IsEmpty() const;
	int64_t Pts();
	void Clear();
private:
	mutable std::mutex mutex_;
	const FieldOrder field_order_;
	std::shared_ptr<const AVFrame> last_frame_;
	std::shared_ptr<const AVFrame> still_frame_;
	bool is_playing_;
	std::shared_ptr<const AVFrame>& GetStillFrame();
	std::shared_ptr<const AVFrame> FrameToField(std::shared_ptr<const AVFrame>& source, bool top_field);
};

}}
