#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <raspicam/raspicam.h>
class PiCam {
private:
	std::thread *thr;
	raspicam::RaspiCam camera;
	size_t sz, width, height;
	bool isYuv;
	std::atomic<bool> die;
	void run();
	void take(char* fn);
public:
	PiCam();
	void stop();
	~PiCam();
};
