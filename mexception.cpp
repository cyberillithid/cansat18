/* mexception.cpp
 */
#include "mexception.h"

mexception::mexception(const char* v) : w(v) {}
virtual const char* mexception::what() const _GLIBCXX_USE_NOEXCEPT
 {
	 return w;
 }


