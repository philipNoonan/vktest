//
//#include "webcam_grabber.h"
//
//
//#include <iostream>
//
//
//
// 
//WebcamGrabber::WebcamGrabber(int32_t width, int32_t height, int32_t exposureTime) {
//	m_imageWidth = width;
//	m_imageHeight = height;
//	m_exposureTime = exposureTime;
//		// list available devices?
//}
//
//WebcamGrabber::~WebcamGrabber()	{
//		
//}
//
//void WebcamGrabber::start() {
//	if (!m_cap.open(0)) {
//		std::cout << "error could not open webcam" << std::endl;
//	}
//	else {
//		// try setting 1080p
//		m_cap.set(cv::CAP_PROP_FRAME_WIDTH, m_imageWidth);
//		m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, m_imageHeight);
//
//		// read what is available to us
//		m_imageWidth = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
//		m_imageHeight = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
//	}
//}
//
//void WebcamGrabber::stop() {
//		
//}
//
//void WebcamGrabber::frame_to_buffer() {
//	m_imageBuffer = (uint16_t *)m_image.data;
//}
//
//void WebcamGrabber::read_frame() {
//	m_cap.read(m_image);
//}