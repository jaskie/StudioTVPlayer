#include "../pch.h"
#include "Preview.h"
#include "../Common/Exceptions.h"

namespace TVPlayR {
	namespace Preview {

	struct Preview::implementation
	{

	};

	Preview::Preview()
		: impl_(std::make_unique<implementation>())
	{
	}

	Preview::~Preview()	{}

	void Preview::ReleaseChannel()
	{
	}

	bool Preview::IsPlaying() const
	{
		return false;
	}

	void Preview::Push(FFmpeg::AVSync& sync)
	{
	}

	void Preview::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
	{
		THROW_EXCEPTION("The preview cannot act as clock source");
	}
}}
