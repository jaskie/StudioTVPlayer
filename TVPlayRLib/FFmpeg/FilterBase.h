#pragma once
#include "../Common/NonCopyable.h"

namespace TVPlayR {
	namespace FFmpeg {

class FilterBase : public Common::NonCopyable
{
public:
	FilterBase();
	virtual AVRational OutputTimeBase() const = 0;
	virtual std::shared_ptr<AVFrame> Pull() = 0;
	virtual void Flush() = 0;
	bool IsFlushed() const { return is_flushed_; }
	bool IsEof() { return is_eof_; };
	std::int64_t TimeFromTs(std::int64_t ts) const;
protected:
	std::unique_ptr<AVFilterGraph, void(*)(AVFilterGraph*)> graph_;
	bool is_flushed_ = false;
	bool is_eof_ = false;
};
	
}}