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

#include "pipeline.h"
#include "vulkanexamplebase.h"

#include "opencv2/opencv.hpp"

#include "ximea_grabber.h"

#include "tables.h"



	// Prepare a texture target that is used to store compute shader calculations
	void HvsPipeline::prepareTextureTarget(vks::Texture* tex, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, uint32_t levels, VkFormat format)
	{
		VkFormatProperties formatProperties;

		// Get device properties for the requested texture format
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
		// Check if requested image format supports image storage operations
		assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

		// Prepare blit target texture
		tex->width = width;
		tex->height = height;

		VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
		imageCreateInfo.imageType = depth == 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D;
		imageCreateInfo.format = format;
		imageCreateInfo.extent = { width, height, depth };
		imageCreateInfo.mipLevels = levels;
		imageCreateInfo.arrayLayers = layers;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		// Image will be sampled in the fragment shader and used as storage target in the compute shader
		imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		imageCreateInfo.flags = 0;
		// If compute and graphics queue family indices differ, we create an image that can be shared between them
		// This can result in worse performance than exclusive sharing mode, but save some synchronization to keep the sample simple
		std::vector<uint32_t> queueFamilyIndices;
		if (vulkanDevice->queueFamilyIndices.graphics != vulkanDevice->queueFamilyIndices.compute) {
			queueFamilyIndices = {
				vulkanDevice->queueFamilyIndices.graphics,
				vulkanDevice->queueFamilyIndices.compute
			};
			imageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
			imageCreateInfo.queueFamilyIndexCount = 2;
			imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}

		VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
		VkMemoryRequirements memReqs;

		VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &tex->image));

		vkGetImageMemoryRequirements(device, tex->image, &memReqs);
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &tex->deviceMemory));
		VK_CHECK_RESULT(vkBindImageMemory(device, tex->image, tex->deviceMemory, 0));

		VkCommandBuffer layoutCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		tex->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		vks::tools::setImageLayout(
			layoutCmd, tex->image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			tex->imageLayout);

		if (layers > 1) {
			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = 1;
			subresourceRange.layerCount = layers;

			vks::tools::setImageLayout(
				layoutCmd,
				tex->image,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_GENERAL,
				subresourceRange);

		}

		vulkanDevice->flushCommandBuffer(layoutCmd, queue, true);


		// Create sampler
		VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
		sampler.magFilter = VK_FILTER_LINEAR;
		sampler.minFilter = VK_FILTER_LINEAR;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		sampler.addressModeV = sampler.addressModeU;
		sampler.addressModeW = sampler.addressModeU;
		sampler.mipLodBias = 0.0f;
		sampler.maxAnisotropy = 8;
		sampler.compareOp = VK_COMPARE_OP_NEVER;
		sampler.minLod = 0.0f;
		sampler.maxLod = tex->mipLevels;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &tex->sampler));

		// Create image view
		VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
		view.image = VK_NULL_HANDLE;
		view.viewType = ((depth > 1) || (layers > 1)) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
		view.format = format;
		view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		view.subresourceRange.layerCount = layers;
		view.subresourceRange.levelCount = 1; // for mips
		view.image = tex->image;
		VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &tex->view));

		// Initialize a descriptor for later use
		tex->descriptor.imageLayout = tex->imageLayout;
		tex->descriptor.imageView = tex->view;
		tex->descriptor.sampler = tex->sampler;
		tex->device = vulkanDevice;


	}

	void HvsPipeline::loadImageBuffer(void* buffer, VkDeviceSize size, uint32_t width, uint32_t height, VkFormat format)
	{
		textureRaw.fromBuffer(buffer, size, format, width, height, vulkanDevice, queue, VK_FILTER_NEAREST, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
	}

	void HvsPipeline::updateImageBuffer(void* buffer, VkDeviceSize size, VkFormat format)
	{
		textureRaw.update(buffer, size, format, vulkanDevice, queue);
	}

	void HvsPipeline::buildCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VkClearValue clearValues[2];
		clearValues[0].color = defaultClearColor;
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;

		for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
		{
			// Set target frame buffer
			renderPassBeginInfo.framebuffer = frameBuffers[i];

			VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

			// Image memory barrier to make sure that compute shader writes are finished before sampling from the texture
			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			// We won't be changing the layout of the image
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageMemoryBarrier.image = sRGB.image;
			imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vkCmdPipelineBarrier(
				drawCmdBuffers[i],
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_FLAGS_NONE,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
			vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			std::vector<VkViewport> viewports;
			VkViewport viewport = vks::initializers::viewport((float)width * 0.5f, (float)height, 0.0f, 1.0f);
			viewports.push_back(viewport);
			vkCmdSetViewport(drawCmdBuffers[i], 0, static_cast<uint32_t>(viewports.size()), viewports.data());

			VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
			vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &vertexBuffer.buffer, offsets);
			vkCmdBindIndexBuffer(drawCmdBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

			// Left (pre compute)
			//int32_t show_type = 0;
			//memcpy(renderUBO.mapped, &show_type, sizeof(int32_t));

			vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipelineLayout, 0, 1, &graphics.descriptorSetPreCompute, 0, NULL);
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipeline);

			vkCmdDrawIndexed(drawCmdBuffers[i], indexCount, 1, 0, 0, 0);



			viewport.x = (float)width / 2.0f;
			vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
			vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipelineLayout, 0, 1, &graphics.descriptorSetHypercube, 0, NULL);
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipelineHypercube);

			vkCmdDrawIndexed(drawCmdBuffers[i], indexCount, 1, 0, 0, 0);

			//// Right (post compute)
			//show_type = 1;
			//memcpy(renderUBO.mapped, &show_type, sizeof(int32_t));
			//vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipelineLayout, 0, 1, &graphics.descriptorSetPostCompute, 0, NULL);
			//vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipeline);


			//vkCmdDrawIndexed(drawCmdBuffers[i], indexCount, 1, 0, 0, 0);

			drawUI(drawCmdBuffers[i]);

			vkCmdEndRenderPass(drawCmdBuffers[i]);

			VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
		}

	}

	// gets run everytime the type of shader chosen is changed
	void HvsPipeline::buildComputeCommandBuffer()
	{
		// Flush the queue if we're rebuilding the command buffer after a pipeline change to ensure it's not currently in use
		vkQueueWaitIdle(compute.queue);

		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VK_CHECK_RESULT(vkBeginCommandBuffer(compute.commandBuffer, &cmdBufInfo));

		vkCmdBindPipeline(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelines[SHADER_SOURCE::CALIBRATE]);
		vkCmdBindDescriptorSets(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout[SHADER_SOURCE::CALIBRATE], 0, 1, &compute.descriptorSet[SHADER_SOURCE::CALIBRATE], 0, 0);

		vkCmdDispatch(compute.commandBuffer, m_image_width / 32, m_image_height / 32, 1);

		vkCmdBindPipeline(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelines[SHADER_SOURCE::DEMOSAIC]);
		vkCmdBindDescriptorSets(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout[SHADER_SOURCE::DEMOSAIC], 0, 1, &compute.descriptorSet[SHADER_SOURCE::DEMOSAIC], 0, 0);

		vkCmdDispatch(compute.commandBuffer, m_image_width / 32, m_image_height / 32, 1);

		vkCmdBindPipeline(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelines[SHADER_SOURCE::CORRECTION]);
		vkCmdBindDescriptorSets(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout[SHADER_SOURCE::CORRECTION], 0, 1, &compute.descriptorSet[SHADER_SOURCE::CORRECTION], 0, 0);

		vkCmdDispatch(compute.commandBuffer, m_image_width / 16, m_image_height / 16, 16);

		vkCmdBindPipeline(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelines[SHADER_SOURCE::HC2SRGB]);
		vkCmdBindDescriptorSets(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout[SHADER_SOURCE::HC2SRGB], 0, 1, &compute.descriptorSet[SHADER_SOURCE::HC2SRGB], 0, 0);

		vkCmdDispatch(compute.commandBuffer, m_image_width / 32, m_image_height / 32, 1);

		vkEndCommandBuffer(compute.commandBuffer);
	}

	// Setup vertices for a single uv-mapped quad
	void HvsPipeline::generateQuad()
	{
		// Setup vertices for a single uv-mapped quad made from two triangles
		std::vector<Vertex> vertices =
		{
			{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f } },
			{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f } },
			{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
			{ {  1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } }
		};

		// Setup indices
		std::vector<uint32_t> indices = { 0,1,2, 2,3,0 };
		indexCount = static_cast<uint32_t>(indices.size());

		// Create buffers
		// For the sake of simplicity we won't stage the vertex data to the gpu memory
		// Vertex buffer
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&vertexBuffer,
			vertices.size() * sizeof(Vertex),
			vertices.data()));
		// Index buffer
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&indexBuffer,
			indices.size() * sizeof(uint32_t),
			indices.data()));
	}

	void HvsPipeline::setupVertexDescriptions()
	{
		// Binding description
		vertices.bindingDescriptions = {
			vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
		};

		// Attribute descriptions
		// Describes memory layout and shader positions
		vertices.attributeDescriptions = {
			// Location 0: Position
			vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)),
			// Location 1: Texture coordinates
			vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)),
		};

		// Assign to vertex buffer
		vertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
		vertices.inputState.vertexBindingDescriptionCount = vertices.bindingDescriptions.size();
		vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
		vertices.inputState.vertexAttributeDescriptionCount = vertices.attributeDescriptions.size();
		vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
	}

	void HvsPipeline::setupDescriptorPool()
	{
		std::vector<VkDescriptorPoolSize> poolSizes = {
			// Graphics pipelines uniform buffers
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8),
			// Graphics pipelines image samplers for displaying compute output image
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 16),
			// Compute pipelines uses a storage image for image reads and writes
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 16),
		};
		VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 20);
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
	}

	void HvsPipeline::setupDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			// Binding 0: Vertex shader uniform buffer
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),

			// Binding 1: Fragment shader input image
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
		};

		VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &graphics.descriptorSetLayout));

		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&graphics.descriptorSetLayout, 1);
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &graphics.pipelineLayout));
	}

	void HvsPipeline::setupDescriptorSet()
	{
		VkDescriptorSetAllocateInfo allocInfo =
			vks::initializers::descriptorSetAllocateInfo(descriptorPool, &graphics.descriptorSetLayout, 1);

		// Input image (before compute post processing)
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &graphics.descriptorSetPreCompute));
		std::vector<VkWriteDescriptorSet> baseImageWriteDescriptorSets = {
			vks::initializers::writeDescriptorSet(graphics.descriptorSetPreCompute, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBufferVS.descriptor),
			vks::initializers::writeDescriptorSet(graphics.descriptorSetPreCompute, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textureRaw.descriptor),

		};
		vkUpdateDescriptorSets(device, baseImageWriteDescriptorSets.size(), baseImageWriteDescriptorSets.data(), 0, nullptr);
		//
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &graphics.descriptorSetHypercube));
		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			vks::initializers::writeDescriptorSet(graphics.descriptorSetHypercube, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBufferVS.descriptor),
			vks::initializers::writeDescriptorSet(graphics.descriptorSetHypercube, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &sRGB.descriptor),

		};
		vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);

		//// Final image (after compute shader processing)
		//VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &graphics.descriptorSetPostCompute));
		//std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
		//	vks::initializers::writeDescriptorSet(graphics.descriptorSetPostCompute, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBufferVS.descriptor),
		//	vks::initializers::writeDescriptorSet(graphics.descriptorSetPostCompute, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &renderUBO.descriptor),
		//	vks::initializers::writeDescriptorSet(graphics.descriptorSetPostCompute, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &textureCalibrated.descriptor),
  //          vks::initializers::writeDescriptorSet(graphics.descriptorSetPostCompute, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &hypercubeCalibrated.descriptor),
		//};
		//vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);

	}

	void HvsPipeline::preparePipelines()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
			vks::initializers::pipelineInputAssemblyStateCreateInfo(
				VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				0,
				VK_FALSE);

		VkPipelineRasterizationStateCreateInfo rasterizationState =
			vks::initializers::pipelineRasterizationStateCreateInfo(
				VK_POLYGON_MODE_FILL,
				VK_CULL_MODE_NONE,
				VK_FRONT_FACE_COUNTER_CLOCKWISE,
				0);

		VkPipelineColorBlendAttachmentState blendAttachmentState =
			vks::initializers::pipelineColorBlendAttachmentState(
				0xf,
				VK_FALSE);

		VkPipelineColorBlendStateCreateInfo colorBlendState =
			vks::initializers::pipelineColorBlendStateCreateInfo(
				1,
				&blendAttachmentState);

		VkPipelineDepthStencilStateCreateInfo depthStencilState =
			vks::initializers::pipelineDepthStencilStateCreateInfo(
				VK_TRUE,
				VK_TRUE,
				VK_COMPARE_OP_LESS_OR_EQUAL);

		VkPipelineViewportStateCreateInfo viewportState =
			vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

		VkPipelineMultisampleStateCreateInfo multisampleState =
			vks::initializers::pipelineMultisampleStateCreateInfo(
				VK_SAMPLE_COUNT_1_BIT,
				0);

		std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState =
			vks::initializers::pipelineDynamicStateCreateInfo(
				dynamicStateEnables.data(),
				dynamicStateEnables.size(),
				0);

		// Rendering pipeline
		// Load shaders
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

		shaderStages[0] = loadShader(getShadersPath() + "pipeline/texture.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = loadShader(getShadersPath() + "pipeline/texture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		VkGraphicsPipelineCreateInfo pipelineCreateInfo =
			vks::initializers::pipelineCreateInfo(
				graphics.pipelineLayout,
				renderPass,
				0);

		pipelineCreateInfo.pVertexInputState = &vertices.inputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = shaderStages.size();
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.renderPass = renderPass;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &graphics.pipeline));



		shaderStages[1] = loadShader(getShadersPath() + "pipeline/texture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		//shaderStages[1] = loadShader(getShadersPath() + "pipeline/textureHypercube.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		pipelineCreateInfo.stageCount = shaderStages.size();
		pipelineCreateInfo.pStages = shaderStages.data();
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &graphics.pipelineHypercube));

	}

	void HvsPipeline::prepareGraphics()
	{
		// Semaphore for compute & graphics sync
		VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &graphics.semaphore));
	}

	void HvsPipeline::prepareCompute()
	{

		// Get a compute queue from the device
		vkGetDeviceQueue(device, vulkanDevice->queueFamilyIndices.compute, 0, &compute.queue);

		// One pipeline for each effect
		shaderNames = { "calibration" , "demosaic",  "correction", "hc2srgb"//, "remosaic"
		};

		// Create compute pipeline
		// Compute pipelines are created separate from graphics pipelines even if they use the same queue
		compute.descriptorSetLayout.resize(shaderNames.size());
		compute.pipelineLayout.resize(shaderNames.size());
		compute.descriptorSet.resize(shaderNames.size());
		compute.pipelines.resize(shaderNames.size());

		// CALIBRATE
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

			VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &compute.descriptorSetLayout[SHADER_SOURCE::CALIBRATE]));

			VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
				vks::initializers::pipelineLayoutCreateInfo(&compute.descriptorSetLayout[SHADER_SOURCE::CALIBRATE], 1);// 0 for calibrate

			VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &compute.pipelineLayout[SHADER_SOURCE::CALIBRATE])); // 0 for calibrate

			VkDescriptorSetAllocateInfo allocInfo =
				vks::initializers::descriptorSetAllocateInfo(descriptorPool, &compute.descriptorSetLayout[SHADER_SOURCE::CALIBRATE], 1); // 0 for calibrate

			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &compute.descriptorSet[SHADER_SOURCE::CALIBRATE])); // 0 for calibrate

			std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
				// images
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::CALIBRATE], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &textureRaw.descriptor),
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::CALIBRATE], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &whiteReference.descriptor),
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::CALIBRATE], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, &darkReference.descriptor),
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::CALIBRATE], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3, &calibrated.descriptor),
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::CALIBRATE], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 4, &fovMask.descriptor),
				// Uniform buffer
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::CALIBRATE], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	5, &calibrateUBO.descriptor),

			};
			vkUpdateDescriptorSets(device, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);

			// Create compute shader pipelines
			VkComputePipelineCreateInfo computePipelineCreateInfo =
				vks::initializers::computePipelineCreateInfo(compute.pipelineLayout[SHADER_SOURCE::CALIBRATE], 0); // 0 for calibrate

			std::string fileName = getShadersPath() + "pipeline/calibration.comp.spv";
			computePipelineCreateInfo.stage = loadShader(fileName, VK_SHADER_STAGE_COMPUTE_BIT);

			VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &compute.pipelines[SHADER_SOURCE::CALIBRATE]));
			
		}

		// DEMOSAIC
		{
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				// images
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0),
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1),
			};

			VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &compute.descriptorSetLayout[SHADER_SOURCE::DEMOSAIC]));

			VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
				vks::initializers::pipelineLayoutCreateInfo(&compute.descriptorSetLayout[SHADER_SOURCE::DEMOSAIC], 1);// 0 for calibrate

			VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &compute.pipelineLayout[SHADER_SOURCE::DEMOSAIC])); // 0 for calibrate

			VkDescriptorSetAllocateInfo allocInfo =
				vks::initializers::descriptorSetAllocateInfo(descriptorPool, &compute.descriptorSetLayout[SHADER_SOURCE::DEMOSAIC], 1); // 0 for calibrate

			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &compute.descriptorSet[SHADER_SOURCE::DEMOSAIC])); // 0 for calibrate

			std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
				// images
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::DEMOSAIC], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &calibrated.descriptor),
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::DEMOSAIC], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &hypercubeCalibrated.descriptor),
			};
			vkUpdateDescriptorSets(device, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);

			// Create compute shader pipelines
			VkComputePipelineCreateInfo computePipelineCreateInfo =
				vks::initializers::computePipelineCreateInfo(compute.pipelineLayout[SHADER_SOURCE::DEMOSAIC], 0); // 0 for calibrate

			std::string fileName = getShadersPath() + "pipeline/demosaic.comp.spv";
			computePipelineCreateInfo.stage = loadShader(fileName, VK_SHADER_STAGE_COMPUTE_BIT);

			VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &compute.pipelines[SHADER_SOURCE::DEMOSAIC]));

		}
		
		// CORRECTION
		{
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				// images
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0),
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1),
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 2),
				// Uniform buffer
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 3),
			};

			VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &compute.descriptorSetLayout[SHADER_SOURCE::CORRECTION]));

			VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
				vks::initializers::pipelineLayoutCreateInfo(&compute.descriptorSetLayout[SHADER_SOURCE::CORRECTION], 1);// 0 for calibrate

			VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &compute.pipelineLayout[SHADER_SOURCE::CORRECTION])); // 0 for calibrate

			VkDescriptorSetAllocateInfo allocInfo =
				vks::initializers::descriptorSetAllocateInfo(descriptorPool, &compute.descriptorSetLayout[SHADER_SOURCE::CORRECTION], 1); // 0 for calibrate

			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &compute.descriptorSet[SHADER_SOURCE::CORRECTION])); // 0 for calibrate

			std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
				// images
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::CORRECTION], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &hypercubeCalibrated.descriptor),
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::CORRECTION], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &hypercubeCalibratedColorCorrected.descriptor),
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::CORRECTION], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, &correctionMatrix.descriptor),
				// Uniform buffer
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::CORRECTION], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &correctionUBO.descriptor),

			};
			vkUpdateDescriptorSets(device, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);

			// Create compute shader pipelines
			VkComputePipelineCreateInfo computePipelineCreateInfo =
				vks::initializers::computePipelineCreateInfo(compute.pipelineLayout[SHADER_SOURCE::CORRECTION], 0); // 0 for calibrate

			std::string fileName = getShadersPath() + "pipeline/correction.comp.spv";
			computePipelineCreateInfo.stage = loadShader(fileName, VK_SHADER_STAGE_COMPUTE_BIT);

			VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &compute.pipelines[SHADER_SOURCE::CORRECTION]));

		}


		// HC2sRGB
		{
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				// images
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0),
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1),
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 2),
				// Uniform buffer
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 3),
			};

			VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &compute.descriptorSetLayout[SHADER_SOURCE::HC2SRGB]));

			VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
				vks::initializers::pipelineLayoutCreateInfo(&compute.descriptorSetLayout[SHADER_SOURCE::HC2SRGB], 1);// 0 for calibrate

			VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &compute.pipelineLayout[SHADER_SOURCE::HC2SRGB])); // 0 for calibrate

			VkDescriptorSetAllocateInfo allocInfo =
				vks::initializers::descriptorSetAllocateInfo(descriptorPool, &compute.descriptorSetLayout[SHADER_SOURCE::HC2SRGB], 1); // 0 for calibrate

			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &compute.descriptorSet[SHADER_SOURCE::HC2SRGB])); // 0 for calibrate

			std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
				// images
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::HC2SRGB], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &hypercubeCalibratedColorCorrected.descriptor),
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::HC2SRGB], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &sRGB.descriptor),
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::HC2SRGB], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, &colorMatrix.descriptor),
				// Uniform buffer
				vks::initializers::writeDescriptorSet(compute.descriptorSet[SHADER_SOURCE::HC2SRGB], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &hc2srgbUBO.descriptor),

			};
			vkUpdateDescriptorSets(device, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);

			// Create compute shader pipelines
			VkComputePipelineCreateInfo computePipelineCreateInfo =
				vks::initializers::computePipelineCreateInfo(compute.pipelineLayout[SHADER_SOURCE::HC2SRGB], 0); // 0 for calibrate

			std::string fileName = getShadersPath() + "pipeline/hc2srgb.comp.spv";
			computePipelineCreateInfo.stage = loadShader(fileName, VK_SHADER_STAGE_COMPUTE_BIT);

			VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &compute.pipelines[SHADER_SOURCE::HC2SRGB]));

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

		// Semaphore for compute & graphics sync
		VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &compute.semaphore));

		// Signal the semaphore
		VkSubmitInfo submitInfo = vks::initializers::submitInfo();
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &compute.semaphore;
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		VK_CHECK_RESULT(vkQueueWaitIdle(queue));

		// Build a single command buffer containing the compute dispatch commands
		buildComputeCommandBuffer();
	}

	// Prepare and initialize uniform buffer containing shader uniforms
	void HvsPipeline::prepareUniformBuffers()
	{
		// uniform buffer objects for compute shaders
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&calibrateUBO,
			sizeof(float));

		// Map for host access
		VK_CHECK_RESULT(calibrateUBO.map());

		// uniform buffer objects for compute shaders
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&correctionUBO,
			sizeof(int32_t));

		// Map for host access
		VK_CHECK_RESULT(correctionUBO.map());

		// uniform buffer objects for render shaders
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&renderUBO,
			sizeof(int32_t)));

		// Map for host access
		VK_CHECK_RESULT(renderUBO.map());
		
		// uniform buffer objects for compute shaders
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&hc2srgbUBO,
			sizeof(int32_t)));

		// Map for host access
		VK_CHECK_RESULT(hc2srgbUBO.map());

		
		// Vertex shader uniform buffer block
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&uniformBufferVS,
			sizeof(uboVS)));

		// Map persistent
		VK_CHECK_RESULT(uniformBufferVS.map());

		updateUniformBuffers();
	}

	void HvsPipeline::updateUniformBuffers()
	{
		uboVS.projection = camera.matrices.perspective;
		uboVS.modelView = camera.matrices.view;
		memcpy(uniformBufferVS.mapped, &uboVS, sizeof(uboVS));

		float rho = 0.9f;
		memcpy(calibrateUBO.mapped, &rho, sizeof(float));

		memcpy(correctionUBO.mapped, &m_number_bands, sizeof(int32_t));

		memcpy(hc2srgbUBO.mapped, &m_number_bands_corrected, sizeof(int32_t));

	}

	void HvsPipeline::draw()
	{
		VulkanExampleBase::prepareFrame();

		VkPipelineStageFlags graphicsWaitStageMasks[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore graphicsWaitSemaphores[] = { compute.semaphore, semaphores.presentComplete };
		VkSemaphore graphicsSignalSemaphores[] = { graphics.semaphore, semaphores.renderComplete };

		//VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
		//vkCmdSetViewport(drawCmdBuffers[currentBuffer], 0, 1, &viewport);

		//vkCmdBindPipeline(drawCmdBuffers[currentBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipeline);

		//std::vector<VkViewport> viewports;
		//VkViewport viewport = vks::initializers::viewport((float)width * 0.5f, (float)height, 0.0f, 1.0f);
		//viewports.push_back(viewport);
		//vkCmdSetViewport(drawCmdBuffers[currentBuffer], 0, static_cast<uint32_t>(viewports.size()), viewports.data());


		// Submit graphics commands
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
		submitInfo.waitSemaphoreCount = 2;
		submitInfo.pWaitSemaphores = graphicsWaitSemaphores;
		submitInfo.pWaitDstStageMask = graphicsWaitStageMasks;
		submitInfo.signalSemaphoreCount = 2;
		submitInfo.pSignalSemaphores = graphicsSignalSemaphores;
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

		VulkanExampleBase::submitFrame();

		
		//viewport.x = (float)width / 2.0f;
		//vkCmdSetViewport(drawCmdBuffers[currentBuffer], 0, 1, &viewport);
		//VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

		//VulkanExampleBase::submitFrame();

		// Wait for rendering finished
		VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		// Dont have to wait for rendering to complete, can be done the other way around
		// Submit compute commands
		VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
		computeSubmitInfo.commandBufferCount = 1;
		computeSubmitInfo.pCommandBuffers = &compute.commandBuffer;
		computeSubmitInfo.waitSemaphoreCount = 1;
		computeSubmitInfo.pWaitSemaphores = &graphics.semaphore;
		computeSubmitInfo.pWaitDstStageMask = &waitStageMask;
		computeSubmitInfo.signalSemaphoreCount = 1;
		computeSubmitInfo.pSignalSemaphores = &compute.semaphore;
		VK_CHECK_RESULT(vkQueueSubmit(compute.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));
	}

	//void HvsPipeline::openCamera()
	//{
	//	if (!m_cap.open(0)) {
	//		std::cout << "error could not open webcam" << std::endl;
	//	}
	//	else
	//	{
	//		m_image_width = 1920;
	//		m_image_height = 1080;
	//		// try setting 1080p
	//		m_cap.set(cv::CAP_PROP_FRAME_WIDTH, m_image_width);
	//		m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, m_image_height);

	//		// read what is available to us
	//		m_image_width = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
	//		m_image_height = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
	//	}
	//}

	void HvsPipeline::openCamera() {
		//m_cam_grab.start();
	}

	void* HvsPipeline::getCameraFrame()
	{
		//m_cap.read(m_frame);
		//cv::cvtColor(m_frame, m_frame4, cv::COLOR_BGR2RGBA);

		return (void*)m_cam_grab.get_buffer();

		//return (void*)m_frame4.data;
	}

	void HvsPipeline::setCamera(CameraGrabber &cameraGrabber) {
		m_cam_grab = cameraGrabber;
		m_image_width = m_cam_grab.get_width();
		m_image_height = m_cam_grab.get_height();
	}

	void HvsPipeline::prepare()
	{
		//openCamera();

		VulkanExampleBase::prepare();

		//m_frame = cv::imread("C://data//logo.png", cv::ImreadModes::IMREAD_UNCHANGED);

		//m_image_width = m_frame.cols;
		//m_image_height = m_frame.rows;
		//loadImageBuffer(m_frame.data, m_image_width * m_image_height * sizeof(uint8_t) * 4, m_image_width, m_image_height);

		loadImageBuffer(getCameraFrame(), m_image_width * m_image_height * sizeof(uint16_t), m_image_width, m_image_height, VK_FORMAT_R16_UINT);

		generateQuad();
		setupVertexDescriptions();
		prepareUniformBuffers();

		// TEXTURES THAT HAVE HOST ACCESS
		std::vector<uint16_t> blankWhite(m_image_width * m_image_height, 600);
		std::vector<uint16_t> blankDark(m_image_width * m_image_height, 0);
		std::vector<uint8_t> blankMask(m_image_width * m_image_height, 255);


		// camera config json
		std::string camera_config_file = getAssetPath() + "/cameras/calibration/config.json";
		std::ifstream ifs(camera_config_file);
		json j;
		ifs >> j;

		//std::cout << "size : " << j["cameras"].size() << std::endl;
		//auto testCorMat = j["cameras"][0]["correction_matrix"]

		std::vector<float> correctionMatrixValues;
		for (auto& band : j["cameras"][1]["correction_matrix"]) {
			for (auto& elem : band){ 
				correctionMatrixValues.push_back(elem);
			}
		}

		std::vector<float> colorMatrixValues;
		for (auto& band : j["cameras"][1]["sRGB_correction_v2"]) {
			for (auto& elem : band){ 
				colorMatrixValues.push_back(elem);
			}
		}



		whiteReference.fromBuffer(blankWhite.data(), m_image_width * m_image_height * sizeof(uint16_t), VK_FORMAT_R16_UINT, m_image_width, m_image_height, vulkanDevice, queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
		darkReference.fromBuffer(blankDark.data(), m_image_width * m_image_height * sizeof(uint16_t), VK_FORMAT_R16_UINT, m_image_width, m_image_height, vulkanDevice, queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
		fovMask.fromBuffer(blankMask.data(), m_image_width * m_image_height * sizeof(uint8_t), VK_FORMAT_R8_UINT, m_image_width, m_image_height, vulkanDevice, queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);

		correctionMatrix.fromBuffer(correctionMatrixValues.data(), 16 * 16 * sizeof(float), VK_FORMAT_R32_SFLOAT, 16, 16, vulkanDevice, queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
		colorMatrix.fromBuffer(colorMatrixValues.data(), 16 * 4 * sizeof(float), VK_FORMAT_R32G32B32A32_SFLOAT, 16, 1, vulkanDevice, queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);

		//prepareTextureTarget(&whiteReference, m_image_width, m_image_height, m_image_depth, 1, VK_FORMAT_R16_UINT);
		//prepareTextureTarget(&darkReference, m_image_width, m_image_height, m_image_depth, 1, VK_FORMAT_R16_UINT);
		//prepareTextureTarget(&fovMask, m_image_width, m_image_height, m_image_depth, 1, VK_FORMAT_R8_UINT);

		// TEXTURES THAT ONLY EXIST IN SHADERS
		//prepareTextureTarget(&textureCalibrated, textureRaw.width, textureRaw.height, m_image_depth, 1, 1, VK_FORMAT_R8G8B8A8_UINT);
		prepareTextureTarget(&calibrated, m_image_width, m_image_height, m_image_depth, 1, 4, VK_FORMAT_R32_SFLOAT);

		
		prepareTextureTarget(&tiled, m_image_width / 4, m_image_height / 4, m_image_depth, 1, 1, VK_FORMAT_R32_SFLOAT);
		prepareTextureTarget(&tiledInput, m_image_width / 4, m_image_height / 4, m_image_depth, 1, 1, VK_FORMAT_R32_SFLOAT);
		prepareTextureTarget(&sRGB, m_image_width / 4, m_image_height / 4, 1, 1, 1, VK_FORMAT_R8G8B8A8_UINT);

		//sRGB.fromBuffer(blankMask.data(), (m_image_width / 4) * (m_image_height / 4) * 4 * sizeof(uint8_t), VK_FORMAT_R8G8B8A8_UINT, m_image_width / 4, m_image_height / 4, vulkanDevice, queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);


		prepareTextureTarget(&hypercubeCalibrated, m_image_width / 4, m_image_height / 4, m_image_depth, m_number_bands, 1, VK_FORMAT_R32_SFLOAT);
		prepareTextureTarget(&hypercubeCalibratedColorCorrected, m_image_width / 4, m_image_height / 4, m_image_depth, m_number_bands_corrected, 1, VK_FORMAT_R32_SFLOAT);

		//std::vector<float> output_image(m_image_width * m_image_height, -1.0f);
		//hypercubeCalibrated.fromBuffer(output_image.data(), m_image_width * m_image_height * sizeof(float_t), VK_FORMAT_R32_SFLOAT, m_image_width / 4, m_image_height / 4, m_image_depth, vulkanDevice, queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_LAYOUT_GENERAL);
		//hypercubeCalibratedColorCorrected.fromBuffer(output_image.data(), m_image_width * m_image_height * sizeof(float_t), VK_FORMAT_R32_SFLOAT, m_image_width / 4, m_image_height / 4, m_image_depth, vulkanDevice, queue, VK_FILTER_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_LAYOUT_GENERAL);


		//vks::Texture2DArray hypercubeCalibrated;
		//vks::Texture2DArray hypercubeColorCorrected;

		//vks::Texture2D sRGB;



		setupDescriptorSetLayout();
		preparePipelines();
		setupDescriptorPool();
		setupDescriptorSet();
		prepareGraphics();
		prepareCompute();
		buildCommandBuffers();
		prepared = true;
	}

	void HvsPipeline::render()
	{
		if (!prepared)
			return;
		updateImageBuffer(getCameraFrame(), m_image_width * m_image_height * sizeof(uint16_t), VK_FORMAT_R16_UINT);

		draw();
		if (camera.updated) {
			//loadImageBuffer();
			updateUniformBuffers();
		}
	}

	void HvsPipeline::OnUpdateUIOverlay(vks::UIOverlay* overlay)
	{
		if (overlay->header("Settings")) {


			if (overlay->comboBox("Shader", &compute.pipelineIndex, shaderNames)) {
							auto im = getCameraFrame();
			std::string imageOutFile = getAssetPath() + "../data/tests/raw.png";

			cv::imwrite(imageOutFile, cv::Mat(cv::Size(2048, 1088), CV_16SC1, im));
				buildComputeCommandBuffer();
			}
		}
	}


