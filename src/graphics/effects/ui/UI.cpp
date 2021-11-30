#include "UI.h"
#include "../../../graphics/resources/RendererResources.h"
#include "../../../graphics/resources/Samplers.h"
#include "../../../graphics/resources/ShaderResources.h"
#include "../../../window/WindowResources.h"

void UI::init(Viewport fullscreenViewport) {
	viewport.init(static_cast<uint32_t>(fullscreenViewport.viewport.width), static_cast<uint32_t>(fullscreenViewport.viewport.height));

	{
		std::vector<RenderPassAttachment> attachments;
		attachments.push_back(RenderPassAttachment(AttachmentType::COLOR, swapchain.surfaceFormat.format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));

		std::vector<SubpassDependency> dependencies;
		dependencies.push_back({ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT });
		dependencies.push_back({ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT });

		renderPass.init(attachments, dependencies);
	}

	BufferTools::createUniformBuffer(cameraBuffer.buffer, sizeof(CameraUniformBufferObject), &cameraBuffer.memoryInfo);

	// Descriptor sets
	// Sprites descriptor sets
	{
		// Descriptor Pool
		VkDescriptorPoolSize spritePoolSize = {};
		spritePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		spritePoolSize.descriptorCount = 524288;

		VkDescriptorPoolCreateInfo spritesPoolCreateInfo = {};
		spritesPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		spritesPoolCreateInfo.pNext = nullptr;
		spritesPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		spritesPoolCreateInfo.maxSets = framesInFlight;
		spritesPoolCreateInfo.poolSizeCount = 1;
		spritesPoolCreateInfo.pPoolSizes = &spritePoolSize;
		NEIGE_VK_CHECK(vkCreateDescriptorPool(logicalDevice.device, &spritesPoolCreateInfo, nullptr, &spritesDescriptorPool.descriptorPool));

		spritesDescriptorPool.remainingSets = framesInFlight;

		// Descriptor Set Layout
		VkDescriptorSetLayoutBinding spritesBinding = {};
		spritesBinding.binding = 0;
		spritesBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		spritesBinding.descriptorCount = 524288;
		spritesBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		spritesBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBindingFlagsCreateInfo spritesLayoutBindingFlagsCreateInfo = {};
		spritesLayoutBindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		spritesLayoutBindingFlagsCreateInfo.pNext = nullptr;
		spritesLayoutBindingFlagsCreateInfo.bindingCount = 1;
		std::array<VkDescriptorBindingFlags, 1> flags = { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT };
		spritesLayoutBindingFlagsCreateInfo.pBindingFlags = flags.data();

		VkDescriptorSetLayoutCreateInfo spritesSetLayoutCreateInfo = {};
		spritesSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		spritesSetLayoutCreateInfo.pNext = &spritesLayoutBindingFlagsCreateInfo;
		spritesSetLayoutCreateInfo.flags = 0;
		spritesSetLayoutCreateInfo.bindingCount = 1;
		spritesSetLayoutCreateInfo.pBindings = &spritesBinding;
		NEIGE_VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice.device, &spritesSetLayoutCreateInfo, nullptr, &spritesDescriptorSetLayout));

		// Allocation
		spritesDescriptorSets.resize(framesInFlight);
		spriteDescriptorSetUpToDate.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++) {
			VkDescriptorSetAllocateInfo spritesAllocateInfo = {};
			spritesAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			spritesAllocateInfo.pNext = nullptr;
			spritesAllocateInfo.descriptorPool = spritesDescriptorPool.descriptorPool;
			spritesAllocateInfo.descriptorSetCount = 1;
			spritesAllocateInfo.pSetLayouts = &spritesDescriptorSetLayout;
			NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &spritesAllocateInfo, &spritesDescriptorSets[i].descriptorSet));

			spritesDescriptorSets[i].descriptorPool = &spritesDescriptorPool;
			spriteDescriptorSetUpToDate[i] = false;
		}
	}

	// Fonts descriptor sets
	{
		// Descriptor Pool
		VkDescriptorPoolSize fontPoolSize = {};
		fontPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		fontPoolSize.descriptorCount = 524288;

		VkDescriptorPoolCreateInfo fontsPoolCreateInfo = {};
		fontsPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		fontsPoolCreateInfo.pNext = nullptr;
		fontsPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		fontsPoolCreateInfo.maxSets = framesInFlight;
		fontsPoolCreateInfo.poolSizeCount = 1;
		fontsPoolCreateInfo.pPoolSizes = &fontPoolSize;
		NEIGE_VK_CHECK(vkCreateDescriptorPool(logicalDevice.device, &fontsPoolCreateInfo, nullptr, &fontsDescriptorPool.descriptorPool));

		fontsDescriptorPool.remainingSets = framesInFlight;

		// Descriptor Set Layout
		VkDescriptorSetLayoutBinding fontsBinding = {};
		fontsBinding.binding = 0;
		fontsBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		fontsBinding.descriptorCount = 524288;
		fontsBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		fontsBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBindingFlagsCreateInfo fontsLayoutBindingFlagsCreateInfo = {};
		fontsLayoutBindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		fontsLayoutBindingFlagsCreateInfo.pNext = nullptr;
		fontsLayoutBindingFlagsCreateInfo.bindingCount = 1;
		std::array<VkDescriptorBindingFlags, 1> flags = { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT };
		fontsLayoutBindingFlagsCreateInfo.pBindingFlags = flags.data();

		VkDescriptorSetLayoutCreateInfo fontsSetLayoutCreateInfo = {};
		fontsSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		fontsSetLayoutCreateInfo.pNext = &fontsLayoutBindingFlagsCreateInfo;
		fontsSetLayoutCreateInfo.flags = 0;
		fontsSetLayoutCreateInfo.bindingCount = 1;
		fontsSetLayoutCreateInfo.pBindings = &fontsBinding;
		NEIGE_VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice.device, &fontsSetLayoutCreateInfo, nullptr, &fontsDescriptorSetLayout));

		// Allocation
		fontsDescriptorSets.resize(framesInFlight);
		fontDescriptorSetUpToDate.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++) {
			VkDescriptorSetAllocateInfo fontsAllocateInfo = {};
			fontsAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			fontsAllocateInfo.pNext = nullptr;
			fontsAllocateInfo.descriptorPool = fontsDescriptorPool.descriptorPool;
			fontsAllocateInfo.descriptorSetCount = 1;
			fontsAllocateInfo.pSetLayouts = &fontsDescriptorSetLayout;
			NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &fontsAllocateInfo, &fontsDescriptorSets[i].descriptorSet));

			fontsDescriptorSets[i].descriptorPool = &fontsDescriptorPool;
			fontDescriptorSetUpToDate[i] = false;
		}
	}

	// Graphics pipelines
	spriteGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/ui/ui.vert";
	spriteGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/ui/sprite.frag";
	spriteGraphicsPipeline.renderPass = &renderPass;
	spriteGraphicsPipeline.viewport = &viewport;
	spriteGraphicsPipeline.multiSample = false;
	spriteGraphicsPipeline.backfaceCulling = false;
	spriteGraphicsPipeline.depthWrite = false;
	spriteGraphicsPipeline.blendings.push_back({ VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD });
	spriteGraphicsPipeline.externalSets.push_back(1);
	spriteGraphicsPipeline.externalDescriptorSetLayouts.push_back(spritesDescriptorSetLayout);
	spriteGraphicsPipeline.init();

	textGraphicsPipeline.vertexShaderPath = "../src/graphics/shaders/ui/ui.vert";
	textGraphicsPipeline.fragmentShaderPath = "../src/graphics/shaders/ui/text.frag";
	textGraphicsPipeline.renderPass = &renderPass;
	textGraphicsPipeline.viewport = &viewport;
	textGraphicsPipeline.multiSample = false;
	textGraphicsPipeline.backfaceCulling = false;
	textGraphicsPipeline.depthWrite = false;
	textGraphicsPipeline.blendings.push_back({ VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD });
	textGraphicsPipeline.externalSets.push_back(1);
	textGraphicsPipeline.externalDescriptorSetLayouts.push_back(fontsDescriptorSetLayout);
	textGraphicsPipeline.init();

	// Camera descriptor set
	{
		cameraDescriptorSet.init(&spriteGraphicsPipeline, 0);

		VkDescriptorBufferInfo cameraInfo = {};
		cameraInfo.buffer = cameraBuffer.buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(CameraUniformBufferObject);

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet cameraWriteDescriptorSet = {};
		cameraWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraWriteDescriptorSet.pNext = nullptr;
		cameraWriteDescriptorSet.dstSet = cameraDescriptorSet.descriptorSet;
		cameraWriteDescriptorSet.dstBinding = 0;
		cameraWriteDescriptorSet.dstArrayElement = 0;
		cameraWriteDescriptorSet.descriptorCount = 1;
		cameraWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraWriteDescriptorSet.pImageInfo = nullptr;
		cameraWriteDescriptorSet.pBufferInfo = &cameraInfo;
		cameraWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(cameraWriteDescriptorSet);

		cameraDescriptorSet.update(writesDescriptorSet);
	}

	createResources(fullscreenViewport);
}

