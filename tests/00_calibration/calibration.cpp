#include "calibration.h"

void HvsTest::loadPNG16(std::string fileName)
{

	std::cout << "Loading file : " << fileName << std::endl;
	int channels;
	uint16_t* img = stbi_load_16(fileName.c_str(), &m_image_width, &m_image_height, &channels, 1);
	std::cout << "w : " << m_image_width << " h : " << m_image_height << std::endl;

	rawImage.fromBuffer((void *)img, width * height * sizeof(uint16_t), VK_FORMAT_R16_UINT, m_image_width, m_image_height, vulkanDevice, queue, VK_FILTER_NEAREST, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);



}

void HvsTest::loadFileU16(std::string fileName)
{
	if (m_fin.is_open()) {
		m_fin.close();
	}

	m_fin.open(fileName, std::ifstream::in | std::ifstream::binary);
	if (m_fin.is_open())
	{
		printf("fin is open\n");

	}
	std::vector<uint16_t> buffer(m_image_width * m_image_height, 0);

	m_fin.read((char*)buffer.data(), buffer.size());
}


void HvsTest::loadFileF32(std::string fileName)
{
	std::cout << "Loading file f32: " << fileName << std::endl;

	int channels;
	float* img = stbi_loadf(fileName.c_str(), &m_image_width, &m_image_height, &channels, 1);

	std::cout << "w : " << m_image_width << " h : " << m_image_height << std::endl;
	rawImage.fromBuffer((void*)img, width * height * sizeof(float), VK_FORMAT_R32_SFLOAT, m_image_width, m_image_height, vulkanDevice, queue, VK_FILTER_NEAREST, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
}

void HvsTest::loadImageBuffer(void* buffer, int32_t m_image_width, int32_t m_image_height, VkFormat format)
{
	rawImage.fromBuffer(buffer, m_image_width * m_image_height * sizeof(float), format, m_image_width, m_image_height, vulkanDevice, queue, VK_FILTER_NEAREST, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
}

void HvsTest::downloadImageBuffer(void* buffer, VkFormat format)
{
	calibratedImage.download(buffer, m_image_width * m_image_height * sizeof(float), format, vulkanDevice, queue);
}

void HvsTest::setWhiteValue(uint16_t val) 
{

}

void HvsTest::setDarkValue(uint16_t val)
{

}

void HvsTest::setMaskValue(uint8_t val)
{

}

// not sure if this has to be flattened on the python side...
void HvsTest::setWhiteImage(py::array_t<uint16_t, py::array::c_style | py::array::forcecast> array)
{
	numpyWhite.resize(m_image_width * m_image_height);

	auto arr_obj_prop = array.request();
	uint16_t* vals = (uint16_t*)arr_obj_prop.ptr;

	for (int i = 0; i < m_image_width * m_image_height; i++) {
		numpyWhite[i] = *(vals + i);
	}
}

void HvsTest::setDarkImage(py::array_t<uint16_t, py::array::c_style | py::array::forcecast> array)
{
	numpyDark.resize(m_image_width * m_image_height);

	auto arr_obj_prop = array.request();
	uint16_t* vals = (uint16_t*)arr_obj_prop.ptr;

	for (int i = 0; i < m_image_width * m_image_height; i++) {
		numpyDark[i] = *(vals + i);
	}
}

void HvsTest::setMaskImage(py::array_t<uint8_t, py::array::c_style | py::array::forcecast> array)
{
	numpyMask.resize(m_image_width * m_image_height);

	auto arr_obj_prop = array.request();
	uint8_t* vals = (uint8_t*)arr_obj_prop.ptr;

	for (int i = 0; i < m_image_width * m_image_height; i++) {
		numpyMask[i] = *(vals + i);
	}
}

void HvsTest::setRawImage(py::array_t<uint16_t, py::array::c_style | py::array::forcecast> array)
{
	numpyRaw.resize(m_image_width * m_image_height);

	auto arr_obj_prop = array.request();
	uint16_t* vals = (uint16_t*)arr_obj_prop.ptr;

	for (int i = 0; i < m_image_width * m_image_height; i++) {
		numpyRaw[i] = *(vals + i);
	}
}

py::array HvsTest::getCalibratedImage()
{
	m_calibratedImageBuffer.resize(10, 69.8008f);
	//py::array ret = py::cast(m_calibratedImageBuffer);
	return py::array(m_calibratedImageBuffer.size(), m_calibratedImageBuffer.data());
}

// gets run everytime the type of shader chosen is changed
void HvsTest::buildComputeCommandBuffer()
{
	std::cout << "0" << std::endl;
	std::cout << &compute.queue << std::endl;
	// Flush the queue if we're rebuilding the command buffer after a pipeline change to ensure it's not currently in use
	vkQueueWaitIdle(compute.queue);
	std::cout << "1" << std::endl;

	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
	std::cout << "2" << std::endl;

	VK_CHECK_RESULT(vkBeginCommandBuffer(compute.commandBuffer, &cmdBufInfo));

	std::cout << "3" << std::endl;

	vkCmdBindPipeline(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipeline);
	std::cout << "4" << std::endl;

	vkCmdBindDescriptorSets(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout, 0, 1, &compute.descriptorSet, 0, 0);
	std::cout << "5" << std::endl;

	vkCmdDispatch(compute.commandBuffer, m_image_width / 32, m_image_height / 32, 1);
	std::cout << "6" << std::endl;

	vkEndCommandBuffer(compute.commandBuffer);
	std::cout << "7" << std::endl;

}

void HvsTest::setupDescriptorPool()
{
	std::cout << "1" << std::endl;

	std::vector<VkDescriptorPoolSize> poolSizes = {
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),

		// Compute pipelines uses a storage image for image reads and writes
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 5),
	};
	std::cout << "111" << std::endl;

	VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 6);
	std::cout << "123" << std::endl;

	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
	std::cout << "1111" << std::endl;

}

