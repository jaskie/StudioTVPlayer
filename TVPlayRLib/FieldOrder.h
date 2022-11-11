#pragma once

namespace TVPlayR {

#if (_MANAGED == 1)
		public
#endif
enum class FieldOrder {
	Unknown,
	Progressive,
	BottomFieldFirst,
	TopFieldFirst
};

#if (_MANAGED != 1)
static FieldOrder FieldOrderFromAVFieldOrder(const AVFieldOrder field_order)
{
	switch (field_order)
	{
	case AV_FIELD_UNKNOWN:
		return FieldOrder::Unknown;
	case AV_FIELD_PROGRESSIVE:
		return FieldOrder::Progressive;
	case AV_FIELD_TT:          //< Top coded_first, top displayed first
	case AV_FIELD_TB:          //< Top coded first, bottom displayed first
			return FieldOrder::TopFieldFirst;
	case AV_FIELD_BB:          //< Bottom coded first, bottom displayed first
	case AV_FIELD_BT:          //< Bottom coded first, top displayed first
		return FieldOrder::BottomFieldFirst;
	default:
		THROW_EXCEPTION("Invalid field order: " + std::to_string(field_order))
	}
}
#endif

}