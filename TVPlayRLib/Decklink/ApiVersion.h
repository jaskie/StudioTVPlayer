#pragma once
namespace TVPlayR {
	namespace Decklink {

struct ApiVersion
{
	ApiVersion(const int major, const int minor, const int point)
		: Major(major)
		, Minor(minor)
		, Point(point)
	{}
	const int Major;
	const int Minor;
	const int Point;
};

}}