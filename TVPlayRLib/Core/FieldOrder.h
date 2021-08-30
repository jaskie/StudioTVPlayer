#pragma once
#include "../Common/Exceptions.h"

namespace TVPlayR {
	namespace Core {

enum class FieldOrder {
	unknown,
	progressive,
	lower,
	upper
};

static FieldOrder FieldOrderFromAVFieldOrder(const AVFieldOrder field_order)
{
	switch (field_order)
	{
	case AV_FIELD_UNKNOWN:
		return FieldOrder::unknown;
	case AV_FIELD_PROGRESSIVE:
		return FieldOrder::progressive;
	case AV_FIELD_TT:          //< Top coded_first, top displayed first
	case AV_FIELD_BT:          //< Bottom coded first, top displayed first
			return FieldOrder::upper;
	case AV_FIELD_BB:          //< Bottom coded first, bottom displayed first
	case AV_FIELD_TB:          //< Top coded first, bottom displayed first
			return FieldOrder::lower;
	default:
		THROW_EXCEPTION("invalid pixel format")
	}
}

}}