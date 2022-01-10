/*
	Test 00_Demosaic for HVS

	Philip Noonan 08/12/2021

*/


#pragma once

#include "pybind11/pybind11.h"
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "vulkanexamplebase.h"

#include "camera_grabber.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define VERTEX_BUFFER_BIND_ID 0
#define ENABLE_VALIDATION true

namespace py = pybind11;





struct Vertex {
	float pos[3];
	float uv[2];
};

enum SHADER_SOURCE
{
	CALIBRATE = 0,
	DEMOSAIC = 1,
	CORRECTION = 2,
	HC2SRGB = 3,
	REMOSAIC = 4,
};
class HvsTest : public VulkanExampleBase
{
private:
	vks::Texture2D rawImage;
	vks::Texture2D whiteImage;
	vks::Texture2D darkImage;
	vks::Texture2D maskImage;

	vks::Texture2D calibratedImage;
	std::vector<float> m_calibratedImageBuffer;

	vks::Buffer calibrationUBO;

	int32_t m_image_width = 2048;
	int32_t m_image_height = 1088;

	float rho = 0.9f;

	std::ifstream m_fin;

	void prepareTextureTarget(vks::Texture* tex, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, uint32_t levels, VkFormat format);

	std::vector<uint16_t> numpyRaw;
	std::vector<uint16_t> numpyWhite;
	std::vector<uint16_t> numpyDark;
	std::vector<uint8_t> numpyMask;



public:
	struct {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertices;



	// Resources for the compute part of the plugin
	struct Compute {
		VkQueue queue;								// Separate queue for compute commands (queue family may differ from the one used for graphics)
		VkCommandPool commandPool;					// Use a separate command pool (queue family may differ from the one used for graphics)
		VkCommandBuffer commandBuffer;				// Command buffer storing the dispatch commands and barriers
		VkSemaphore semaphore;                      // Execution dependency between compute & graphic submission
		VkFence fence;
		VkDescriptorSetLayout descriptorSetLayout;	// Compute shader binding layout
		VkDescriptorSet descriptorSet;				// Compute shader bindings
		VkPipelineLayout pipelineLayout;			// Layout of the compute pipeline
		VkPipeline pipeline;			// Compute pipelines for image filters
		int32_t pipelineIndex = 0;					// Current image filtering compute pipeline index
	} compute;


	std::vector<std::string> shaderNames;


	HvsTest() : VulkanExampleBase(ENABLE_VALIDATION)
	{
		initVulkan();

		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));

		std::cout << "inited" << std::endl;

		setupDescriptorPool();
		std::cout << "desc" << std::endl;

		prepareCompute();
		std::cout << "compute" << std::endl;



	}


	~HvsTest()
	{

		// Compute

		vkDestroyPipeline(device, compute.pipeline, nullptr);
		vkDestroyPipelineLayout(device, compute.pipelineLayout, nullptr);
		
		//vkDestroyPipelineLayout(device, compute.pipelineLayout, nullptr);


		vkDestroyDescriptorSetLayout(device, compute.descriptorSetLayout, nullptr);
		
		//vkDestroyDescriptorSetLayout(device, compute.descriptorSetLayout, nullptr);

		vkDestroySemaphore(device, compute.semaphore, nullptr);
		vkDestroyCommandPool(device, compute.commandPool, nullptr);

		rawImage.destroy();
		whiteImage.destroy();
		darkImage.destroy();
		maskImage.destroy();

		calibratedImage.destroy();

	}


	void loadPNG16(std::string fileName);
	void loadFileU16(std::string fileName);
	void loadFileF32(std::string fileName);

	void loadImageBuffer(void* buffer, int32_t m_image_width, int32_t m_image_height, VkFormat format);
	void downloadImageBuffer(void* buffer, VkFormat format);

	void buildComputeCommandBuffer();

	void setupDescriptorPool();

	void prepareTextures();
	void prepareCompute();

	void setWhiteValue(uint16_t val);
	void setDarkValue(uint16_t val);
	void setMaskValue(uint8_t val);

	// https://pybind11.readthedocs.io/en/stable/advanced/pycpp/numpy.html#arrays 
	void setWhiteImage(py::array_t<uint16_t, py::array::c_style | py::array::forcecast> array);
	void setDarkImage(py::array_t<uint16_t, py::array::c_style | py::array::forcecast> array);
	void setMaskImage(py::array_t<uint8_t, py::array::c_style | py::array::forcecast> array);
	void setRawImage(py::array_t<uint16_t, py::array::c_style | py::array::forcecast> array);

	py::array getCalibratedImage();




};



PYBIND11_MODULE(calibration, m) {
	py::class_<VulkanExampleBase>(m, "VulkanExampleBase");
	py::class_<HvsTest, VulkanExampleBase>(m, "HvsTest")
		.def(py::init<>())
		.def("loadPNG16", &HvsTest::loadPNG16)
		.def("loadFileU16", &HvsTest::loadFileU16)
		.def("loadFileF32", &HvsTest::loadFileF32)
		.def("loadImageBuffer", &HvsTest::loadImageBuffer)
		.def("setWhiteValue", &HvsTest::setWhiteValue) 
		.def("setDarkValue", &HvsTest::setDarkValue)
		.def("setMaskValue", &HvsTest::setMaskValue)
		.def("setWhiteImage", &HvsTest::setWhiteImage)
		.def("setDarkImage", &HvsTest::setDarkImage)
		.def("setMaskImage", &HvsTest::setMaskImage)
		.def("setRawImage", &HvsTest::setRawImage)
		.def("getCalibratedImage", &HvsTest::getCalibratedImage)
		;
}