void UI::destroy() {
	destroyResources();
	renderPass.destroy();
	spriteGraphicsPipeline.destroy();
	textGraphicsPipeline.destroy();
	cameraBuffer.destroy();
	if (spritesDescriptorSetLayout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(logicalDevice.device, spritesDescriptorSetLayout, nullptr);
		spritesDescriptorSetLayout = VK_NULL_HANDLE;
	}
	spritesDescriptorPool.destroy();
	for (SpriteImage spriteImage : spritesImages) {
		spriteImage.image.destroy();
	}
	if (fontsDescriptorSetLayout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(logicalDevice.device, fontsDescriptorSetLayout, nullptr);
		fontsDescriptorSetLayout = VK_NULL_HANDLE;
	}
	fontsDescriptorPool.destroy();
	for (Font font : fonts) {
		font.image.destroy();
	}
}

void UI::createResources(Viewport fullscreenViewport) {
	viewport.init(static_cast<uint32_t>(fullscreenViewport.viewport.width), static_cast<uint32_t>(fullscreenViewport.viewport.height));

	// Framebuffers
	framebuffers.resize(swapchainSize);

	{
		std::vector<std::vector<VkImageView>> framebufferAttachments;
		framebufferAttachments.resize(swapchainSize);
		framebuffers.resize(swapchainSize);
		for (uint32_t i = 0; i < swapchainSize; i++) {
			framebufferAttachments[i].push_back(swapchain.imageViews[i]);
			framebuffers[i].init(&renderPass, framebufferAttachments[i], static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height), 1);
		}
	}

	// Camera
	CameraUniformBufferObject cubo = {};
	cubo.view = glm::mat4(0.0f);
	cubo.projection = Camera::createOrthoProjection(0.0f, fullscreenViewport.viewport.width, 0.0f, fullscreenViewport.viewport.height);
	cubo.position = glm::vec3(0.0f);

	memcpy(reinterpret_cast<void*>(reinterpret_cast<char*>(cameraBuffer.memoryInfo.data) + cameraBuffer.memoryInfo.offset), &cubo, sizeof(CameraUniformBufferObject));
}

