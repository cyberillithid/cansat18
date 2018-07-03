#include <vector>
#include <thread>
#include <mutex>
#include <raspicam/raspicam.h>
class PiCam {
private:
	std::vector<std::thread> jobs;
	raspicam::RaspiCam camera;
	size_t sz, width, height;
	bool isYuv;
	std::mutex m;
	void cam_thr(const char* t, int fr);
public:
	PiCam();
	void run_cam (const char* tag, int frames=1);
	void die();
	~PiCam();
};
