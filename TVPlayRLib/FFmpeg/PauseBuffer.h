#pragma once
#include "../FieldOrder.h"
namespace TVPlayR {
	namespace FFmpeg {

class PauseBuffer: public Common::NonCopyable
{
public:
	PauseBuffer(FieldOrder field_order, bool is_playing);
	void SetFrame(std::shared_ptr<AVFrame>& frame);
	std::shared_ptr<AVFrame> GetFrame();
	void SetIsPlaying(bool is_playing);
	bool IsEmpty();
	int64_t Pts();
	void Clear();
private:
	std::mutex mutex_;
	const FieldOrder field_order_;
	std::shared_ptr<AVFrame> last_frame_;
	std::shared_ptr<AVFrame> still_frame_;
	bool is_playing_;
	std::shared_ptr<AVFrame>& GetStillFrame();
	std::shared_ptr<AVFrame> FrameToField(std::shared_ptr<AVFrame>& source, bool top_field);
};

}}
