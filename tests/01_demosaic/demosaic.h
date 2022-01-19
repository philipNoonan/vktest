#ifndef TESTS_DEMOSAIC_H_
#define TESTS_DEMOSAIC_H_

/*
	Test 00_Demosaic for HVS

	Philip Noonan 08/12/2021


*/

/*
* Using extracts from ...
* Vulkan Example base class
*
* Copyright (C) by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/



#include "pybind11/pybind11.h"
#include <pybind11/numpy.h>
#include <pybind11/stl.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <array>
#include <unordered_map>
#include <numeric>
#include <ctime>
#include <iostream>
#include <chrono>
#include <random>
#include <algorithm>
#include <sys/stat.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <numeric>
#include <array>

#define VK_ENABLE_BETA_EXTENSIONS

#include "vulkan/vulkan.h"

#include "keycodes.hpp"
#include "VulkanTools.h"
#include "VulkanDebug.h"
#include "VulkanUIOverlay.h"
#include "VulkanSwapChain.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanTexture.h"

#include "VulkanInitializers.hpp"
#include "camera.hpp"
#include "benchmark.hpp"

//#include "camera_grabber.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define VERTEX_BUFFER_BIND_ID 0
#define ENABLE_VALIDATION true

namespace py = pybind11;


class HvsTest
{
private:

	// Vulkan Variables
	std::string name_ = "HvsTest";
	std::string shader_name_ = "demosaic";

	uint32_t api_version_ = VK_API_VERSION_1_0;
	bool validation_ = true;
	// Vulkan instance, stores all per-application states
	VkInstance instance_;
	// Physical device (GPU) that Vulkan will use
	VkPhysicalDevice physical_device_;
	// Stores physical device properties (for e.g. checking device limits)
	VkPhysicalDeviceProperties device_properties_;
	// Stores the features available on the selected physical device (for e.g. checking if a feature is available)
	VkPhysicalDeviceFeatures device_features_;
	// Stores all available memory (type) properties for the physical device
	VkPhysicalDeviceMemoryProperties device_memory_properties_;
	/** @brief Set of physical device features to be enabled for this example (must be set in the derived constructor) */
	VkPhysicalDeviceFeatures enabled_features_{};

	std::vector<std::string> supported_instance_extensions_;

	/** @brief Set of device extensions to be enabled for this example (must be set in the derived constructor) */
	std::vector<const char*> enabled_device_extensions_;
	std::vector<const char*> enabled_instance_extensions_;

	/** @brief Optional pNext structure for passing extension structures to device creation */
	void* device_create_p_next_chain_ = nullptr;
	/** @brief Logical device, application's view of the physical device (GPU) */
	VkDevice device_;

	/** @brief Encapsulated physical and logical vulkan device */
	vks::VulkanDevice *vulkan_device_;

	// Descriptor set pool
	VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
	// List of shader modules created (stored for cleanup)
	std::vector<VkShaderModule> shader_modules_;

	VkPipelineCache pipeline_cache_;



	// Test Specific Variables
	int32_t image_width_ = 2048;
	int32_t image_height_ = 1088;
	int32_t image_depth_ = 16;
	
	vks::Texture2D raw_image_;

	vks::Texture2D calibrated_image_;
	vks::Texture2DArray calibrated_hypercube_;

	std::vector<float> calibrated_hypercube_buffer_;

	std::ifstream fin_;




	// Functions
	std::string GetShadersPath();
	VkPipelineShaderStageCreateInfo LoadShader(std::string fileName, VkShaderStageFlagBits stage);
	void CreatePipelineCache();
	VkResult CreateInstance();
	bool InitVulkan();
	void PrepareTextureTarget(vks::Texture* tex, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, uint32_t levels, VkFormat format);


public:

	// Resources for the compute part of the plugin
	struct Compute {
		VkQueue queue;								// Separate queue for compute commands (queue family may differ from the one used for graphics)
		VkCommandPool command_pool;					// Use a separate command pool (queue family may differ from the one used for graphics)
		VkCommandBuffer command_buffer;				// Command buffer storing the dispatch commands and barriers
		VkSemaphore semaphore;                      // Execution dependency between compute & graphic submission
		VkFence fence;
		VkDescriptorSetLayout descriptor_set_layout;	// Compute shader binding layout
		VkDescriptorSet descriptor_set;				// Compute shader bindings
		VkPipelineLayout pipeline_layout;			// Layout of the compute pipeline
		VkPipeline pipeline;			// Compute pipelines for image filters
		int32_t pipeline_index = 0;					// Current image filtering compute pipeline index
	} compute;

	HvsTest()
	{
		enabled_instance_extensions_.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		enabled_device_extensions_.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
		enabled_instance_extensions_.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

		InitVulkan();
		CreatePipelineCache();

				std::cout << "starting..." << std::endl;

		if (vulkan_device_->enableDebugMarkers) {
			vks::debugmarker::setup(device_);
		}

	// Get a compute queue from the device
		vkGetDeviceQueue(device_, vulkan_device_->queueFamilyIndices.compute, 0, &compute.queue);

	}


	~HvsTest()
	{

		vkDestroyPipeline(device_, compute.pipeline, nullptr);
		vkDestroyPipelineLayout(device_, compute.pipeline_layout, nullptr);
		
		vkDestroyDescriptorSetLayout(device_, compute.descriptor_set_layout, nullptr);

		vkDestroyCommandPool(device_, compute.command_pool, nullptr);

		vkDestroyFence(device_, compute.fence, nullptr);



		calibrated_image_.destroy();
		calibrated_hypercube_.destroy();


		std::cout << "cleared up" << std::endl;

	}

	void InitGPU();

	void DownloadImageBuffer(void* buffer, VkFormat format);

	void BuildComputeCommandBuffer();

	void SetupDescriptorPool();

	void PrepareTextures();
	void PrepareCompute();


	void SetCalibratedImage(py::array_t<float_t, py::array::c_style | py::array::forcecast> array);

	void Run();

	py::array GetCalibratedHypercube();


};


PYBIND11_MODULE(demosaic, m) {
	py::class_<HvsTest>(m, "HvsTest")
		.def(py::init<>())
		.def("setCalibratedImage", &HvsTest::SetCalibratedImage)
		.def("run", &HvsTest::Run)
		.def("getCalibratedHypercube", &HvsTest::GetCalibratedHypercube, py::return_value_policy::reference)
		.def("initGPU", &HvsTest::InitGPU)
		;
}

#endif // TESTS_DEMOSAIC_H_