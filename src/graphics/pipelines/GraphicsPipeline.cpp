#include "GraphicsPipeline.h"
#include "../resources/RendererResources.h"
#include "../resources/ShaderResources.h"

void GraphicsPipeline::init() {
	sets.clear();
	sets.shrink_to_fit();
	pushConstantRanges.clear();
	pushConstantRanges.shrink_to_fit();

	std::vector<VkPipelineShaderStageCreateInfo> pipelineStages;
	std::set<VkDescriptorType> uniqueDescriptorTypes;
	std::vector<InputVariable> inputVariables;

	if (vertexShaderPath != "") {
		Shader shader;
		std::unordered_map<std::string, Shader>::const_iterator mapSearch = shaders.find(vertexShaderPath);
		if (mapSearch == shaders.end()) {
			shader.init(vertexShaderPath);
			shaders.emplace(vertexShaderPath, shader);
		}
		else {
			shader = shaders[vertexShaderPath];
		}
		NEIGE_ASSERT(shader.type == ShaderType::VERTEX, "Vertex shader in pipeline is not a vertex shader.");

		VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
		vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShaderCreateInfo.pNext = nullptr;
		vertexShaderCreateInfo.flags = 0;
		vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexShaderCreateInfo.module = shader.module;
		vertexShaderCreateInfo.pName = "main";
		vertexShaderCreateInfo.pSpecializationInfo = nullptr;
		pipelineStages.push_back(vertexShaderCreateInfo);

		inputVariables.insert(inputVariables.end(), shader.inputVariables.begin(), shader.inputVariables.end());
		for (size_t i = 0; i < shader.sets.size(); i++) {
			bool found = false;
			for (size_t j = 0; j < sets.size(); j++) {
				if (sets[j].set == shader.sets[i].set) {
					sets[j].bindings.insert(sets[j].bindings.end(), shader.sets[i].bindings.begin(), shader.sets[i].bindings.end());
					found = true;
				}
			}
			if (!found) {
				sets.push_back(shader.sets[i]);
			}
		}
		pushConstantRanges.insert(pushConstantRanges.end(), shader.pushConstantRanges.begin(), shader.pushConstantRanges.end());
		uniqueDescriptorTypes.insert(shader.uniqueDescriptorTypes.begin(), shader.uniqueDescriptorTypes.end());
	}

	VkSpecializationInfo fragmentSpecialization = {};
	VkSpecializationMapEntry fragmentSpecializationEntry = {};
	if (fragmentShaderPath != "") {
		Shader shader;
		std::unordered_map<std::string, Shader>::const_iterator mapSearch = shaders.find(fragmentShaderPath);
		if (mapSearch == shaders.end()) {
			shader.init(fragmentShaderPath);
			shaders.emplace(fragmentShaderPath, shader);
		}
		else {
			shader = shaders[fragmentShaderPath];
		}
		NEIGE_ASSERT(shader.type == ShaderType::FRAGMENT, "Fragment shader in pipeline is not a fragment shader.");

		if (shader.specializationConstants.size() != 0 && specializationConstantValues.size() != 0) {
			fragmentSpecializationEntry.constantID = 0;
			fragmentSpecializationEntry.offset = 0;
			fragmentSpecializationEntry.size = sizeof(int);

			fragmentSpecialization.mapEntryCount = 1;
			fragmentSpecialization.pMapEntries = &fragmentSpecializationEntry;
			fragmentSpecialization.dataSize = sizeof(int);
			fragmentSpecialization.pData = &specializationConstantValues[0];
		}

		VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
		fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentShaderCreateInfo.pNext = nullptr;
		fragmentShaderCreateInfo.flags = 0;
		fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentShaderCreateInfo.module = shader.module;
		fragmentShaderCreateInfo.pName = "main";
		fragmentShaderCreateInfo.pSpecializationInfo = (shader.specializationConstants.size() != 0 && specializationConstantValues.size() != 0) ? &fragmentSpecialization : nullptr;
		pipelineStages.push_back(fragmentShaderCreateInfo);

		for (size_t i = 0; i < shader.sets.size(); i++) {
			bool found = false;
			for (size_t j = 0; j < sets.size(); j++) {
				if (sets[j].set == shader.sets[i].set) {
					sets[j].bindings.insert(sets[j].bindings.end(), shader.sets[i].bindings.begin(), shader.sets[i].bindings.end());
					found = true;
				}
			}
			if (!found) {
				sets.push_back(shader.sets[i]);
			}
		}

		if (pushConstantRanges.size() != 0 && shader.pushConstantRanges.size() != 0) {
			shader.pushConstantRanges[shader.pushConstantRanges.size() - 1].offset += pushConstantRanges[pushConstantRanges.size() - 1].size;
		}
		pushConstantRanges.insert(pushConstantRanges.end(), shader.pushConstantRanges.begin(), shader.pushConstantRanges.end());
		uniqueDescriptorTypes.insert(shader.uniqueDescriptorTypes.begin(), shader.uniqueDescriptorTypes.end());
	}

	if (tesselationControlShaderPath != "") {
		Shader shader;
		std::unordered_map<std::string, Shader>::const_iterator mapSearch = shaders.find(tesselationControlShaderPath);
		if (mapSearch == shaders.end()) {
			shader.init(tesselationControlShaderPath);
			shaders.emplace(tesselationControlShaderPath, shader);
		}
		else {
			shader = shaders[tesselationControlShaderPath];
		}
		NEIGE_ASSERT(shader.type == ShaderType::TESSELATION_CONTROL, "Tesselation control shader in pipeline is not a tesselation control shader.");

		VkPipelineShaderStageCreateInfo tesselationControlShaderCreateInfo = {};
		tesselationControlShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		tesselationControlShaderCreateInfo.pNext = nullptr;
		tesselationControlShaderCreateInfo.flags = 0;
		tesselationControlShaderCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		tesselationControlShaderCreateInfo.module = shader.module;
		tesselationControlShaderCreateInfo.pName = "main";
		tesselationControlShaderCreateInfo.pSpecializationInfo = nullptr;
		pipelineStages.push_back(tesselationControlShaderCreateInfo);

		for (size_t i = 0; i < shader.sets.size(); i++) {
			bool found = false;
			for (size_t j = 0; j < sets.size(); j++) {
				if (sets[j].set == shader.sets[i].set) {
					sets[j].bindings.insert(sets[j].bindings.end(), shader.sets[i].bindings.begin(), shader.sets[i].bindings.end());
					found = true;
				}
			}
			if (!found) {
				sets.push_back(shader.sets[i]);
			}
		}

		if (pushConstantRanges.size() != 0 && shader.pushConstantRanges.size() != 0) {
			shader.pushConstantRanges[shader.pushConstantRanges.size() - 1].offset += pushConstantRanges[pushConstantRanges.size() - 1].size;
		}
		pushConstantRanges.insert(pushConstantRanges.end(), shader.pushConstantRanges.begin(), shader.pushConstantRanges.end());
		uniqueDescriptorTypes.insert(shader.uniqueDescriptorTypes.begin(), shader.uniqueDescriptorTypes.end());
	}

	if (tesselationEvaluationShaderPath != "") {
		Shader shader;
		std::unordered_map<std::string, Shader>::const_iterator mapSearch = shaders.find(tesselationEvaluationShaderPath);
		if (mapSearch == shaders.end()) {
			shader.init(tesselationEvaluationShaderPath);
			shaders.emplace(tesselationEvaluationShaderPath, shader);
		}
		else {
			shader = shaders[tesselationEvaluationShaderPath];
		}
		NEIGE_ASSERT(shader.type == ShaderType::TESSELATION_EVALUATION, "Tesselation evaluation shader in pipeline is not a tesselation evaluation shader.");

		VkPipelineShaderStageCreateInfo tesselationEvaluationShaderCreateInfo = {};
		tesselationEvaluationShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		tesselationEvaluationShaderCreateInfo.pNext = nullptr;
		tesselationEvaluationShaderCreateInfo.flags = 0;
		tesselationEvaluationShaderCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		tesselationEvaluationShaderCreateInfo.module = shader.module;
		tesselationEvaluationShaderCreateInfo.pName = "main";
		tesselationEvaluationShaderCreateInfo.pSpecializationInfo = nullptr;
		pipelineStages.push_back(tesselationEvaluationShaderCreateInfo);

		for (size_t i = 0; i < shader.sets.size(); i++) {
			bool found = false;
			for (size_t j = 0; j < sets.size(); j++) {
				if (sets[j].set == shader.sets[i].set) {
					sets[j].bindings.insert(sets[j].bindings.end(), shader.sets[i].bindings.begin(), shader.sets[i].bindings.end());
					found = true;
				}
			}
			if (!found) {
				sets.push_back(shader.sets[i]);
			}
		}

		if (pushConstantRanges.size() != 0 && shader.pushConstantRanges.size() != 0) {
			shader.pushConstantRanges[shader.pushConstantRanges.size() - 1].offset += pushConstantRanges[pushConstantRanges.size() - 1].size;
		}
		pushConstantRanges.insert(pushConstantRanges.end(), shader.pushConstantRanges.begin(), shader.pushConstantRanges.end());
		uniqueDescriptorTypes.insert(shader.uniqueDescriptorTypes.begin(), shader.uniqueDescriptorTypes.end());
	}

	if (geometryShaderPath != "") {
		Shader shader;
		std::unordered_map<std::string, Shader>::const_iterator mapSearch = shaders.find(geometryShaderPath);
		if (mapSearch == shaders.end()) {
			shader.init(geometryShaderPath);
			shaders.emplace(geometryShaderPath, shader);
		}
		else {
			shader = shaders[geometryShaderPath];
		}
		NEIGE_ASSERT(shader.type == ShaderType::GEOMETRY, "Geometry shader in pipeline is not a geometry shader.");

		VkPipelineShaderStageCreateInfo geometryShaderCreateInfo = {};
		geometryShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		geometryShaderCreateInfo.pNext = nullptr;
		geometryShaderCreateInfo.flags = 0;
		geometryShaderCreateInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		geometryShaderCreateInfo.module = shader.module;
		geometryShaderCreateInfo.pName = "main";
		geometryShaderCreateInfo.pSpecializationInfo = nullptr;
		pipelineStages.push_back(geometryShaderCreateInfo);

		for (size_t i = 0; i < shader.sets.size(); i++) {
			bool found = false;
			for (size_t j = 0; j < sets.size(); j++) {
				if (sets[j].set == shader.sets[i].set) {
					sets[j].bindings.insert(sets[j].bindings.end(), shader.sets[i].bindings.begin(), shader.sets[i].bindings.end());
					found = true;
				}
			}
			if (!found) {
				sets.push_back(shader.sets[i]);
			}
		}

		if (pushConstantRanges.size() != 0 && shader.pushConstantRanges.size() != 0) {
			shader.pushConstantRanges[shader.pushConstantRanges.size() - 1].offset += pushConstantRanges[pushConstantRanges.size() - 1].size;
		}
		pushConstantRanges.insert(pushConstantRanges.end(), shader.pushConstantRanges.begin(), shader.pushConstantRanges.end());
		uniqueDescriptorTypes.insert(shader.uniqueDescriptorTypes.begin(), shader.uniqueDescriptorTypes.end());
	}

	NEIGE_ASSERT(pipelineStages.size() != 0, "Graphics pipeline got no stage (no shader given).");

	// Sort input variables
	std::sort(inputVariables.begin(), inputVariables.end(), [](InputVariable a, InputVariable b) { return a.location < b.location; });

	// Sort sets
	std::sort(sets.begin(), sets.end(), [](Set a, Set b) { return a.set < b.set; });
	for (size_t i = 0; i < sets.size(); i++) {
		// Sort bindings
		std::sort(sets[i].bindings.begin(), sets[i].bindings.end(), [](Binding a, Binding b) { return a.binding.binding < b.binding.binding; });
	}

	std::vector<std::vector<VkDescriptorSetLayoutBinding>> setBindings;
	setBindings.resize(sets.size());
	for (size_t i = 0; i < sets.size(); i++) {
		for (size_t j = 0; j < sets[i].bindings.size(); j++) {
			bool found = false;
			for (size_t k = 0; k < setBindings[i].size(); k++) {
				if (setBindings[i][k].binding == sets[i].bindings[j].binding.binding) {
					// Add stage flag
					setBindings[i][k].stageFlags |= sets[i].bindings[j].binding.stageFlags;
					found = true;
					break;
				}
			}
			if (!found) {
				setBindings[i].push_back(sets[i].bindings[j].binding);
			}
		}
	}

	// Descriptor set layouts
	descriptorSetLayouts.resize(sets.size());
	for (size_t i = 0; i < sets.size(); i++) {
		if (externalSet(i) == -1 && descriptorSetLayouts[i] == VK_NULL_HANDLE) {
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
			descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutCreateInfo.pNext = nullptr;
			descriptorSetLayoutCreateInfo.flags = 0;
			descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(setBindings[i].size());
			descriptorSetLayoutCreateInfo.pBindings = setBindings[i].data();
			NEIGE_VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice.device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayouts[i]));
		}
	}
	// External sets
	for (size_t i = 0; i < externalSets.size(); i++) {
		descriptorSetLayouts[externalSets[i]] = externalDescriptorSetLayouts[i];
	}

	// Descriptor pool
	if (descriptorPools.empty()) {
		descriptorPools.resize(1);
		descriptorPools[0].descriptorPool = VK_NULL_HANDLE;
		descriptorPools[0].remainingSets = 0;
	}
	for (size_t i = 0; i < descriptorPools.size(); i++) {
		if (descriptorPools[i].descriptorPool == VK_NULL_HANDLE) {
			for (std::set<VkDescriptorType>::iterator it = uniqueDescriptorTypes.begin(); it != uniqueDescriptorTypes.end(); it++) {
				VkDescriptorPoolSize descriptorPoolSize = {};
				descriptorPoolSize.type = *it;
				descriptorPoolSize.descriptorCount = 2048;
				descriptorPoolSizes.push_back(descriptorPoolSize);
			}

			if (descriptorPoolSizes.size() != 0) {
				VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
				descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				descriptorPoolCreateInfo.pNext = nullptr;
				descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
				descriptorPoolCreateInfo.maxSets = 8192;
				descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
				descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
				NEIGE_VK_CHECK(vkCreateDescriptorPool(logicalDevice.device, &descriptorPoolCreateInfo, nullptr, &descriptorPools[i].descriptorPool));

				descriptorPools[i].remainingSets = 8192;
			}
		}
	}

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
	pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
	NEIGE_VK_CHECK(vkCreatePipelineLayout(logicalDevice.device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	// Vertex input
	VkVertexInputBindingDescription inputBindingDescription = Vertex::getInputBindingDescription();
	std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions = Vertex::getInputAttributeDescriptions(inputVariables);

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.pNext = nullptr;
	vertexInputCreateInfo.flags = 0;
	if (inputAttributeDescriptions.size() == 0) {
		vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
		vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
	}
	else {
		vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
		vertexInputCreateInfo.pVertexBindingDescriptions = &inputBindingDescription;
	}
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(inputAttributeDescriptions.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions = inputAttributeDescriptions.data();

	// Assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.pNext = nullptr;
	inputAssemblyCreateInfo.flags = 0;
	inputAssemblyCreateInfo.topology = topologyToVkTopology();
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	// Rasterization
	VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};
	rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationCreateInfo.pNext = nullptr;
	rasterizationCreateInfo.flags = 0;
	rasterizationCreateInfo.depthClampEnable = VK_FALSE;
	rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationCreateInfo.polygonMode = topology != Topology::WIREFRAME ? VK_POLYGON_MODE_FILL : VK_POLYGON_MODE_LINE;
	rasterizationCreateInfo.cullMode = !backfaceCulling ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;
	rasterizationCreateInfo.frontFace = frontFaceCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
	rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizationCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterizationCreateInfo.depthBiasClamp = 0.0f;
	rasterizationCreateInfo.depthBiasSlopeFactor = 0.0f;
	rasterizationCreateInfo.lineWidth = 1.0f;

	// Multisample
	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.pNext = nullptr;
	multisampleCreateInfo.flags = 0;
	multisampleCreateInfo.rasterizationSamples = multiSample ? physicalDevice.maxUsableSampleCount : VK_SAMPLE_COUNT_1_BIT;
	multisampleCreateInfo.sampleShadingEnable = VK_TRUE;
	multisampleCreateInfo.minSampleShading = 0.1f;
	multisampleCreateInfo.pSampleMask = nullptr;
	multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

	// Depth stencil
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.pNext = nullptr;
	depthStencilCreateInfo.flags = 0;
	depthStencilCreateInfo.depthTestEnable = depthTest ? VK_TRUE : VK_FALSE;
	depthStencilCreateInfo.depthWriteEnable = depthWrite ? VK_TRUE : VK_FALSE;
	depthStencilCreateInfo.depthCompareOp = compareToVkCompare();
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
	depthStencilCreateInfo.front = {};
	depthStencilCreateInfo.back = {};
	depthStencilCreateInfo.minDepthBounds = 0.0f;
	depthStencilCreateInfo.maxDepthBounds = 1.0f;

	// Color blend
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	for (size_t i = 0; i < renderPass->colorAttachmentReferences.size(); i++) {
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		if (i < blendings.size()) {
			colorBlendAttachment.blendEnable = blendings[i].blendEnable;
			colorBlendAttachment.srcColorBlendFactor = blendings[i].srcColorBlendFactor;
			colorBlendAttachment.dstColorBlendFactor = blendings[i].dstColorBlendFactor;
			colorBlendAttachment.colorBlendOp = blendings[i].colorBlendOp;
			colorBlendAttachment.srcAlphaBlendFactor = blendings[i].srcAlphaBlendFactor;
			colorBlendAttachment.dstAlphaBlendFactor = blendings[i].dstAlphaBlendFactor;
			colorBlendAttachment.alphaBlendOp = blendings[i].alphaBlendOp;
		}
		else {
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		}
		colorBlendAttachment.colorWriteMask = { VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT };
		colorBlendAttachments.push_back(colorBlendAttachment);
	}

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
	colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendCreateInfo.pNext = nullptr;
	colorBlendCreateInfo.flags = 0;
	colorBlendCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendCreateInfo.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
	colorBlendCreateInfo.pAttachments = colorBlendAttachments.data();
	colorBlendCreateInfo.blendConstants[0] = 0.0f;
	colorBlendCreateInfo.blendConstants[1] = 0.0f;
	colorBlendCreateInfo.blendConstants[2] = 0.0f;
	colorBlendCreateInfo.blendConstants[3] = 0.0f;

	// Dynamic states
	VkPipelineDynamicStateCreateInfo dynamicCreateInfo = {};
	dynamicCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicCreateInfo.pNext = nullptr;
	dynamicCreateInfo.flags = 0;
	std::array<VkDynamicState, 2> dynamicStates = { VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT };
	dynamicCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicCreateInfo.pDynamicStates = dynamicStates.data();

	// Pipeline
	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.pNext = nullptr;
	graphicsPipelineCreateInfo.flags = 0;
	graphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(pipelineStages.size());
	graphicsPipelineCreateInfo.pStages = pipelineStages.data();
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	graphicsPipelineCreateInfo.pTessellationState = nullptr;
	graphicsPipelineCreateInfo.pViewportState = &viewport->viewportCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = &dynamicCreateInfo;
	graphicsPipelineCreateInfo.layout = pipelineLayout;
	graphicsPipelineCreateInfo.renderPass = renderPass->renderPass;
	graphicsPipelineCreateInfo.subpass = 0;
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex = -1;
	NEIGE_VK_CHECK(vkCreateGraphicsPipelines(logicalDevice.device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline));
}

void GraphicsPipeline::destroy() {
	for (size_t i = 0; i < descriptorPools.size(); i++) {
		descriptorPools[i].destroy();
	}
	for (size_t i = 0; i < descriptorSetLayouts.size(); i++) {
		if (externalSet(i) == -1 && descriptorSetLayouts[i] != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(logicalDevice.device, descriptorSetLayouts[i], nullptr);
			descriptorSetLayouts[i] = VK_NULL_HANDLE;
		}
	}
	destroyPipeline();
}

DescriptorPool* GraphicsPipeline::getDescriptorPool(uint32_t setsToAllocate) {
	for (size_t i = 0; i < descriptorPools.size(); i++) {
		if (descriptorPools[i].descriptorPool != VK_NULL_HANDLE && descriptorPools[i].remainingSets >= setsToAllocate) {
			descriptorPools[i].remainingSets -= setsToAllocate;
			return &descriptorPools[i];
		}
	}

	// Create new descriptor pool
	DescriptorPool descriptorPool;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = nullptr;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.maxSets = 8192;
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
	NEIGE_VK_CHECK(vkCreateDescriptorPool(logicalDevice.device, &descriptorPoolCreateInfo, nullptr, &descriptorPool.descriptorPool));

	descriptorPool.remainingSets = 8192 - setsToAllocate;

	descriptorPools.push_back(descriptorPool);

	return &descriptorPools[descriptorPools.size() - 1];
}

void GraphicsPipeline::bind(CommandBuffer* commandBuffer) {
	vkCmdBindPipeline(commandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	viewport->setViewport(commandBuffer);
	viewport->setScissor(commandBuffer);
}

void GraphicsPipeline::pushConstant(CommandBuffer* commandBuffer, VkShaderStageFlags stages, uint32_t offset, uint32_t size, const void* data) {
	vkCmdPushConstants(commandBuffer->commandBuffer, pipelineLayout, stages, offset, size, data);
}

void GraphicsPipeline::destroyPipeline() {
	if (pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(logicalDevice.device, pipelineLayout, nullptr);
		pipelineLayout = VK_NULL_HANDLE;
	}
	if (pipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(logicalDevice.device, pipeline, nullptr);
		pipeline = VK_NULL_HANDLE;
	}
}

VkPrimitiveTopology GraphicsPipeline::topologyToVkTopology() {
	switch (topology) {
	case Topology::TRIANGLE_LIST:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case Topology::TRIANGLE_STRIP:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	case Topology::LINE_LIST:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case Topology::LINE_STRIP:
		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case Topology::POINT_LIST:
		return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case Topology::WIREFRAME:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	default:
		NEIGE_ERROR("Unsupported primitive topology");
	}
}

VkCompareOp GraphicsPipeline::compareToVkCompare() {
	switch (depthCompare) {
	case Compare::LESS_OR_EQUAL:
		return VK_COMPARE_OP_LESS_OR_EQUAL;
	case Compare::LESS:
		return VK_COMPARE_OP_LESS;
	case Compare::EQUAL:
		return VK_COMPARE_OP_EQUAL;
	default:
		NEIGE_ERROR("Unsupported depth compare operator");
	}
}

int GraphicsPipeline::externalSet(size_t index) {
	for (size_t i = 0; i < externalSets.size(); i++) {
		if (externalSets[i] == index) {
			return static_cast<int>(i);
		}
	}

	return -1;
}
