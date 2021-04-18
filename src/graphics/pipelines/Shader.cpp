#include "Shader.h"
#include "../resources/RendererResources.h"

void Shader::init(const std::string& filePath) {
	file = filePath;
	std::string extension = FileTools::extension(filePath);
	if (extension == "vert") {
		type = ShaderType::VERTEX;
	}
	else if (extension == "frag") {
		type = ShaderType::FRAGMENT;
	}
	else if (extension == "tesc") {
		type = ShaderType::TESSELATION_CONTROL;
	}
	else if (extension == "tese") {
		type = ShaderType::TESSELATION_EVALUATION;
	}
	else if (extension == "geom") {
		type = ShaderType::GEOMETRY;
	}
	else if (extension == "comp") {
		type = ShaderType::COMPUTE;
	}
	else {
		NEIGE_ERROR("\"." + extension + "\" shader extension not supported.");
	}

	if (!glslInitialized) {
		glslang::InitializeProcess();
		glslInitialized = true;
	}

	// Compilation to SPIR-V
	compile();

	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext = nullptr;
	shaderModuleCreateInfo.flags = 0;
	shaderModuleCreateInfo.codeSize = spvCode.size() * sizeof(uint32_t);
	shaderModuleCreateInfo.pCode = spvCode.data();
	NEIGE_VK_CHECK(vkCreateShaderModule(logicalDevice.device, &shaderModuleCreateInfo, nullptr, &module));

	// Reflection from SPIR-V
	reflect();
}

void Shader::destroy() {
	sets.clear();
	sets.shrink_to_fit();
	layoutBindingsShaderTypes.clear();
	layoutBindingsShaderTypes.shrink_to_fit();
	vkDestroyShaderModule(logicalDevice.device, module, nullptr);
}

bool Shader::compile() {
	spvCode.clear();
	spvCode.shrink_to_fit();

	std::string code = FileTools::readAscii(file);

	// Parse specialization constants
	size_t specializationPos = code.find("constant_id");
	while (specializationPos != std::string::npos) {
		SpecializationConstant specializationConstant = {};

		// Get constant_id
		size_t closingParenthesisPos = code.find_first_of(')', specializationPos + 1);
		if (closingParenthesisPos != std::string::npos) {
			size_t equalPos = code.find_first_of('=', specializationPos + 1);
			if (equalPos != std::string::npos) {
				std::string constantId = code.substr(equalPos + 1, closingParenthesisPos - equalPos - 1);
				specializationConstant.id = std::stoi(constantId);
			}
		}

		// Get type and name
		size_t constPos = code.find("const", specializationPos + 1);
		if (constPos != std::string::npos) {
			size_t spacePos = code.find_first_of(' ', constPos + 1);
			if (spacePos != std::string::npos) {
				size_t equalPos = code.find_first_of('=', spacePos + 1);
				if (equalPos != std::string::npos) {
					std::string typeAndName = code.substr(spacePos + 1, equalPos - spacePos - 2);
					spacePos = typeAndName.find_first_of(' ');
					if (spacePos != std::string::npos) {
						specializationConstant.type = typeAndName.substr(0, spacePos);
						specializationConstant.name = typeAndName.substr(spacePos + 1, typeAndName.length() - spacePos - 1);
					}
				}
			}
		}

		specializationConstants.push_back(specializationConstant);

		specializationPos = code.find("constant_id", specializationPos + 1);
	}

	const char* codeString = code.c_str();
	EShLanguage shaderType = shaderTypeToGlslangShaderType();

	glslang::TShader shader(shaderType);
	shader.setStrings(&codeString, 1);
	int clientInputSemanticsVersion = 120;
	glslang::EShTargetClientVersion vulkanClientVersion = glslang::EShTargetVulkan_1_2;
	glslang::EShTargetLanguageVersion languageVersion = glslang::EShTargetSpv_1_5;
	shader.setEnvInput(glslang::EShSourceGlsl, shaderType, glslang::EShClientVulkan, clientInputSemanticsVersion);
	shader.setEnvClient(glslang::EShClientVulkan, vulkanClientVersion);
	shader.setEnvTarget(glslang::EShTargetSpv, languageVersion);
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
	int defaultVersion = 460;

	// Preprocess
	DirStackFileIncluder includer;
	includer.pushExternalLocalDirectory(file);
	std::string preprocess;
	if (!shader.preprocess(&defaultTBuiltInResource, defaultVersion, ENoProfile, false, false, messages, &preprocess, includer)) {
		NEIGE_SHADER_ERROR("\"" + file + "\" shader preprocessing failed.\n" + "\"" + shader.getInfoLog() + "\n" + shader.getInfoDebugLog());
		if (module != VK_NULL_HANDLE) {
			return false;
		}
		NEIGE_ERROR("Shader preprocess error.");
	}

	// Parse
	const char* preprocessString = preprocess.c_str();
	shader.setStrings(&preprocessString, 1);
	if (!shader.parse(&defaultTBuiltInResource, defaultVersion, false, messages)) {
		NEIGE_SHADER_ERROR("\"" + file + "\" shader parsing failed.\n" + "\"" + shader.getInfoLog() + "\n" + shader.getInfoDebugLog());
		if (module != VK_NULL_HANDLE) {
			return false;
		}
		NEIGE_ERROR("Shader parsing error.");
	}

	// Link
	glslang::TProgram program;
	program.addShader(&shader);
	if (!program.link(messages)) {
		NEIGE_SHADER_ERROR("\"" + file + "\" shader linking failed.\n" + "\"" + shader.getInfoLog() + "\n" + shader.getInfoDebugLog());
		if (module != VK_NULL_HANDLE) {
			return false;
		}
		NEIGE_ERROR("Shader linking error.");
	}

	// Compile
	spv::SpvBuildLogger buildLogger;
	glslang::SpvOptions spvOptions;
	glslang::GlslangToSpv(*program.getIntermediate(shaderType), spvCode, &buildLogger, &spvOptions);

	return true;
}

