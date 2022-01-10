#include "demosaic.h"

void HvsTest::loadFileU16(std::string fileName)
{

	std::cout << "Loading file : " << fileName << std::endl;
	int channels;
	uint16_t* img = stbi_load_16(fileName.c_str(), &m_image_width, &m_image_height, &channels, 1);
	std::cout << "w : " << m_image_width << " h : " << m_image_height << std::endl;

	calibrated.fromBuffer((void *)img, width * height * sizeof(uint16_t), VK_FORMAT_R16_UINT, m_image_width, m_image_height, vulkanDevice, queue, VK_FILTER_NEAREST, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);



}

void HvsTest::loadFileF32(std::string fileName)
{
	std::cout << "Loading file f32: " << fileName << std::endl;

	int channels;
	float* img = stbi_loadf(fileName.c_str(), &m_image_width, &m_image_height, &channels, 1);

	std::cout << "w : " << m_image_width << " h : " << m_image_height << std::endl;
	calibrated.fromBuffer((void*)img, width * height * sizeof(float), VK_FORMAT_R32_SFLOAT, m_image_width, m_image_height, vulkanDevice, queue, VK_FILTER_NEAREST, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
}

void HvsTest::loadImageBuffer(void* buffer, int32_t m_image_width, int32_t m_image_height, VkFormat format)
{
	calibrated.fromBuffer(buffer, m_image_width * m_image_height * sizeof(float), format, m_image_width, m_image_height, vulkanDevice, queue, VK_FILTER_NEAREST, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
}

void HvsTest::downloadImageBuffer(void* buffer, VkFormat format)
{
	calibrated.download(buffer, m_image_width * m_image_height * sizeof(float), format, vulkanDevice, queue);
}



// gets run everytime the type of shader chosen is changed
void HvsTest::buildComputeCommandBuffer()
{
	// Flush the queue if we're rebuilding the command buffer after a pipeline change to ensure it's not currently in use
	vkQueueWaitIdle(compute.queue);

	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VK_CHECK_RESULT(vkBeginCommandBuffer(compute.commandBuffer, &cmdBufInfo));


	vkCmdBindPipeline(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipeline);
	vkCmdBindDescriptorSets(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout, 0, 1, &compute.descriptorSet, 0, 0);

	vkCmdDispatch(compute.commandBuffer, m_image_width / 32, m_image_height / 32, 1);

	vkEndCommandBuffer(compute.commandBuffer);
}

void HvsTest::setupDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes = {
		// Compute pipelines uses a storage image for image reads and writes
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2),
	};
	VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 2);
	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
}

void HvsTest::prepareCompute()
{
	// Get a compute queue from the device
	vkGetDeviceQueue(device, vulkanDevice->queueFamilyIndices.compute, 0, &compute.queue);

	// One pipeline for each effect
	std::string shaderName = "demosaic";

	// Create compute pipeline

	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			// images
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1),
		};

		VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &compute.descriptorSetLayout));

		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
			vks::initializers::pipelineLayoutCreateInfo(&compute.descriptorSetLayout, 1);

		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &compute.pipelineLayout)); 

		VkDescriptorSetAllocateInfo allocInfo =
			vks::initializers::descriptorSetAllocateInfo(descriptorPool, &compute.descriptorSetLayout, 1); 

		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &compute.descriptorSet)); 

		std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
			// images
			vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &calibrated.descriptor),
			vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &hypercubeCalibrated.descriptor),
		};
		vkUpdateDescriptorSets(device, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);

		// Create compute shader pipelines
		VkComputePipelineCreateInfo computePipelineCreateInfo =
			vks::initializers::computePipelineCreateInfo(compute.pipelineLayout, 0); 

		std::string fileName = getShadersPath() + "pipeline/" + shaderName + ".comp.spv";
		computePipelineCreateInfo.stage = loadShader(fileName, VK_SHADER_STAGE_COMPUTE_BIT);

		VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &compute.pipeline));

	}


	// Separate command pool as queue family for compute may be different than graphics
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = vulkanDevice->queueFamilyIndices.compute;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &compute.commandPool));

	// Create a command buffer for compute operations
	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vks::initializers::commandBufferAllocateInfo(
			compute.commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1);

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &compute.commandBuffer));

	// Fence for compute CB sync
	VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &compute.fence));

	// Build a single command buffer containing the compute dispatch commands
	buildComputeCommandBuffer();
}

void HvsTest::run()
{

}
