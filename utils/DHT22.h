#include <unistd.h>
#include <stdint.h>

class DHT22 {
private:
int retries;
int pin;
uint32_t val;
public:
DHT22(int wPin);
bool fetch();
uint32_t get();
};
