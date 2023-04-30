#include "stdafx.h"
#include "InputBase.h"
#include "Core/InputSource.h"
#include "Core/FrameTimeInfo.h"
#include "TimeEventArgs.h"
#include "FieldOrder.h"
#include "OutputBase.h"

namespace TVPlayR
{
	InputBase::InputBase(std::shared_ptr<Core::InputSource>& input_source)
		: _nativeSource(new std::shared_ptr<Core::InputSource>(input_source))
	{
		_framePlayedDelegate = gcnew FramePlayedDelegate(this, &InputBase::FramePlayedCallback);
		_framePlayedHandle = GCHandle::Alloc(_framePlayedDelegate);
		IntPtr framePlayedIp = Marshal::GetFunctionPointerForDelegate(_framePlayedDelegate);
		typedef void(__stdcall* TIME_CALLBACK)(Core::FrameTimeInfo& time_info); // compatible with Core::InputSource::TIME_CALLBACK
		(*_nativeSource)->SetFramePlayedCallback(static_cast<TIME_CALLBACK>(framePlayedIp.ToPointer()));
		
		_loadedDelegate = gcnew LoadedDelegate(this, &InputBase::LoadedCallback);
		_loadedHandle = GCHandle::Alloc(_loadedDelegate);
		IntPtr loadedIp = Marshal::GetFunctionPointerForDelegate(_loadedDelegate);
		typedef void(__stdcall* LOADED_CALLBACK)(); // compatible with Core::InputSource::TIME_CALLBACK
		(*_nativeSource)->SetLoadedCallback(static_cast<LOADED_CALLBACK>(loadedIp.ToPointer()));
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
		REWRAP_EXCEPTION(delete _nativeSource;)
		_nativeSource = nullptr;
	}

	void InputBase::FramePlayedCallback(Core::FrameTimeInfo& time_info)
	{
		FramePlayed(this, gcnew TimeEventArgs(time_info.TimeFromBegin, time_info.TimeToEnd, time_info.Timecode));
	}

	void InputBase::LoadedCallback()
	{
		Loaded(this, EventArgs::Empty);
	}

	void InputBase::AddOutputSink(OutputSink^ output)
	{
		REWRAP_EXCEPTION((*_nativeSource)->AddOutputSink(output->GetNativeSink());)
	}

	void InputBase::RemoveOutputSink(OutputSink^ output)
	{
		REWRAP_EXCEPTION((*_nativeSource)->RemoveOutputSink(output->GetNativeSink());)
	}


}