void HvsTest::prepareTextures()
{
	std::vector<float> blankImage(m_image_width * m_image_height, -1.0f);

	rawImage.fromBuffer(numpyRaw.data(), m_image_width * m_image_height * sizeof(uint16_t), VK_FORMAT_R16_UINT, m_image_width, m_image_height, vulkanDevice, queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);

	whiteImage.fromBuffer(numpyWhite.data(), m_image_width * m_image_height * sizeof(uint16_t), VK_FORMAT_R16_UINT, m_image_width, m_image_height, vulkanDevice, queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
	darkImage.fromBuffer(numpyDark.data(), m_image_width * m_image_height * sizeof(uint16_t), VK_FORMAT_R16_UINT, m_image_width, m_image_height, vulkanDevice, queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
	maskImage.fromBuffer(numpyMask.data(), m_image_width * m_image_height * sizeof(uint8_t), VK_FORMAT_R8_UINT, m_image_width, m_image_height, vulkanDevice, queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);

	calibratedImage.fromBuffer(blankImage.data(), m_image_width * m_image_height * sizeof(uint16_t), VK_FORMAT_R16_UINT, m_image_width, m_image_height, vulkanDevice, queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);

}

void HvsTest::prepareCompute()
{

	// Get a compute queue from the device
	vkGetDeviceQueue(device, vulkanDevice->queueFamilyIndices.compute, 0, &compute.queue);

	std::cout << "1" << std::endl;
	// One pipeline for each effect
	std::string shaderName = "correction";

	// Create compute pipeline

	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			// images
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1),
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 2),
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 3),
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 4),
			// Uniform buffer
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 5),
		};
		std::cout << "1" << std::endl;

		VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &compute.descriptorSetLayout));
		std::cout << "1" << std::endl;

		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
			vks::initializers::pipelineLayoutCreateInfo(&compute.descriptorSetLayout, 1);
		std::cout << "1" << std::endl;

		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &compute.pipelineLayout)); 
		std::cout << "1" << std::endl;

		VkDescriptorSetAllocateInfo allocInfo =
			vks::initializers::descriptorSetAllocateInfo(descriptorPool, &compute.descriptorSetLayout, 1); 
		std::cout << "1" << std::endl;

		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &compute.descriptorSet)); 
		std::cout << "1" << std::endl;

		std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
			// images
			vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &rawImage.descriptor),
			vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &whiteImage.descriptor),
			vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, &darkImage.descriptor),
			vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3, &calibratedImage.descriptor),
			vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 4, &maskImage.descriptor),
			// Uniform buffer							 
			vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	5, &calibrationUBO.descriptor),

		};
		vkUpdateDescriptorSets(device, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);
		std::cout << "1" << std::endl;

		// Create compute shader pipelines
		VkComputePipelineCreateInfo computePipelineCreateInfo =
			vks::initializers::computePipelineCreateInfo(compute.pipelineLayout, 0); 
		std::cout << "1" << std::endl;

		std::string fileName = getShadersPath() + "pipeline/" + shaderName + ".comp.spv";
		computePipelineCreateInfo.stage = loadShader(fileName, VK_SHADER_STAGE_COMPUTE_BIT);
		std::cout << "1" << std::endl;

		VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &compute.pipeline));
		std::cout << "1" << std::endl;

	}
	std::cout << "1" << std::endl;


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
