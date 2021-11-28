//
//#include "ximea_grabber.h"
//#ifdef WIN32
//#include <xiApi.h>       // Windows
//#else
//#include <m3api/xiApi.h> // Linux, OSX
//#endif
//
//#include <iostream>
//
//
//
// 
//XimeaGrabber::XimeaGrabber(int32_t width, int32_t height, int32_t exposureTime) {
//    m_imageWidth = width;
//    m_imageHeight = height;
//    m_exposureTime = exposureTime;
//		// list available devices?
//}
//
//
//XimeaGrabber::~XimeaGrabber() {
//    xiCloseDevice(xiH);
//}
//
//void XimeaGrabber::start() {
//    printf("Opening first camera...\n");
//    CE(xiOpenDevice(0, &xiH));
//
//    printf("Setting exposure time to 10ms...\n");
//    CE(xiSetParamInt(xiH, XI_PRM_EXPOSURE, (m_exposureTime * 1000)));
//    CE(xiSetParamInt(xiH, XI_PRM_IMAGE_DATA_FORMAT, XI_MONO16));
//
//    CE(xiSetParamInt(xiH, XI_PRM_ACQ_TIMING_MODE, 1));
//	//CE(xiSetParamInt(xiH, XI_PRM_FRAMERATE, 30));
//
//    printf("Starting acquisition...\n");
//    CE(xiStartAcquisition(xiH));
//
//    memset(&m_image, 0, sizeof(m_image));
//    m_image.size = sizeof(XI_IMG);
//    CE(xiGetImage(xiH, 5000, &m_image)); // getting next image from the camera opened
//
//    if (m_imageHeight != m_image.height || m_imageWidth != m_image.width) {
//        printf("ERROR... image dimensions not as requested.\n");
//    }
//}
//
//void XimeaGrabber::stop() {
//    printf("Stopping acquisition...\n");
//    xiStopAcquisition(xiH);
//}
//
//void XimeaGrabber::frame_to_buffer() {
//    m_imageBuffer = (uint16_t *)m_image.bp;
//}
//
//void XimeaGrabber::read_frame(int32_t blockingTime = 600) {
//    xiGetImage(xiH, blockingTime, &m_image);
//}