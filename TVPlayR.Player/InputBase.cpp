#include "stdafx.h"
#include "InputBase.h"
#include "Core/InputSource.h"

namespace TVPlayR
{
	InputBase::InputBase(std::shared_ptr<Core::InputSource> input_source)
		: _nativeSource(new std::shared_ptr<Core::InputSource>(input_source))
	{
		_framePlayedDelegate = gcnew FramePlayedDelegate(this, &InputBase::FramePlayedCallback);
		_framePlayedHandle = GCHandle::Alloc(_framePlayedDelegate);
		IntPtr framePlayedIp = Marshal::GetFunctionPointerForDelegate(_framePlayedDelegate);
		typedef void(__stdcall* TIME_CALLBACK)(std::int64_t); // compatible with Core::InputSource::TIME_CALLBACK
		(*_nativeSource)->SetFramePlayedCallback(static_cast<TIME_CALLBACK>(framePlayedIp.ToPointer()));
	}

	InputBase::~InputBase()
	{
		this->!InputBase();
	}

	InputBase::!InputBase()
	{
		if (!_nativeSource)
			return;
		(*_nativeSource)->SetFramePlayedCallback(nullptr);
		_framePlayedHandle.Free();
		delete _nativeSource;
		_nativeSource = nullptr;
	}

	void InputBase::FramePlayedCallback(std::int64_t time)
	{
		FramePlayed(this, gcnew TimeEventArgs(TimeSpan(time * 10)));
	}


}