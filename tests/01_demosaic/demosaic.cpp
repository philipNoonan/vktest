#include "demosaic.h"

std::string HvsTest::GetShadersPath()
{
	return getAssetPath() + "shaders/glsl/";
}

VkPipelineShaderStageCreateInfo HvsTest::LoadShader(std::string fileName, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shader_stage = {};
	shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage.stage = stage;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	shaderStage.module = vks::tools::loadShader(androidApp->activity->assetManager, fileName.c_str(), device);
#else
	shader_stage.module = vks::tools::loadShader(fileName.c_str(), device_);
#endif
	shader_stage.pName = "main";
	assert(shader_stage.module != VK_NULL_HANDLE);
	shader_modules_.push_back(shader_stage.module);
	return shader_stage;
}



void HvsTest::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo pipeline_cache_create_info = {};
	pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VK_CHECK_RESULT(vkCreatePipelineCache(device_, &pipeline_cache_create_info, nullptr, &pipeline_cache_));
}


VkResult HvsTest::CreateInstance()
{
	//this->settings.validation = enableValidation;

	// Validation can also be forced via a define

	std::cout << "creating instance " << std::endl;


	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = name_.c_str();
	app_info.pEngineName = name_.c_str();
	app_info.apiVersion = api_version_;

	std::vector<const char*> instance_extensions = { VK_KHR_SURFACE_EXTENSION_NAME };

	// Enable surface extensions depending on os
#if defined(_WIN32)
	instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
	instanceExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
	instanceExtensions.push_back(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
	instanceExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	instance_extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
	instanceExtensions.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
#endif

	// Get extensions supported by the instance and store for later use
	uint32_t ext_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
	if (ext_count > 0)
	{
		std::vector<VkExtensionProperties> extensions(ext_count);
		if (vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, &extensions.front()) == VK_SUCCESS)
		{
			for (VkExtensionProperties extension : extensions)
			{
				supported_instance_extensions_.push_back(extension.extensionName);
			}
		}
	}

	// Enabled requested instance extensions
	if (enabled_instance_extensions_.size() > 0) 
	{
		for (const char * enabled_extension : enabled_instance_extensions_) 
		{
			// Output message if requested extension is not available
			if (std::find(supported_instance_extensions_.begin(), supported_instance_extensions_.end(), enabled_extension) == supported_instance_extensions_.end())
			{
				std::cerr << "Enabled instance extension \"" << enabled_extension << "\" is not present at instance level\n";
			}
			instance_extensions.push_back(enabled_extension);
		}
	}

	VkInstanceCreateInfo instance_create_info = {};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pNext = NULL;
	instance_create_info.pApplicationInfo = &app_info;
	if (instance_extensions.size() > 0)
	{
		if (validation_)
		{
			instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		instance_create_info.enabledExtensionCount = (uint32_t)instance_extensions.size();
		instance_create_info.ppEnabledExtensionNames = instance_extensions.data();
	}

	// The VK_LAYER_KHRONOS_validation contains all current validation functionality.
	// Note that on Android this layer requires at least NDK r20
	const char* validation_layer_name = "VK_LAYER_KHRONOS_validation";
	if (validation_)
	{
		// Check if this layer is available at instance level
		uint32_t instance_layer_count;
		vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
		std::vector<VkLayerProperties> instance_layer_properties(instance_layer_count);
		vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layer_properties.data());
		bool validation_layer_present = false;
		for (VkLayerProperties layer : instance_layer_properties) {
			if (strcmp(layer.layerName, validation_layer_name) == 0) {
				validation_layer_present = true;
				break;
			}
		}
		if (validation_layer_present) {
			instance_create_info.ppEnabledLayerNames = &validation_layer_name;
			instance_create_info.enabledLayerCount = 1;
		} else {
			std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
		}
	}
	return vkCreateInstance(&instance_create_info, nullptr, &instance_);
}


