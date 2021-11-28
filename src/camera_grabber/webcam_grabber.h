//#pragma once
//
//
//#include "camera_grabber.h"
//
//
//#include <iostream>
//#include <stdint.h>
//
//#include "opencv2/opencv.hpp"
//
//
//
//class WebcamGrabber : public CameraGrabber
//{
//private:
//	int32_t m_exposureTime = 30;
//
//	cv::VideoCapture m_cap;
//	cv::Mat m_image;
//
//
//public:
//	WebcamGrabber(int32_t width, int32_t height, int32_t exposureTime);
//
//	~WebcamGrabber();
//
//	void start();
//
//	void stop();
//
//	void frame_to_buffer();
//
//	void read_frame();
//
//};