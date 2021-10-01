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
		(*_nativeSource)->SetFramePlayedCallback(static_cast<Core::InputSource::TIME_CALLBACK>(framePlayedIp.ToPointer()));
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

	void InputBase::FramePlayedCallback(int64_t time)
	{
		FramePlayed(this, gcnew TimeEventArgs(TimeSpan(time * 10)));
	}


}