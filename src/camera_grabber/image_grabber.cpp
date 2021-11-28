//
//#include "image_grabber.h"
//
//
//
// 
//ImageGrabber::ImageGrabber() {
//
//}
//
//
//ImageGrabber::~ImageGrabber() {
//
//}
//
//void ImageGrabber::glob_files(std::string dir) {
//	// Check if these read in both 16 bit and 8 bit pngs correctly
//	cv::glob(dir + "/*.png", m_fn, false);
//	m_number_of_files = m_fn.size();
//
//	//std::vector<cv::Mat> images;
//	//size_t count = fn.size(); 
//	//for (size_t i = 0; i < count; i++)
//	//	images.push_back(cv::imread(fn[i]));
//}
//
//void ImageGrabber::open_image(int32_t fileID) {
//	m_image = cv::imread(m_fn[fileID]);
//}
//
////void ImageGrabber::frame_to_buffer() {
////	m_imageBuffer = (uint16_t*)m_image.data;
////}
//
//// Returns buffer from next image in directory sequence. Loops to start when last image is read in.
//uint16_t* ImageGrabber::get_frame() {
//	open_image(m_file_index);
//	m_file_index >= (m_number_of_files - 1) ? m_file_index = 0 : m_file_index++;
//	return (uint16_t*)m_image.data;
//}
//
//// Returns buffer from next image in directory sequence. Loops to start when last image is read in.
//uint16_t* ImageGrabber::get_frame(int32_t fileID) {
//	open_image(fileID);
//	return (uint16_t*)m_image.data;
//}