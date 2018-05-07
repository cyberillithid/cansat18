/* mexception.cpp
 */
#include "mexception.h"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

mexception::mexception(const char* v) : w(v) {}
const char* mexception::what() const _GLIBCXX_USE_NOEXCEPT
 {
	 return w;
 }

void pthrow(const char* s) {
	perror(s);
	throw mexception(s);
}

void delay(int ms) {
	usleep(ms*1000);
}