bool HvsTest::InitVulkan(){

	std::cout << "init vulkan " << std::endl;
	VkResult err;

	// Vulkan instance
	err = CreateInstance();
	if (err) {
		vks::tools::exitFatal("Could not create Vulkan instance : \n" + vks::tools::errorString(err), err);

		return false;
	}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	vks::android::loadVulkanFunctions(instance);
#endif

	// If requested, we enable the default validation layers for debugging
	if (validation_)
	{
		// The report flags determine what type of messages for the layers will be displayed
		// For validating (debugging) an application the error and warning bits should suffice
		VkDebugReportFlagsEXT debug_report_flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		// Additional flags include performance info, loader and layer debug messages, etc.
		vks::debug::setupDebugging(instance_, debug_report_flags, VK_NULL_HANDLE);
	}	

	// Physical device
	uint32_t gpu_count = 0;
	// Get number of available physical devices
	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance_, &gpu_count, nullptr));
	if (gpu_count == 0) {
		vks::tools::exitFatal("No device with Vulkan support found", -1);
		return false;
	}	

	// Enumerate devices
	std::vector<VkPhysicalDevice> physical_devices(gpu_count);
	err = vkEnumeratePhysicalDevices(instance_, &gpu_count, physical_devices.data());
	if (err) {
		vks::tools::exitFatal("Could not enumerate physical devices : \n" + vks::tools::errorString(err), err);
		return false;
	}

	// GPU selection

	// Select physical device to be used for the Vulkan example
	// Defaults to the first device unless specified by command line
	uint32_t selected_device = 0;

// #if !defined(VK_USE_PLATFORM_ANDROID_KHR)
// 	// GPU selection via command line argument
// 	// if (commandLineParser.isSet("gpuselection")) {
// 	// 	uint32_t index = commandLineParser.getValueAsInt("gpuselection", 0);
// 	// 	if (index > gpuCount - 1) {
// 	// 		std::cerr << "Selected device index " << index << " is out of range, reverting to device 0 (use -listgpus to show available Vulkan devices)" << "\n";
// 	// 	} else {
// 	// 		selectedDevice = index;
// 	// 	}
// 	// }
// 	// if (commandLineParser.isSet("gpulist")) {
// 	// 	std::cout << "Available Vulkan devices" << "\n";
// 	// 	for (uint32_t i = 0; i < gpuCount; i++) {
// 	// 		VkPhysicalDeviceProperties deviceProperties;
// 	// 		vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
// 	// 		std::cout << "Device [" << i << "] : " << deviceProperties.deviceName << std::endl;
// 	// 		std::cout << " Type: " << vks::tools::physicalDeviceTypeString(deviceProperties.deviceType) << "\n";
// 	// 		std::cout << " API: " << (deviceProperties.apiVersion >> 22) << "." << ((deviceProperties.apiVersion >> 12) & 0x3ff) << "." << (deviceProperties.apiVersion & 0xfff) << "\n";
// 	// 	}
// 	// }
// #endif

	physical_device_ = physical_devices[selected_device];


	// Store properties (including limits), features and memory properties of the physical device (so that examples can check against them)
	vkGetPhysicalDeviceProperties(physical_device_, &device_properties_);
	vkGetPhysicalDeviceFeatures(physical_device_, &device_features_);
	vkGetPhysicalDeviceMemoryProperties(physical_device_, &device_memory_properties_);

	// Derived examples can override this to set actual features (based on above readings) to enable for logical device creation
	//getEnabledFeatures();

	// Vulkan device creation
	// This is handled by a separate class that gets a logical device representation
	// and encapsulates functions related to a device
	vulkan_device_ = new vks::VulkanDevice(physical_device_);

	VkResult res = vulkan_device_->createLogicalDevice(enabled_features_, enabled_device_extensions_, device_create_p_next_chain_);

	if (res != VK_SUCCESS) {
		vks::tools::exitFatal("Could not create Vulkan device: \n" + vks::tools::errorString(res), res);
		return false;
	}

	device_ = vulkan_device_->logicalDevice;

	// // Get a graphics queue from the device
	// vkGetDeviceQueue(device, vulkan_device_->queueFamilyIndices.graphics, 0, &compute.queue);
	// //std::cout << "g " << vulkan_device_->queueFamilyIndices.graphics << std::endl;
	// //std::cout << "c " << vulkan_device_->queueFamilyIndices.compute << std::endl;

	// // Find a suitable depth format
	// VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(physicalDevice, &depthFormat);
	// assert(validDepthFormat);

	// swapChain.connect(instance, physicalDevice, device);

	// // Create synchronization objects
	// VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
	// // Create a semaphore used to synchronize image presentation
	// // Ensures that the image is displayed before we start submitting new commands to the queue
	// VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));
	// // Create a semaphore used to synchronize command submission
	// // Ensures that the image is not presented until all commands have been submitted and executed
	// VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));

	// // Set up submit info structure
	// // Semaphores will stay the same during application lifetime
	// // Command buffer submission info is set by each example
	// submitInfo = vks::initializers::submitInfo();
	// submitInfo.pWaitDstStageMask = &submitPipelineStages;
	// submitInfo.waitSemaphoreCount = 1;
	// submitInfo.pWaitSemaphores = &semaphores.presentComplete;
	// submitInfo.signalSemaphoreCount = 1;
	// submitInfo.pSignalSemaphores = &semaphores.renderComplete;


	return true;

}

