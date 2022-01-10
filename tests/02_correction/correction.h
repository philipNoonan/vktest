/*
	Test 00_Demosaic for HVS

	Philip Noonan 08/12/2021

*/


#pragma once

#include "pybind11/pybind11.h"


#include "vulkanexamplebase.h"

#include "opencv2/opencv.hpp"

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
	vks::Texture2D calibrated;

	vks::Texture2DArray hypercubeCalibrated;


	int32_t m_image_width;
	int32_t m_image_height;
	int32_t m_number_bands = 16;
	int32_t m_number_bands_corrected = 10;

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

		calibrated.destroy();
		hypercubeCalibrated.destroy();
	}

	// Prepare a texture target that is used to store compute shader calculations
	void prepareTextureTarget(vks::Texture* tex, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, uint32_t levels, VkFormat format);

	void loadFileU16(std::string fileName);
	void loadFileF32(std::string fileName);

	void loadImageBuffer(void* buffer, int32_t m_image_width, int32_t m_image_height, VkFormat format);
	void downloadImageBuffer(void* buffer, VkFormat format);

	// gets run everytime the type of shader chosen is changed
	void buildComputeCommandBuffer();

	void setupDescriptorPool();

	void prepareCompute();

	void run();


};



PYBIND11_MODULE(demosaic, m) {
	py::class_<VulkanExampleBase>(m, "VulkanExampleBase");
	py::class_<HvsTest, VulkanExampleBase>(m, "HvsTest", py::buffer_protocol())
		.def(py::init<>())
		.def("loadFileU16", &HvsTest::loadFileU16)
		.def("loadFileF32", &HvsTest::loadFileF32)
		.def("loadImageBuffer", &HvsTest::loadImageBuffer);
}

