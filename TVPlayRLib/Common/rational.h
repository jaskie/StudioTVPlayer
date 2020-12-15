#pragma once

namespace TVPlayR {
	namespace Common {
		
template <class T> class Rational
{
private:
	T numerator_;
	T denominator_ = 1;
public:
	Rational(T num, T den)
		: numerator_(num)
		, denominator_(den) { }

	Rational(const Rational<T>& other)
		: numerator_ (other.numerator_)
		, denominator_ (other.denominator_)	{ }

	Rational(const AVRational& av_rational)
		: numerator_(av_rational.num)
		, denominator_(av_rational.den)	{ }

	Rational() : Rational(0, 1) {}
			
	T numerator() const { return numerator_; }

	T denominator() const { return denominator_; }

	bool operator==(const Rational<T>& other) const {
		return other.numerator_ * denominator_ == other.denominator_ * numerator_;
	}

	bool operator!=(const Rational<T>& other) const {
		return other.numerator_ * denominator_ != other.denominator_ * numerator_;
	}


	Rational<T> operator *(T by) const {
		return Rational<T>(numerator_ * by, denominator_);
	}

	Rational<T> operator /(T by) const {
		return Rational<T>(numerator_, denominator_ * by);
	}

	Rational<T> invert(const Rational<T>& other) const {
		return Rational<T>(other.denominator_, other.numerator_);
	}

	AVRational av() const{
		return av_make_q(numerator_, denominator_);
	}

};

}}