void Shader::reflect() {
	inputVariables.clear();
	inputVariables.shrink_to_fit();
	pushConstantRanges.clear();
	pushConstantRanges.shrink_to_fit();
	uniqueDescriptorTypes.clear();

	SpvReflectShaderModule spvShaderModule;
	SpvReflectResult result = spvReflectCreateShaderModule(spvCode.size() * sizeof(uint32_t), spvCode.data(), &spvShaderModule);
	NEIGE_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "\"" + file + "\" shader reflection failed.");

	if (type == ShaderType::VERTEX) {
		uint32_t inputVariablesCount;
		result = spvReflectEnumerateInputVariables(&spvShaderModule, &inputVariablesCount, nullptr);
		NEIGE_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "\"" + file + "\" : unable to count input variables.");
		std::vector<SpvReflectInterfaceVariable*> reflectedInputVariables(inputVariablesCount);
		result = spvReflectEnumerateInputVariables(&spvShaderModule, &inputVariablesCount, reflectedInputVariables.data());
		NEIGE_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "\"" + file + "\" : unable to find input variables.");
		
		for (uint32_t i = 0; i < inputVariablesCount; i++) {
			InputVariable inputVariable;
			inputVariable.location = reflectedInputVariables[i]->location;
			inputVariable.name = reflectedInputVariables[i]->name;

			inputVariables.push_back(inputVariable);
		}
	}

	uint32_t descriptorSetsCount;
	result = spvReflectEnumerateDescriptorSets(&spvShaderModule, &descriptorSetsCount, nullptr);
	NEIGE_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "\"" + file + "\" : unable to count descriptors sets.");
	std::vector<SpvReflectDescriptorSet*> descriptorSets(descriptorSetsCount);
	result = spvReflectEnumerateDescriptorSets(&spvShaderModule, &descriptorSetsCount, descriptorSets.data());
	NEIGE_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "\"" + file + "\" : unable to find descriptors sets.");

	// Bindings
	for (uint32_t i = 0; i < descriptorSetsCount; i++) {
		const SpvReflectDescriptorSet& reflectedSet = *descriptorSets[i];
		Set set;
		set.set = reflectedSet.set;
		for (uint32_t j = 0; j < reflectedSet.binding_count; j++) {
			const SpvReflectDescriptorBinding& reflectedBinding = *reflectedSet.bindings[j];
			Binding b = {};
			VkDescriptorSetLayoutBinding binding = {};
			std::string name = reflectedBinding.name;
			binding.binding = reflectedBinding.binding;
			binding.descriptorType = static_cast<VkDescriptorType>(reflectedBinding.descriptor_type);
			uniqueDescriptorTypes.insert(binding.descriptorType);
			binding.descriptorCount = 1;
			for (uint32_t k = 0; k < reflectedBinding.array.dims_count; k++) {
				binding.descriptorCount *= reflectedBinding.array.dims[k];
			}
			binding.stageFlags = shaderTypeToVkShaderFlagBits();
			binding.pImmutableSamplers = nullptr;
			b.name = name;
			b.binding = binding;
			set.bindings.push_back(b);
			layoutBindingsShaderTypes.push_back(type);
		}
		sets.push_back(set);
	}
	
	// Push constants
	uint32_t pushConstantsCount;
	result = spvReflectEnumeratePushConstantBlocks(&spvShaderModule, &pushConstantsCount, nullptr);
	NEIGE_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "\"" + file + "\" : unable to count push constants.");
	std::vector<SpvReflectBlockVariable*> pushConstants(pushConstantsCount);
	result = spvReflectEnumeratePushConstantBlocks(&spvShaderModule, &pushConstantsCount, pushConstants.data());
	NEIGE_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "\"" + file + "\" : unable to find push constants.");

	uint32_t pushConstantsOffset = 0;
	for (uint32_t i = 0; i < pushConstantsCount; i++) {
		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = shaderTypeToVkShaderFlagBits();
		pushConstantRange.offset = pushConstantsOffset;
		pushConstantRange.size = pushConstants[i]->size;
		pushConstantRanges.push_back(pushConstantRange);
		pushConstantsOffset += pushConstants[i]->size;
	}

	spvReflectDestroyShaderModule(&spvShaderModule);
}

