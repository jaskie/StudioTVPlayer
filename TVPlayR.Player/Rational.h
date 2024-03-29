#pragma once

namespace TVPlayR {
	namespace Common {
		template <class T> class Rational;
	}
	public value class Rational sealed
	{
	private:
		const int _numerator;
		const int _denominator;
	
	internal:
		Rational(const Common::Rational<int>& rational)
			: _numerator(rational.Numerator())
			, _denominator(rational.Denominator())
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