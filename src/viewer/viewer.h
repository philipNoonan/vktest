#pragma once

#include <stdint.h>

#include "vulkanexamplebase.h"



#include "camera_grabber.h"
#include "ximea_grabber.h"
#include "webcam_grabber.h"

#include "pipeline.h"




//#if defined(_WIN32)

//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int);

//#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
//#if defined(VK_EXAMPLE_XCODE_GENERATED)
int main(const int argc, const char* argv[]);
//#endif
//#endif