void HvsTest::InitGPU()
{

		//settings.overlay = false;

		//initVulkan();
		//setupWindow(nullptr);
		//prepare();


		// VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		// pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		// VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));

		// std::cout << "inited" << std::endl;


		std::cout << "preparing images..." << std::endl;

		PrepareTextures();

		SetupDescriptorPool();
		std::cout << "desc" << std::endl;

		PrepareCompute();
		std::cout << "compute" << std::endl;

		//prepared = true;
}



void HvsTest::DownloadImageBuffer(void* buffer, VkFormat format)
{
	calibrated_hypercube_.download(buffer, image_width_ * image_height_ * sizeof(float), format, vulkan_device_, compute.queue);
}



void HvsTest::SetCalibratedImage(py::array_t<float_t, py::array::c_style | py::array::forcecast> array)
{
	std::vector<float> numpy_array(image_width_ * image_height_, -1.0f);


	auto arr_obj_prop = array.request();
	float_t* vals = (float_t*)arr_obj_prop.ptr;

	for (int i = 0; i < image_width_ * image_height_; i++) {
		numpy_array[i] = *(vals + i);
	}
	calibrated_image_.fromBuffer(numpy_array.data(), image_width_ * image_height_ * sizeof(float_t), VK_FORMAT_R32_SFLOAT, image_width_, image_height_, vulkan_device_, compute.queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_LAYOUT_GENERAL);
}

py::array HvsTest::GetCalibratedHypercube()
{
	calibrated_hypercube_buffer_.resize(image_width_ * image_height_, 0);
	DownloadImageBuffer(calibrated_hypercube_buffer_.data(), VK_FORMAT_R32_SFLOAT);
	return py::array(calibrated_hypercube_buffer_.size(), calibrated_hypercube_buffer_.data());
}

// gets run everytime the type of shader chosen is changed
void HvsTest::BuildComputeCommandBuffer()
{
	// Flush the queue if we're rebuilding the command buffer after a pipeline change to ensure it's not currently in use
	vkQueueWaitIdle(compute.queue);

	VkCommandBufferBeginInfo cmd_buf_info = vks::initializers::commandBufferBeginInfo();

	VK_CHECK_RESULT(vkBeginCommandBuffer(compute.command_buffer, &cmd_buf_info));

	vkCmdBindPipeline(compute.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipeline);
	vkCmdBindDescriptorSets(compute.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipeline_layout, 0, 1, &compute.descriptor_set, 0, 0);

	vkCmdDispatch(compute.command_buffer, image_width_ / 32, image_height_ / 32, 1);

	vkEndCommandBuffer(compute.command_buffer);

}

void HvsTest::SetupDescriptorPool()
{

	std::vector<VkDescriptorPoolSize> pool_sizes = {
		// Compute pipelines uses a storage image for image reads and writes
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2),
	};

	VkDescriptorPoolCreateInfo descriptor_pool_info = vks::initializers::descriptorPoolCreateInfo(pool_sizes, 2);

	VK_CHECK_RESULT(vkCreateDescriptorPool(device_, &descriptor_pool_info, nullptr, &descriptor_pool_));

}

