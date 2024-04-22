#pragma once

namespace TVPlayR {

#if (_MANAGED == 1)
	public
#endif
		enum class ColorSpace
	{
		bt601,
		bt709,
		bt2020
	};
#if (_MANAGED != 1)
	static std::string ColorSpaceToString(enum ColorSpace color_space) {
		switch (color_space)
		{
		case ColorSpace::bt601:
			return "bt601";
		case ColorSpace::bt709:
			return "bt709";
		case ColorSpace::bt2020:
			return "bt2020";
		default:
			THROW_EXCEPTION("Invalid color space");
		}
	}
#endif
}