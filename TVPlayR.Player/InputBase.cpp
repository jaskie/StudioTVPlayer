#include "stdafx.h"
#include "InputBase.h"
#include "Core/InputSource.h"
#include "Core/FrameTimeInfo.h"
#include "TimeEventArgs.h"
#include "FieldOrder.h"

namespace TVPlayR
{
	InputBase::InputBase(std::shared_ptr<Core::InputSource> input_source)
		: _nativeSource(new std::shared_ptr<Core::InputSource>(input_source))
	{
		_framePlayedDelegate = gcnew FramePlayedDelegate(this, &InputBase::FramePlayedCallback);
		_framePlayedHandle = GCHandle::Alloc(_framePlayedDelegate);
		IntPtr framePlayedIp = Marshal::GetFunctionPointerForDelegate(_framePlayedDelegate);
		typedef void(__stdcall* TIME_CALLBACK)(Core::FrameTimeInfo& time_info); // compatible with Core::InputSource::TIME_CALLBACK
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

	void InputBase::FramePlayedCallback(Core::FrameTimeInfo& time_info)
	{
		FramePlayed(this, gcnew TimeEventArgs(time_info.TimeFromBegin, time_info.TimeToEnd, time_info.Timecode));
	}


}