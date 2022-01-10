#include "viewer.h"


/*
//int Viewer::main() {


	// OS specific macros for the pipeline main entry points
#if defined(_WIN32)
// Windows entry point
HvsPipeline* hvsPipeline;


//CameraGrabber* cameraGrabber;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hvsPipeline != NULL)
	{
		hvsPipeline->handleMessages(hWnd, uMsg, wParam, lParam);
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	for (int32_t i = 0; i < __argc; i++) { HvsPipeline::args.push_back(__argv[i]); };

	// Just pass the original pointer?
	//ximeaGrabber = new XimeaGrabber(2048, 1088, 30);
	//webcamGrabber = new WebcamGrabber(2048, 1088, 30);

	CameraGrabber cameraGrabber;
	cameraGrabber.set_source(CameraGrabber::INPUT_SOURCE::WEBCAM);
	cameraGrabber.set_width(2048);
	cameraGrabber.set_height(1088);


	cameraGrabber.start();
	//CameraGrabber& cgChild = *webcamGrabber;
	

	//cameraGrabber = webcamGrabber;

	//cgChild.start();

	hvsPipeline = new HvsPipeline();
	hvsPipeline->initVulkan();
	hvsPipeline->setupWindow(hInstance, WndProc);
	hvsPipeline->setCamera(cameraGrabber);
	hvsPipeline->prepare();
	hvsPipeline->renderLoop();
	delete(hvsPipeline);

	return 0;
}

#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
//#if defined(VK_EXAMPLE_XCODE_GENERATED)
*/
HvsPlugin* hvsPlugin;
int main(const int argc, const char* argv[])
{
	CameraGrabber cameraGrabber;
	cameraGrabber.set_source(CameraGrabber::INPUT_SOURCE::XIMEA);
	cameraGrabber.set_width(2048);
	cameraGrabber.set_height(1088);


	cameraGrabber.start();
	@autoreleasepool
	{
		for (size_t i = 0; i < argc; i++) { HvsPlugin::args.push_back(argv[i]); };
		hvsPlugin = new HvsPlugin();
		hvsPlugin->initVulkan();
		hvsPlugin->setupWindow(nullptr);
		hvsPlugin->setCamera(cameraGrabber);
		hvsPlugin->prepare();
		hvsPlugin->renderLoop();
		delete(hvsPlugin);
	}
	return 0;
}
/*
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
// Android entry point
HvsPlugin* hvsPlugin;
void android_main(android_app* state)
{
	hvsPlugin = new HvsPlugin();
	state->userData = hvsPlugin;
	state->onAppCmd = hvsPlugin::handleAppCommand;
	state->onInputEvent = HvsPlugin::handleAppInput;
	androidApp = state;
	vks::android::getDeviceConfig();
	hvsPlugin->renderLoop();
	delete(hvsPlugin);
}
#elif defined(_DIRECT2DISPLAY)
// Linux entry point with direct to display wsi
HvsPlugin* hvsPlugin;
static void handleEvent()
{
}
int main(const int argc, const char* argv[])
{
	for (size_t i = 0; i < argc; i++) { HvsPlugin::args.push_back(argv[i]); };
	hvsPlugin = new HvsPlugin();
	hvsPlugin->initVulkan();
	hvsPlugin->prepare();
	hvsPlugin->renderLoop();
	delete(hvsPlugin);
	return 0;
}
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
HvsPlugin* hvsPlugin;
static void handleEvent(const DFBWindowEvent* event)
{
	if (hvsPlugin != NULL)
	{
		hvsPlugin->handleEvent(event);
	}
}
int main(const int argc, const char* argv[])
{
	for (size_t i = 0; i < argc; i++) { HvsPlugin::args.push_back(argv[i]); };
	hvsPlugin = new HvsPlugin();
	hvsPlugin->initVulkan();
	hvsPlugin->setupWindow();
	hvsPlugin->prepare();
	hvsPlugin->renderLoop();
	delete(hvsPlugin);
	return 0;
}
#elif (defined(VK_USE_PLATFORM_WAYLAND_KHR) || defined(VK_USE_PLATFORM_HEADLESS_EXT))
HvsPlugin* hvsPlugin;
int main(const int argc, const char* argv[])
{
	for (size_t i = 0; i < argc; i++) { HvsPlugin::args.push_back(argv[i]); };
	hvsPlugin = new HvsPlugin();
	hvsPlugin->initVulkan();
	hvsPlugin->setupWindow();
	hvsPlugin->prepare();
	hvsPlugin->renderLoop();
	delete(hvsPlugin);
	return 0;
}
#elif defined(VK_USE_PLATFORM_XCB_KHR)
HvsPlugin* hvsPlugin;
static void handleEvent(const xcb_generic_event_t* event)
{
	if (hvsPlugin != NULL)
	{
		hvsPlugin->handleEvent(event);
	}
}
int main(const int argc, const char* argv[])
{
	for (size_t i = 0; i < argc; i++) { HvsPlugin::args.push_back(argv[i]); };
	hvsPlugin = new HvsPlugin();
	hvsPlugin->initVulkan();
	hvsPlugin->setupWindow();
	hvsPlugin->prepare();
	hvsPlugin->renderLoop();
	delete(hvsPlugin);
	return 0;
}
#else
#endif
#endif

*/



	//return 0;
//}