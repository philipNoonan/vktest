#pragma once

#ifdef WIN32
#include <xiApi.h>       // Windows
#else
#include <m3api/xiApi.h> // Linux, OSX
#endif


#include "opencv2/opencv.hpp"

#include <stdint.h>
#include <iostream>

#define CE(func) {XI_RETURN stat = (func); if (XI_OK!=stat) {printf("Error:%d returned from function:"#func"\n",stat);throw "Error";}}

class CameraGrabber
{

public:

	enum INPUT_SOURCE
	{
		DEMO = 0,
		XIMEA = 1,
		WEBCAM = 2,
		VIDEO_FILE = 3,
		IMAGE_FILE = 4
	};

	CameraGrabber();

	~CameraGrabber();

	void set_source(INPUT_SOURCE source);

	void start();

	void stop();

	uint16_t* get_buffer();

	void set_width(int32_t w);
	int32_t get_width();

	void set_height(int32_t h);
	int32_t get_height();

private:



	HANDLE xiH = NULL;
	XI_IMG m_image;
	int32_t m_exposure_time = 20;

    cv::VideoCapture m_cap;
    cv::Mat m_mat;
	cv::Mat m_mat_large;
	cv::Mat m_mat_16;



	INPUT_SOURCE m_input_source;

	int32_t m_image_width;
	int32_t m_image_height;
	
	uint16_t* m_image_buffer;




};