void Shader::reload() {
	if (compile()) {
		destroy();

		VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.pNext = nullptr;
		shaderModuleCreateInfo.flags = 0;
		shaderModuleCreateInfo.codeSize = spvCode.size() * sizeof(uint32_t);
		shaderModuleCreateInfo.pCode = spvCode.data();
		NEIGE_VK_CHECK(vkCreateShaderModule(logicalDevice.device, &shaderModuleCreateInfo, nullptr, &module));

		reflect();
	}
}

EShLanguage Shader::shaderTypeToGlslangShaderType() {
	switch (type) {
	case ShaderType::VERTEX:
		return EShLangVertex;
		break;
	case ShaderType::FRAGMENT:
		return EShLangFragment;
		break;
	case ShaderType::TESSELATION_CONTROL:
		return EShLangTessControl;
		break;
	case ShaderType::TESSELATION_EVALUATION:
		return EShLangTessEvaluation;
		break;
	case ShaderType::GEOMETRY:
		return EShLangGeometry;
		break;
	case ShaderType::COMPUTE:
		return EShLangCompute;
		break;
	default:
		NEIGE_ERROR("Error converting shader type to glslang shader type.");
	}
}

VkShaderStageFlagBits Shader::shaderTypeToVkShaderFlagBits() {
	switch (type) {
	case ShaderType::VERTEX:
		return VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case ShaderType::FRAGMENT:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	case ShaderType::TESSELATION_CONTROL:
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		break;
	case ShaderType::TESSELATION_EVALUATION:
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		break;
	case ShaderType::GEOMETRY:
		return VK_SHADER_STAGE_GEOMETRY_BIT;
		break;
	case ShaderType::COMPUTE:
		return VK_SHADER_STAGE_COMPUTE_BIT;
		break;
	default:
		NEIGE_ERROR("Error converting shader type to Vulkan shader flag bits.");
	}
}
