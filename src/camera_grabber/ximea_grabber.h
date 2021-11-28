//#pragma once
//
//
//#ifdef WIN32
//#include <xiApi.h>       // Windows
//#else
//#include <m3api/xiApi.h> // Linux, OSX
//#endif
//
//#include "camera_grabber.h"
//
//#include <iostream>
//#include <stdint.h>
//
//
//#define CE(func) {XI_RETURN stat = (func); if (XI_OK!=stat) {printf("Error:%d returned from function:"#func"\n",stat);throw "Error";}}
//
//
//class XimeaGrabber : public CameraGrabber
//{
//private:
//	HANDLE xiH = NULL;
//	XI_IMG m_image;
//	int32_t m_exposureTime = 30;
//
//
//public:
//	XimeaGrabber(int32_t width, int32_t height, int32_t exposureTime);
//
//	~XimeaGrabber();
//
//	void start();
//
//	void stop();
//
//	void frame_to_buffer();
//
//	void read_frame(int32_t blockingTime);
//
//};