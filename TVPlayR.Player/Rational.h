#pragma once
#include "Common/rational.h"

namespace TVPlayR {
	public value  class Rational
	{
	private:
		const int _numerator;
		const int _denominator;
	
	internal:
		Rational(const Common::Rational<int>& rational)
			: _numerator(rational.numerator())
			, _denominator(rational.denominator())
		{ }

	public:
		Rational(const int numerator, const int denominator)
			: _numerator(numerator)
			, _denominator(denominator)
		{ }

		property int Numerator
		{
			int get() { return _numerator; }
		}

		property int Denominator
		{
			int get() { return _denominator; }
		}
	};
}