#pragma once
#include "Utils.h"

namespace TVPlayR {
	namespace FFmpeg {

class FilterBase
{
public:
	virtual AVRational OutputTimeBase() const = 0;
	virtual void Flush() = 0;
	virtual bool IsFlushed() const = 0;
	virtual bool IsEof() const = 0;
	int64_t TimeFromTs(int64_t ts) const;
	virtual void Reset() = 0;
};
	
}}