void UI::destroyResources() {
	for (Framebuffer& framebuffer : framebuffers) {
		framebuffer.destroy();
	}
	framebuffers.clear();
	framebuffers.shrink_to_fit();
}

void UI::updateSpriteDescriptorSet(uint32_t frameInFlightIndex) {
	std::vector<VkDescriptorImageInfo> spriteInfos;
	for (SpriteImage& spriteImage : spritesImages) {
		VkDescriptorImageInfo spriteImageInfo = {};
		spriteImageInfo.sampler = nearestEdgeOneLodBlackSampler;
		spriteImageInfo.imageView = spriteImage.image.imageView;
		spriteImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		spriteInfos.push_back(spriteImageInfo);
	}

	std::vector<VkWriteDescriptorSet> writesDescriptorSet;

	VkWriteDescriptorSet spriteWriteDescriptorSet = {};
	spriteWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	spriteWriteDescriptorSet.pNext = nullptr;
	spriteWriteDescriptorSet.dstSet = spritesDescriptorSets[frameInFlightIndex].descriptorSet;
	spriteWriteDescriptorSet.dstBinding = 0;
	spriteWriteDescriptorSet.dstArrayElement = 0;
	spriteWriteDescriptorSet.descriptorCount = static_cast<uint32_t>(spritesImages.size());
	spriteWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	spriteWriteDescriptorSet.pImageInfo = spriteInfos.data();
	spriteWriteDescriptorSet.pBufferInfo = nullptr;
	spriteWriteDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(spriteWriteDescriptorSet);

	spritesDescriptorSets[frameInFlightIndex].update(writesDescriptorSet);
}

void UI::updateFontDescriptorSet(uint32_t frameInFlightIndex) {
	std::vector<VkDescriptorImageInfo> fontInfos;
	for (Font& font : fonts) {
		VkDescriptorImageInfo fontInfo = {};
		fontInfo.sampler = fontSampler;
		fontInfo.imageView = font.image.imageView;
		fontInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		fontInfos.push_back(fontInfo);
	}

	std::vector<VkWriteDescriptorSet> writesDescriptorSet;

	VkWriteDescriptorSet fontWriteDescriptorSet = {};
	fontWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	fontWriteDescriptorSet.pNext = nullptr;
	fontWriteDescriptorSet.dstSet = fontsDescriptorSets[frameInFlightIndex].descriptorSet;
	fontWriteDescriptorSet.dstBinding = 0;
	fontWriteDescriptorSet.dstArrayElement = 0;
	fontWriteDescriptorSet.descriptorCount = static_cast<uint32_t>(fonts.size());
	fontWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	fontWriteDescriptorSet.pImageInfo = fontInfos.data();
	fontWriteDescriptorSet.pBufferInfo = nullptr;
	fontWriteDescriptorSet.pTexelBufferView = nullptr;
	writesDescriptorSet.push_back(fontWriteDescriptorSet);

	fontsDescriptorSets[frameInFlightIndex].update(writesDescriptorSet);
}

