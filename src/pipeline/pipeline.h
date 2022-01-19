/*
	Pipeline for HVS

	Philip Noonan 11/11/2021

*/

//// PLUGIN EXAMPLE TEMPLATE INSPIRED FROM ...
/*
* Vulkan Example - Compute shader image processing
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once


#include "vulkanexamplebase.h"

#include "opencv2/opencv.hpp"

//#include "ximea_grabber.h"
//#include "webcam_grabber.h"
#include "camera_grabber.h"

#define VERTEX_BUFFER_BIND_ID 0
#define ENABLE_VALIDATION true


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
class HvsPipeline : public VulkanExampleBase
{
private:
	vks::Texture2D textureRaw;
	vks::Texture2D textureCalibrated;

	vks::Texture2D calibrated;

	vks::Texture2D whiteReference;
	vks::Texture2D darkReference;

	vks::Texture2D fovMask;

	vks::Texture2D tiled;
	vks::Texture2D tiledInput;

	vks::Texture2DArray hypercubeCalibrated;
	vks::Texture2DArray hypercubeCalibratedColorCorrected;

	vks::Texture2D sRGB;

	vks::Texture2D correctionMatrix;
	vks::Texture2D colorMatrix;

	vks::Buffer calibrateUBO;
	vks::Buffer correctionUBO;
	vks::Buffer hc2srgbUBO;

	vks::Buffer renderUBO;


	cv::VideoCapture m_cap;

	CameraGrabber m_cam_grab;

	cv::Mat m_frame;
	cv::Mat m_frame4;
	int32_t m_image_width;
	int32_t m_image_height;
	int32_t m_image_depth = 1;
	int32_t m_number_bands = 16;
	int32_t m_number_bands_corrected = 10;

public:
	struct {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertices;

	// Resources for the graphics part of the plugin
	struct {
		VkDescriptorSetLayout descriptorSetLayout;	// Image display shader binding layout
		VkDescriptorSet descriptorSetPreCompute;	// Image display shader bindings before compute shader image manipulation
		VkDescriptorSet descriptorSetPostCompute;	// Image display shader bindings after compute shader image manipulation
		VkDescriptorSet descriptorSetHypercube;
		VkPipeline pipeline;						// Image display pipeline
		VkPipeline pipelineHypercube;
		VkPipelineLayout pipelineLayout;			// Layout of the graphics pipeline
		VkSemaphore semaphore;                      // Execution dependency between compute & graphic submission
	} graphics;

	// Resources for the compute part of the plugin
	struct Compute {
		VkQueue queue;								// Separate queue for compute commands (queue family may differ from the one used for graphics)
		VkCommandPool commandPool;					// Use a separate command pool (queue family may differ from the one used for graphics)
		VkCommandBuffer commandBuffer;				// Command buffer storing the dispatch commands and barriers
		VkSemaphore semaphore;                      // Execution dependency between compute & graphic submission
		std::vector<VkDescriptorSetLayout> descriptorSetLayout;	// Compute shader binding layout
		std::vector<VkDescriptorSet> descriptorSet;				// Compute shader bindings
		std::vector<VkPipelineLayout> pipelineLayout;			// Layout of the compute pipeline
		std::vector<VkPipeline> pipelines;			// Compute pipelines for image filters
		int32_t pipelineIndex = 0;					// Current image filtering compute pipeline index
	} compute;

	vks::Buffer vertexBuffer;
	vks::Buffer indexBuffer;
	uint32_t indexCount;

	vks::Buffer uniformBufferVS;

	struct {
		glm::mat4 projection;
		glm::mat4 modelView;
	} uboVS;

	int vertexBufferSize;

	std::vector<std::string> shaderNames;

	HvsPipeline() : VulkanExampleBase(ENABLE_VALIDATION)
	{
		title = "HVS plugin example";
		camera.type = Camera::CameraType::lookat;
		camera.setPosition(glm::vec3(0.0f, 0.0f, -2.0f));
		camera.setRotation(glm::vec3(0.0f));
		camera.setPerspective(60.0f, (float)width * 0.5f / (float)height, 0.01f, 256.0f);

		enabledInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
		enabledInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		//enabledDeviceExtensions.push_back();

	}


	~HvsPipeline()
	{
		// Graphics
		vkDestroyPipeline(device, graphics.pipeline, nullptr);
		vkDestroyPipelineLayout(device, graphics.pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, graphics.descriptorSetLayout, nullptr);
		vkDestroySemaphore(device, graphics.semaphore, nullptr);


		// Compute
		for (auto& pipeline : compute.pipelines)
		{
			vkDestroyPipeline(device, pipeline, nullptr);
		}
		for (auto& pipelineLayout : compute.pipelineLayout)
		{
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		}
		//vkDestroyPipelineLayout(device, compute.pipelineLayout, nullptr);

		for (auto& dsl : compute.descriptorSetLayout)
		{
			vkDestroyDescriptorSetLayout(device, dsl, nullptr);
		}
		//vkDestroyDescriptorSetLayout(device, compute.descriptorSetLayout, nullptr);

		vkDestroySemaphore(device, compute.semaphore, nullptr);
		vkDestroyCommandPool(device, compute.commandPool, nullptr);

		vertexBuffer.destroy();
		indexBuffer.destroy();
		uniformBufferVS.destroy();

		textureRaw.destroy();
		textureCalibrated.destroy();
	}

	// Prepare a texture target that is used to store compute shader calculations
	void prepareTextureTarget(vks::Texture* tex, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, uint32_t levels, VkFormat format);
	void loadImageBuffer(void* buffer, VkDeviceSize size, uint32_t width, uint32_t height, VkFormat format);

	void updateImageBuffer(void* buffer, VkDeviceSize size, VkFormat format);

	void buildCommandBuffers();

	// gets run everytime the type of shader chosen is changed
	void buildComputeCommandBuffer();

	// Setup vertices for a single uv-mapped quad
	void generateQuad();

	void setupVertexDescriptions();

	void setupDescriptorPool();

	void setupDescriptorSetLayout();

	void setupDescriptorSet();

	void preparePipelines();

	void prepareGraphics();

	void prepareCompute();

	// Prepare and initialize uniform buffer containing shader uniforms
	void prepareUniformBuffers();

	void updateUniformBuffers();

	void draw();

	void openCamera();

	void* getCameraFrame();

	void setCamera(CameraGrabber& cameraGrabber);

	void prepare();

	virtual void render();

	virtual void OnUpdateUIOverlay(vks::UIOverlay* overlay);

};
