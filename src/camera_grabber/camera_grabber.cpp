#include "camera_grabber.h"

CameraGrabber::CameraGrabber() {

}

CameraGrabber::~CameraGrabber() {

}

void CameraGrabber::set_width(int32_t w) {
	m_image_width = w;
};
int32_t CameraGrabber::get_width() {
	return m_image_width;
}

void CameraGrabber::set_height(int32_t h) {
	m_image_height = h;
};

int32_t CameraGrabber::get_height(){
	return m_image_height;
}

void CameraGrabber::set_source(CameraGrabber::INPUT_SOURCE source) {
	m_input_source = source;
}

void CameraGrabber::start() {
	// placeholder virt function
		// depending on currently chose source, either open the ximea handle, webcam, or hevc file
	//int codec = cv::VideoWriter::fourcc('P', '0', '1', '0');

	// if ximea is chosen
	switch (m_input_source) {

	case INPUT_SOURCE::XIMEA:
		printf("Opening first camera...\n");
		CE(xiOpenDevice(0, &xiH));

		printf("Setting exposure time to 10ms...\n");
		CE(xiSetParamInt(xiH, XI_PRM_EXPOSURE, (m_exposure_time * 1000)));
		CE(xiSetParamInt(xiH, XI_PRM_IMAGE_DATA_FORMAT, XI_MONO16));

		CE(xiSetParamInt(xiH, XI_PRM_ACQ_TIMING_MODE, 1));
		//CE(xiSetParamInt(xiH, XI_PRM_FRAMERATE, 30));

		printf("Starting acquisition...\n");
		CE(xiStartAcquisition(xiH));

		memset(&m_image, 0, sizeof(m_image));
		m_image.size = sizeof(XI_IMG);
		CE(xiGetImage(xiH, 5000, &m_image)); // getting next image from the camera opened

		m_image_height = m_image.height;
		m_image_width = m_image.width;

		break;

	case INPUT_SOURCE::WEBCAM:
		if (!m_cap.open(0)) {
			std::cout << "error could not open webcam" << std::endl;
		}
		else {
			// try setting 1080p
			m_cap.set(cv::CAP_PROP_FRAME_WIDTH, m_image_width);
			m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, m_image_height);

			// read what is available to us
			//m_image_width = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
			//m_image_height = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
		}

		break;

	//case INPUT_SOURCE::VIDEO_FILE:
	//	//if (!m_cap.open(m_filename)) {
	//	//	std::cout << "error could not open file" << std::endl;
	//	//}			

	//	m_fin.open(m_filename, std::ifstream::in | std::ifstream::binary);
	//	if (m_fin.is_open())
	//	{
	//		printf("fin is open\n");

	//	}

	//	//m_cap.set(cv::CAP_PROP_FOURCC, codec);
	//	//m_imageWidth = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
	//	//m_imageHeight = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
	//	//cv::VideoWriter::fourcc
	//	//m_cap.set(cv::CAP_PROP_FORMAT, -1); // this sets to raw stream of Mat 8uC1, i think
	//	break;

	//case INPUT_SOURCE::IMAGE_FILE:
	//	//if (!m_cap.open(m_filename)) {
	//	//	std::cout << "error could not open file" << std::endl;
	//	//}			

	//	m_fin.open(m_filename, std::ifstream::in | std::ifstream::binary);
	//	if (m_fin.is_open())
	//	{
	//		printf("fin is open\n");

	//	}

	//	break;

	default:

		break;

	}


}

void CameraGrabber::stop() {
	
}

