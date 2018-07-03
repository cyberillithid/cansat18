#include "picam.h"
#include <thread>
#include <raspicam/raspicam.h>
#include <sstream>

void PiCam::cam_thr(const char* tag, int frames) {
	std::unique_lock<std::mutex> lock1(m);
	unsigned char* data = new unsigned char[sz];

	for (int i = 0; i < frames; i++) {
		camera.grab();
		camera.retrieve(data);
		std::stringstream fn;
		fn << "/home/pi/" << tag << i << (isYuv ? ".yuv" : ".ppm");
		FILE* f = fopen(fn.str().c_str(), "wb");
		fprintf(f, "P%d\n%d %d 255\n", isYuv?0:6, width, height);
		fwrite(data, 1, sz, f);
		fclose(f);
	}
}

PiCam::PiCam(){
	sz = camera.getImageBufferSize();
	isYuv = camera.getFormat() == raspicam::RASPICAM_FORMAT_YUV420;
}

void PiCam::run_cam(const char* tag, int frames){
	jobs.push_back(std::thread(&PiCam::cam_thr, this, tag, frames));
}

void PiCam::die(){
/*	while (!jobs.empty()) {
		std::thread a = jobs.back();
		a.join();
		a.pop_back();
	}*/
}

PiCam::~PiCam(){
	die();
}
