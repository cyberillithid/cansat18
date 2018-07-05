#include "picam.h"
#include <thread>
#include <raspicam/raspicam.h>
#include <sstream>
#include <jpeglib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

void PiCam::take(char* fn) {
	unsigned char* data = new unsigned char[sz];
	camera.grab();
	camera.retrieve(data);
/*	FILE* f = fopen(fnppm, "wb");
	fprintf(f, "P%d\n%d %d 255\n", isYuv?0:6, width, height);
	fwrite(data, 1, sz, f);
	fclose(f);*/
	// get time
	FILE* outfile = fopen(fn, "wb");
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 75, true);
	jpeg_start_compress(&cinfo, true);
	JSAMPROW row_pointer[1];
	int r_stride = 3*width;
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &data[cinfo.next_scanline*r_stride];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(outfile);
	delete[] data;
}

void PiCam::run() {
	char fn[1000];
	struct tm *ti;
	time_t rt;
	while (!die) {
		time (&rt);
		ti = localtime(&rt);
		strftime(fn, sizeof(fn), "/home/pi/vid/%H %M %S.jpg", ti);
		take(fn);
		sleep(5);
	}
}

PiCam::PiCam(){
	camera.open();
	width = camera.getWidth();
	height = camera.getHeight();
	sz = camera.getImageBufferSize();
	isYuv = camera.getFormat() == raspicam::RASPICAM_FORMAT_YUV420;
	thr = new std::thread(&PiCam::run, this);
}

PiCam::~PiCam(){
	stop();
}

void PiCam::stop(){
	die = true;
	if (thr) {
		thr->join();
		delete thr;
		thr = NULL;
	}
}

/*
int main (int argc, char** argv){
PiCam p;
p.run();
//p.take("/home/pi/a.jpg", "/home/pi/a.ppm");
return 0;
}
*/