uint16_t* CameraGrabber::get_buffer() {

	std::vector<cv::Mat> rgb;
	uint64_t a0;
	uint64_t* c0ptr;
	uint8_t* a;
	uint16_t* b;

	switch (m_input_source) {
	case INPUT_SOURCE::DEMO:

		//// get time and log it to frame time log file
		//a0 = epochTimeDouble();
		//AppFrameTimeLogger(std::to_string(a0));

		//a = (uint8_t*)&a0;

		//// is this safe to do??
		//uint16_t* tsPtr;
		//tsPtr = (uint16_t*)m_logo.data;

		//*tsPtr = (uint16_t)*a;
		//*(tsPtr + 1) = (uint16_t) * (a + 1);
		//*(tsPtr + 2) = (uint16_t) * (a + 2);
		//*(tsPtr + 3) = (uint16_t) * (a + 3);
		//*(tsPtr + 4) = (uint16_t) * (a + 4);
		//*(tsPtr + 5) = (uint16_t) * (a + 5);
		//*(tsPtr + 6) = (uint16_t) * (a + 6);
		//*(tsPtr + 7) = (uint16_t) * (a + 7);

		//return (uint16_t*)m_logo.data;

	case INPUT_SOURCE::XIMEA:
		xiGetImage(xiH, 600, &m_image);
		//epochTimeDouble();
		//uint16_t* tsPtr;
		//tsPtr = (uint16_t*)image.bp;

		//tsPtr[0] = 8008;

		return (uint16_t*)m_image.bp;

	case INPUT_SOURCE::WEBCAM:
		m_cap.read(m_mat);
		cv::cvtColor(m_mat, m_mat, cv::COLOR_RGB2GRAY);
		cv::resize(m_mat, m_mat_large, cv::Size(m_image_width, m_image_height));
		m_mat_large.convertTo(m_mat_16, CV_16UC1, 4.0);

		return (uint16_t*)m_mat_16.data;

	//case INPUT_SOURCE::VIDEO_FILE:
	//	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	//	m_fin.read(m_bufferInputFile, 2048 * 1088 * 3);

	//	// the first 8 words of each buffer should be the 8 words to make up the uint64_t timestamp

	//	b = (uint16_t*)&m_bufferInputFile[0];

	//	//std::cout << ((*b) >> 6) << std::endl;
	//	//std::cout << (*(b + 1) >> 6) << std::endl;
	//	//std::cout << (*(b + 2) >> 6) << std::endl;
	//	//std::cout << (*(b + 3) >> 6) << std::endl;
	//	//std::cout << (*(b + 4) >> 6) << std::endl;
	//	//std::cout << (*(b + 5) >> 6) << std::endl;
	//	//std::cout << (*(b + 6) >> 6) << std::endl;
	//	//std::cout << (*(b + 7) >> 6) << std::endl;

	//	uint8_t buffer[8];

	//	for (int i = 0; i < 8; i++) {
	//		buffer[i] = (uint8_t)(*(b + i) >> 6);
	//	}
	//	c0ptr = (uint64_t*)buffer;
	//	std::cout << "time : " << *c0ptr << std::endl;


	//	if (m_fin.eof()) {
	//		m_fin.clear();
	//		m_fin.seekg(0, std::ios::beg);
	//		std::cout << "returning to start of file " << std::endl;
	//	}

	//	//m_frame = cv::Mat(1088, 2048, CV_)
	//	//m_cap.read(m_frame);
	//	//std::cout << m_frame.channels() << std::endl;
	//	//std::cout << type2str(m_frame.type()) << std::endl;
	//	//std::cout << m_frame.cols << " " << m_frame.rows << std::endl;
	//	//cv::split(m_frame, rgb);

	// //   cv::imshow("r", rgb[0]);
	//	//cv::imshow("g", rgb[1]);
	//	//cv::imshow("b", rgb[2]);

	//	//cv::waitKey(0);

	//	return (uint16_t*)&m_bufferInputFile[0];

	//case INPUT_SOURCE::IMAGE_FILE:
	//	if (m_sourceChanged) {
	//		m_fin.read(m_bufferInputImageFile, 2048 * 1088 * sizeof(uint16_t));
	//	}

	//	return (uint16_t*)&m_bufferInputImageFile[0];

	default:
		return (nullptr);
	}
}