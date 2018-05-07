#ifndef __MEXCEPTION_H
#define __MEXCEPTION_H

#include <exception>

class mexception : public std::exception {
private:
	const char* w;
public:
	mexception(const char* v);// : w(v) {}
	virtual const char* what() const _GLIBCXX_USE_NOEXCEPT;// {return w;}
};

void pthrow(const char* s);
void delay(int ms);

#endif //__MEXCEPTION_H