void UI::draw(CommandBuffer* commandBuffer, uint32_t framebufferIndex) {
	renderPass.begin(commandBuffer, framebuffers[framebufferIndex].framebuffer, { static_cast<uint32_t>(viewport.viewport.width), static_cast<uint32_t>(viewport.viewport.height) });

	currentGraphicsPipeline = nullptr;
	bool setsBound = false;

	// Sprites
	while (!elementsToDraw.empty()) {
		UIElement element = elementsToDraw.front();

		if (element == UIElement::SPRITE) {
			if (&spriteGraphicsPipeline != currentGraphicsPipeline) {
				spriteGraphicsPipeline.bind(commandBuffer);

				currentGraphicsPipeline = &spriteGraphicsPipeline;
				setsBound = false;
			}

			if (!setsBound) {
				cameraDescriptorSet.bind(commandBuffer, &spriteGraphicsPipeline, 0);
				spritesDescriptorSets[framebufferIndex].bind(commandBuffer, &spriteGraphicsPipeline, 1);

				setsBound = true;
			}

			Sprite sprite = sprites.front();
			drawSprite(commandBuffer, sprite);
			sprites.pop();
		}
		else if (element == UIElement::TEXT) {
			if (&textGraphicsPipeline != currentGraphicsPipeline) {
				textGraphicsPipeline.bind(commandBuffer);

				currentGraphicsPipeline = &textGraphicsPipeline;
				setsBound = false;
			}

			if (!setsBound) {
				cameraDescriptorSet.bind(commandBuffer, &spriteGraphicsPipeline, 0);
				fontsDescriptorSets[framebufferIndex].bind(commandBuffer, &textGraphicsPipeline, 1);

				setsBound = true;
			}

			Text text = texts.front();
			drawText(commandBuffer, text);
			texts.pop();
		}

		elementsToDraw.pop();
	}

	renderPass.end(commandBuffer);
}

void UI::drawSprite(CommandBuffer* commandBuffer, Sprite sprite) {
	SpriteImage* spriteImage = &spritesImages[sprite.spriteIndex];
	float width = static_cast<float>(spriteImage->width);
	float height = static_cast<float>(spriteImage->height);

	float data[24] = { /* Position */ /* 0 */ sprite.position.x, sprite.position.y, /* 1 */ sprite.position.x + width, sprite.position.y, /* 2 */ sprite.position.x + width, sprite.position.y + height, /* 3 */ sprite.position.x, sprite.position.y, /* 4 */ sprite.position.x + width, sprite.position.y + height, /* 5 */ sprite.position.x, sprite.position.y + height, /* UV */ /* 0 */ 0.0f, 0.0f, /* 1 */ 1.0f, 0.0f, /* 2 */ 1.0f, 1.0f, /* 3 */ 0.0f, 0.0f, /* 4 */ 1.0f, 1.0f, /* 5 */ 0.0f, 1.0f };

	spriteGraphicsPipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_VERTEX_BIT, 0, ((2 * 6) + (2 * 6)) * sizeof(float), data);
	spriteGraphicsPipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, ((2 * 6) + (2 * 6)) * sizeof(float), sizeof(int), &sprite.spriteIndex);

	vkCmdDraw(commandBuffer->commandBuffer, 6, 1, 0, 0);
}

void UI::drawText(CommandBuffer* commandBuffer, Text text) {
	for (std::string::const_iterator c = text.text.begin(); c != text.text.end(); c++) {
		if (*c >= fonts[text.fontIndex].startChar && *c <= fonts[text.fontIndex].endChar) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(fonts[text.fontIndex].backedChar, 512, 512, *c - 32, &text.position.x, &text.position.y, &q, 1);

			float data[24] = { /* Position */ /* 0 */ q.x0, q.y0, /* 1 */ q.x1, q.y0, /* 2 */ q.x1, q.y1, /* 3 */ q.x0, q.y0, /* 4 */ q.x1, q.y1, /* 5 */ q.x0, q.y1, /* UV */ /* 0 */ q.s0, q.t0, /* 1 */ q.s1, q.t0, /* 2 */ q.s1, q.t1, /* 3 */ q.s0, q.t0, /* 4 */ q.s1, q.t1, /* 5 */ q.s0, q.t1 };

			textGraphicsPipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_VERTEX_BIT, 0, ((2 * 6) + (2 * 6)) * sizeof(float), data);
			textGraphicsPipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, ((2 * 6) + (2 * 6)) * sizeof(float), 3 * sizeof(float), &text.color);
			textGraphicsPipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, ((2 * 6) + (2 * 6)) * sizeof(float) + (3 * sizeof(float)), sizeof(int), &text.fontIndex);

			vkCmdDraw(commandBuffer->commandBuffer, 6, 1, 0, 0);
		}
	}
}