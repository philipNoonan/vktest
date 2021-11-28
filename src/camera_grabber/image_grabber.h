//#pragma once
//
//#include "camera_grabber.h"
//
//#include <iostream>
//#include <stdint.h>
//
//#include "opencv2/opencv.hpp"
//
//
//
//
//class ImageGrabber : public CameraGrabber
//{
//private:
//	std::vector<cv::String> m_fn;
//	int32_t m_file_index = 0;
//	int32_t m_number_of_files;
//	cv::Mat m_image;
//
//	void open_image(int32_t fileID);
//
//
//
//public:
//	ImageGrabber();
//
//	~ImageGrabber();
//
//	void glob_files(std::string dir);
//
//
//	//void frame_to_buffer();
//
//	uint16_t* get_frame();
//	uint16_t* get_frame(int32_t fileID);
//
//};