void HvsTest::PrepareTextures()
{
	std::vector<float> blank_image(image_width_ * image_height_, -1.0f);


	calibrated_hypercube_.fromBuffer(blank_image.data(), image_width_ * image_height_ * sizeof(float_t), VK_FORMAT_R32_SFLOAT, image_width_ / 4, image_height_ / 4, image_depth_, vulkan_device_, compute.queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_LAYOUT_GENERAL);

}



void HvsTest::PrepareCompute()
{


	// Create compute pipeline

	{
		std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings = {
			// images
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1),
		};

		VkDescriptorSetLayoutCreateInfo descriptor_layout = vks::initializers::descriptorSetLayoutCreateInfo(set_layout_bindings);
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device_, &descriptor_layout, nullptr, &compute.descriptor_set_layout));

		VkPipelineLayoutCreateInfo p_pipeline_layout_create_info =
			vks::initializers::pipelineLayoutCreateInfo(&compute.descriptor_set_layout, 1);

		VK_CHECK_RESULT(vkCreatePipelineLayout(device_, &p_pipeline_layout_create_info, nullptr, &compute.pipeline_layout)); 

		VkDescriptorSetAllocateInfo alloc_info =
			vks::initializers::descriptorSetAllocateInfo(descriptor_pool_, &compute.descriptor_set_layout, 1); 

		VK_CHECK_RESULT(vkAllocateDescriptorSets(device_, &alloc_info, &compute.descriptor_set)); 

		std::vector<VkWriteDescriptorSet> compute_write_descriptor_sets = {
			// images
			vks::initializers::writeDescriptorSet(compute.descriptor_set, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &calibrated_image_.descriptor),
			vks::initializers::writeDescriptorSet(compute.descriptor_set, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &calibrated_hypercube_.descriptor),
		};
		vkUpdateDescriptorSets(device_, compute_write_descriptor_sets.size(), compute_write_descriptor_sets.data(), 0, NULL);

		// Create compute shader pipelines
		VkComputePipelineCreateInfo compute_pipeline_create_info =
			vks::initializers::computePipelineCreateInfo(compute.pipeline_layout, 0); 

		std::string file_name = GetShadersPath() + "pipeline/" + shader_name_ + ".comp.spv";
		compute_pipeline_create_info.stage = LoadShader(file_name, VK_SHADER_STAGE_COMPUTE_BIT);

		VK_CHECK_RESULT(vkCreateComputePipelines(device_, pipeline_cache_, 1, &compute_pipeline_create_info, nullptr, &compute.pipeline));

	}

	// Separate command pool as queue family for compute may be different than graphics
	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.queueFamilyIndex = vulkan_device_->queueFamilyIndices.compute;
	cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(device_, &cmd_pool_info, nullptr, &compute.command_pool));

	// Create a command buffer for compute operations
	VkCommandBufferAllocateInfo cmd_buf_allocate_info =
		vks::initializers::commandBufferAllocateInfo(
			compute.command_pool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1);

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device_, &cmd_buf_allocate_info, &compute.command_buffer));

	// Fence for compute CB sync
	VkFenceCreateInfo fence_create_info = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VK_CHECK_RESULT(vkCreateFence(device_, &fence_create_info, nullptr, &compute.fence));

	// Build a single command buffer containing the compute dispatch commands
	BuildComputeCommandBuffer();
}

void HvsTest::Run()
{
	VkFenceCreateInfo fence_info = vks::initializers::fenceCreateInfo(VK_FLAGS_NONE);
	VkFence fence;
	VK_CHECK_RESULT(vkCreateFence(device_, &fence_info, nullptr, &fence));

	const VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;


	VkSubmitInfo compute_submit_info = vks::initializers::submitInfo();
	compute_submit_info.pWaitDstStageMask = &waitStageMask;
	compute_submit_info.commandBufferCount = 1;
	compute_submit_info.pCommandBuffers = &compute.command_buffer;
	VK_CHECK_RESULT(vkQueueSubmit(compute.queue, 1, &compute_submit_info, fence));
	VK_CHECK_RESULT(vkWaitForFences(device_, 1, &fence, VK_TRUE, UINT64_MAX));

	vkDestroyFence(device_, fence, nullptr);

}