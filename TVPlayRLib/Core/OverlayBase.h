#pragma once
#include "../Common/NonCopyable.h"

namespace TVPlayR {
	namespace Core {

		struct AVSync;

class OverlayBase : public Common::NonCopyable
{
public:
	virtual Core::AVSync Transform(Core::AVSync& sync) = 0;
};

}}