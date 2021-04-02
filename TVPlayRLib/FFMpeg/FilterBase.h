#pragma once
#include "Utils.h"

namespace TVPlayR {
	namespace FFmpeg {

class FilterBase
{
public:
	FilterBase();
	virtual AVRational OutputTimeBase() const = 0;
	virtual std::shared_ptr<AVFrame> Pull() = 0;
	virtual void Flush() = 0;
	bool IsFlushed() const { return is_flushed_; }
	bool IsEof() { return is_eof_; };
	int64_t TimeFromTs(int64_t ts) const;
protected:
	virtual void Initialize() = 0;
	std::unique_ptr<AVFilterGraph, void(*)(AVFilterGraph*)> graph_;
	bool is_flushed_ = false;
	bool is_eof_ = false;
};
	
}}