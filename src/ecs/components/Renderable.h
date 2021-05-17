#pragma once
#include "../../graphics/resources/Buffer.h"
#include "../../graphics/pipelines/DescriptorSet.h"
#include "../../graphics/pipelines/GraphicsPipeline.h"
#include "../../graphics/resources/Samplers.h"
#include "../../graphics/resources/ShaderResources.h"
#include "../../utils/structs/ShaderStructs.h"
#include <string>
#include <vector>
#include <iostream>

struct Renderable {
	// Component paramaters
	// Model
	std::string modelPath = "";

	// Shaders
	std::string vertexShaderPath = "";
	std::string fragmentShaderPath = "";
	std::string tesselationControlShaderPath = "";
	std::string tesselationEvaluationShaderPath = "";
	std::string geometryShaderPath = "";

	// Model
	Model* model;

	// Pipeline topology
	Topology topology = Topology::TRIANGLE_LIST;

	// Graphics pipelines
	GraphicsPipeline* opaqueGraphicsPipeline;
	GraphicsPipeline* maskGraphicsPipeline;
	GraphicsPipeline* blendGraphicsPipeline;
	std::string lookupString = "";

	void createLookupString() {
		lookupString = vertexShaderPath + fragmentShaderPath + tesselationControlShaderPath + tesselationEvaluationShaderPath + geometryShaderPath + std::to_string(static_cast<int>(topology));
	}

	// Descriptor sets and buffers
	std::vector<Buffer> buffers;
	std::vector<DescriptorSet> descriptorSets;
	std::vector<DescriptorSet> depthPrepassDescriptorSets;
	std::vector<DescriptorSet> depthPrepassMaskDescriptorSets;
	std::vector<DescriptorSet> shadowDescriptorSets;
	std::vector<DescriptorSet> shadowMaskDescriptorSets;

	// Frustum culling
	std::vector<VkBuffer> indirectBuffers;
	std::vector<VkBuffer> drawCountBuffers;
	std::vector<VkBuffer> perDrawBuffers;

	uint32_t opaqueCulledDrawCount = 0;
	Buffer opaqueCulledDrawCountBuffer;
	Buffer opaqueCulledDrawIndirectBuffer;
	Buffer opaqueCulledDrawIndirectInfoBuffer;
	std::vector<DescriptorSet> opaqueFrustumCullingDescriptorSets;
	DescriptorSet opaqueCulledDrawIndirectInfoDescriptorSet;

	uint32_t maskCulledDrawCount = 0;
	Buffer maskCulledDrawCountBuffer;
	Buffer maskCulledDrawIndirectBuffer;
	Buffer maskCulledDrawIndirectInfoBuffer;
	std::vector<DescriptorSet> maskFrustumCullingDescriptorSets;
	DescriptorSet maskCulledDrawIndirectInfoDescriptorSet;

	uint32_t blendCulledDrawCount = 0;
	Buffer blendCulledDrawCountBuffer;
	Buffer blendCulledDrawIndirectBuffer;
	Buffer blendCulledDrawIndirectInfoBuffer;
	std::vector<DescriptorSet> blendFrustumCullingDescriptorSets;
	DescriptorSet blendCulledDrawIndirectInfoDescriptorSet;

	void createEntityDescriptorSet(uint32_t frameInFlightIndex, GraphicsPipeline* graphicsPipeline) {
		descriptorSets[frameInFlightIndex].init(graphicsPipeline, 0);

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkDescriptorBufferInfo objectInfo = {};
		VkDescriptorBufferInfo cameraInfo = {};
		VkDescriptorBufferInfo shadowInfo = {};
		VkDescriptorBufferInfo lightingInfo = {};
		VkDescriptorImageInfo irradianceInfo = {};
		VkDescriptorImageInfo prefilterInfo = {};
		VkDescriptorImageInfo brdfLUTInfo = {};
		std::vector<VkDescriptorImageInfo> shadowMapsInfos;
		VkDescriptorBufferInfo timeInfo = {};

		for (size_t i = 0; i < graphicsPipeline->sets[0].bindings.size(); i++) {
			std::string bindingName = graphicsPipeline->sets[0].bindings[i].name;
			if (bindingName == "object") {
				objectInfo.buffer = buffers.at(frameInFlightIndex).buffer;
				objectInfo.offset = 0;
				objectInfo.range = sizeof(ObjectUniformBufferObject);

				VkWriteDescriptorSet objectWriteDescriptorSet = {};
				objectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				objectWriteDescriptorSet.pNext = nullptr;
				objectWriteDescriptorSet.dstSet = descriptorSets.at(frameInFlightIndex).descriptorSet;
				objectWriteDescriptorSet.dstBinding = graphicsPipeline->sets[0].bindings[i].binding.binding;
				objectWriteDescriptorSet.dstArrayElement = 0;
				objectWriteDescriptorSet.descriptorCount = 1;
				objectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				objectWriteDescriptorSet.pImageInfo = nullptr;
				objectWriteDescriptorSet.pBufferInfo = &objectInfo;
				objectWriteDescriptorSet.pTexelBufferView = nullptr;

				writesDescriptorSet.push_back(objectWriteDescriptorSet);
			}
			else if (bindingName == "camera") {
				cameraInfo.buffer = cameraBuffers.at(frameInFlightIndex).buffer;
				cameraInfo.offset = 0;
				cameraInfo.range = sizeof(CameraUniformBufferObject);

				VkWriteDescriptorSet cameraWriteDescriptorSet = {};
				cameraWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				cameraWriteDescriptorSet.pNext = nullptr;
				cameraWriteDescriptorSet.dstSet = descriptorSets.at(frameInFlightIndex).descriptorSet;
				cameraWriteDescriptorSet.dstBinding = graphicsPipeline->sets[0].bindings[i].binding.binding;
				cameraWriteDescriptorSet.dstArrayElement = 0;
				cameraWriteDescriptorSet.descriptorCount = 1;
				cameraWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				cameraWriteDescriptorSet.pImageInfo = nullptr;
				cameraWriteDescriptorSet.pBufferInfo = &cameraInfo;
				cameraWriteDescriptorSet.pTexelBufferView = nullptr;

				writesDescriptorSet.push_back(cameraWriteDescriptorSet);
			}
			else if (bindingName == "shadow") {
				shadowInfo.buffer = shadow.buffers.at(frameInFlightIndex).buffer;
				shadowInfo.offset = 0;
				shadowInfo.range = sizeof(ShadowUniformBufferObject);

				VkWriteDescriptorSet shadowWriteDescriptorSet = {};
				shadowWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				shadowWriteDescriptorSet.pNext = nullptr;
				shadowWriteDescriptorSet.dstSet = descriptorSets.at(frameInFlightIndex).descriptorSet;
				shadowWriteDescriptorSet.dstBinding = graphicsPipeline->sets[0].bindings[i].binding.binding;
				shadowWriteDescriptorSet.dstArrayElement = 0;
				shadowWriteDescriptorSet.descriptorCount = 1;
				shadowWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				shadowWriteDescriptorSet.pImageInfo = nullptr;
				shadowWriteDescriptorSet.pBufferInfo = &shadowInfo;
				shadowWriteDescriptorSet.pTexelBufferView = nullptr;

				writesDescriptorSet.push_back(shadowWriteDescriptorSet);
			}
			else if (bindingName == "lights") {
				lightingInfo.buffer = lightingBuffers.at(frameInFlightIndex).buffer;
				lightingInfo.offset = 0;
				lightingInfo.range = sizeof(LightingUniformBufferObject);

				VkWriteDescriptorSet lightingWriteDescriptorSet = {};
				lightingWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				lightingWriteDescriptorSet.pNext = nullptr;
				lightingWriteDescriptorSet.dstSet = descriptorSets.at(frameInFlightIndex).descriptorSet;
				lightingWriteDescriptorSet.dstBinding = graphicsPipeline->sets[0].bindings[i].binding.binding;
				lightingWriteDescriptorSet.dstArrayElement = 0;
				lightingWriteDescriptorSet.descriptorCount = 1;
				lightingWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				lightingWriteDescriptorSet.pImageInfo = nullptr;
				lightingWriteDescriptorSet.pBufferInfo = &lightingInfo;
				lightingWriteDescriptorSet.pTexelBufferView = nullptr;

				writesDescriptorSet.push_back(lightingWriteDescriptorSet);
			}
			else if (bindingName == "irradianceMap") {
				irradianceInfo.sampler = trilinearEdgeBlackSampler;
				irradianceInfo.imageView = envmap.diffuseIradianceImage.imageView;
				irradianceInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				VkWriteDescriptorSet irradianceWriteDescriptorSet = {};
				irradianceWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				irradianceWriteDescriptorSet.pNext = nullptr;
				irradianceWriteDescriptorSet.dstSet = descriptorSets.at(frameInFlightIndex).descriptorSet;
				irradianceWriteDescriptorSet.dstBinding = graphicsPipeline->sets[0].bindings[i].binding.binding;
				irradianceWriteDescriptorSet.dstArrayElement = 0;
				irradianceWriteDescriptorSet.descriptorCount = 1;
				irradianceWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				irradianceWriteDescriptorSet.pImageInfo = &irradianceInfo;
				irradianceWriteDescriptorSet.pBufferInfo = nullptr;
				irradianceWriteDescriptorSet.pTexelBufferView = nullptr;

				writesDescriptorSet.push_back(irradianceWriteDescriptorSet);
			}
			else if (bindingName == "prefilterMap") {
				prefilterInfo.sampler = trilinearEdgeBlackSampler;
				prefilterInfo.imageView = envmap.prefilterImage.imageView;
				prefilterInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				VkWriteDescriptorSet prefilterWriteDescriptorSet = {};
				prefilterWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				prefilterWriteDescriptorSet.pNext = nullptr;
				prefilterWriteDescriptorSet.dstSet = descriptorSets.at(frameInFlightIndex).descriptorSet;
				prefilterWriteDescriptorSet.dstBinding = graphicsPipeline->sets[0].bindings[i].binding.binding;
				prefilterWriteDescriptorSet.dstArrayElement = 0;
				prefilterWriteDescriptorSet.descriptorCount = 1;
				prefilterWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				prefilterWriteDescriptorSet.pImageInfo = &prefilterInfo;
				prefilterWriteDescriptorSet.pBufferInfo = nullptr;
				prefilterWriteDescriptorSet.pTexelBufferView = nullptr;

				writesDescriptorSet.push_back(prefilterWriteDescriptorSet);
			}
			else if (bindingName == "brdfLUT") {
				brdfLUTInfo.sampler = trilinearEdgeBlackSampler;
				brdfLUTInfo.imageView = envmap.brdfConvolutionImage.imageView;
				brdfLUTInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				VkWriteDescriptorSet brdfLUTWriteDescriptorSet = {};
				brdfLUTWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				brdfLUTWriteDescriptorSet.pNext = nullptr;
				brdfLUTWriteDescriptorSet.dstSet = descriptorSets.at(frameInFlightIndex).descriptorSet;
				brdfLUTWriteDescriptorSet.dstBinding = graphicsPipeline->sets[0].bindings[i].binding.binding;
				brdfLUTWriteDescriptorSet.dstArrayElement = 0;
				brdfLUTWriteDescriptorSet.descriptorCount = 1;
				brdfLUTWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				brdfLUTWriteDescriptorSet.pImageInfo = &brdfLUTInfo;
				brdfLUTWriteDescriptorSet.pBufferInfo = nullptr;
				brdfLUTWriteDescriptorSet.pTexelBufferView = nullptr;

				writesDescriptorSet.push_back(brdfLUTWriteDescriptorSet);
			}
			else if (bindingName == "shadowMaps") {
				shadowMapsInfos.resize(MAX_DIR_LIGHTS + MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS);

				for (int j = 0; j < MAX_DIR_LIGHTS + MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS; j++) {
					shadowMapsInfos[j].sampler = trilinearBorderWhiteLessSampler;
					shadowMapsInfos[j].imageView = (j < shadow.mapCount) ? shadow.images[j].imageView : shadow.defaultShadow.imageView;
					shadowMapsInfos[j].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				}

				VkWriteDescriptorSet shadowMapsWriteDescriptorSet = {};
				shadowMapsWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				shadowMapsWriteDescriptorSet.pNext = nullptr;
				shadowMapsWriteDescriptorSet.dstSet = descriptorSets.at(frameInFlightIndex).descriptorSet;
				shadowMapsWriteDescriptorSet.dstBinding = graphicsPipeline->sets[0].bindings[i].binding.binding;
				shadowMapsWriteDescriptorSet.dstArrayElement = 0;
				shadowMapsWriteDescriptorSet.descriptorCount = static_cast<uint32_t>(shadowMapsInfos.size());
				shadowMapsWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				shadowMapsWriteDescriptorSet.pImageInfo = shadowMapsInfos.data();
				shadowMapsWriteDescriptorSet.pBufferInfo = nullptr;
				shadowMapsWriteDescriptorSet.pTexelBufferView = nullptr;

				writesDescriptorSet.push_back(shadowMapsWriteDescriptorSet);
			}
			else if (bindingName == "time") {
				timeInfo.buffer = timeBuffers.at(frameInFlightIndex).buffer;
				timeInfo.offset = 0;
				timeInfo.range = sizeof(float);

				VkWriteDescriptorSet timeWriteDescriptorSet = {};
				timeWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				timeWriteDescriptorSet.pNext = nullptr;
				timeWriteDescriptorSet.dstSet = descriptorSets.at(frameInFlightIndex).descriptorSet;
				timeWriteDescriptorSet.dstBinding = graphicsPipeline->sets[0].bindings[i].binding.binding;
				timeWriteDescriptorSet.dstArrayElement = 0;
				timeWriteDescriptorSet.descriptorCount = 1;
				timeWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				timeWriteDescriptorSet.pImageInfo = nullptr;
				timeWriteDescriptorSet.pBufferInfo = &timeInfo;
				timeWriteDescriptorSet.pTexelBufferView = nullptr;

				writesDescriptorSet.push_back(timeWriteDescriptorSet);
			}
		}
		descriptorSets.at(frameInFlightIndex).update(writesDescriptorSet);
	}

	void createOpaqueFrustumCullingEntityDescriptorSet(uint32_t frameInFlightIndex) {
		opaqueFrustumCullingDescriptorSets[frameInFlightIndex].init(&frustumCulling.computePipeline, 0);

		VkDescriptorBufferInfo objectInfo = {};
		objectInfo.buffer = buffers.at(frameInFlightIndex).buffer;
		objectInfo.offset = 0;
		objectInfo.range = sizeof(ObjectUniformBufferObject);

		VkDescriptorBufferInfo inFrustumInfo = {};
		inFrustumInfo.buffer = frustumBuffers.at(frameInFlightIndex).buffer;
		inFrustumInfo.offset = 0;
		inFrustumInfo.range = 6 * 4 * sizeof(float);

		VkDescriptorBufferInfo outDrawIndirectInfo;
		outDrawIndirectInfo.buffer = opaqueCulledDrawIndirectBuffer.buffer;
		outDrawIndirectInfo.offset = 0;
		outDrawIndirectInfo.range = model->opaqueDrawCount * sizeof(VkDrawIndexedIndirectCommand);

		VkDescriptorBufferInfo outPerDrawInfo = {};
		outPerDrawInfo.buffer = opaqueCulledDrawIndirectInfoBuffer.buffer;
		outPerDrawInfo.offset = 0;
		outPerDrawInfo.range = model->opaqueDrawCount * sizeof(PerDraw);

		VkDescriptorBufferInfo outDrawCountInfo = {};
		outDrawCountInfo.buffer = opaqueCulledDrawCountBuffer.buffer;
		outDrawCountInfo.offset = 0;
		outDrawCountInfo.range = sizeof(uint32_t);

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet objectWriteDescriptorSet = {};
		objectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		objectWriteDescriptorSet.pNext = nullptr;
		objectWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		objectWriteDescriptorSet.dstBinding = 0;
		objectWriteDescriptorSet.dstArrayElement = 0;
		objectWriteDescriptorSet.descriptorCount = 1;
		objectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		objectWriteDescriptorSet.pImageInfo = nullptr;
		objectWriteDescriptorSet.pBufferInfo = &objectInfo;
		objectWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(objectWriteDescriptorSet);

		VkWriteDescriptorSet inFrustumWriteDescriptorSet = {};
		inFrustumWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		inFrustumWriteDescriptorSet.pNext = nullptr;
		inFrustumWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		inFrustumWriteDescriptorSet.dstBinding = 1;
		inFrustumWriteDescriptorSet.dstArrayElement = 0;
		inFrustumWriteDescriptorSet.descriptorCount = 1;
		inFrustumWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		inFrustumWriteDescriptorSet.pImageInfo = nullptr;
		inFrustumWriteDescriptorSet.pBufferInfo = &inFrustumInfo;
		inFrustumWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(inFrustumWriteDescriptorSet);

		VkWriteDescriptorSet outDrawIndirectWriteDescriptorSet = {};
		outDrawIndirectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outDrawIndirectWriteDescriptorSet.pNext = nullptr;
		outDrawIndirectWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		outDrawIndirectWriteDescriptorSet.dstBinding = 2;
		outDrawIndirectWriteDescriptorSet.dstArrayElement = 0;
		outDrawIndirectWriteDescriptorSet.descriptorCount = 1;
		outDrawIndirectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		outDrawIndirectWriteDescriptorSet.pImageInfo = nullptr;
		outDrawIndirectWriteDescriptorSet.pBufferInfo = &outDrawIndirectInfo;
		outDrawIndirectWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(outDrawIndirectWriteDescriptorSet);

		VkWriteDescriptorSet outPerDrawWriteDescriptorSet = {};
		outPerDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outPerDrawWriteDescriptorSet.pNext = nullptr;
		outPerDrawWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		outPerDrawWriteDescriptorSet.dstBinding = 3;
		outPerDrawWriteDescriptorSet.dstArrayElement = 0;
		outPerDrawWriteDescriptorSet.descriptorCount = 1;
		outPerDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		outPerDrawWriteDescriptorSet.pImageInfo = nullptr;
		outPerDrawWriteDescriptorSet.pBufferInfo = &outPerDrawInfo;
		outPerDrawWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(outPerDrawWriteDescriptorSet);

		VkWriteDescriptorSet outDrawCountWriteDescriptorSet = {};
		outDrawCountWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outDrawCountWriteDescriptorSet.pNext = nullptr;
		outDrawCountWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		outDrawCountWriteDescriptorSet.dstBinding = 4;
		outDrawCountWriteDescriptorSet.dstArrayElement = 0;
		outDrawCountWriteDescriptorSet.descriptorCount = 1;
		outDrawCountWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		outDrawCountWriteDescriptorSet.pImageInfo = nullptr;
		outDrawCountWriteDescriptorSet.pBufferInfo = &outDrawCountInfo;
		outDrawCountWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(outDrawCountWriteDescriptorSet);

		opaqueFrustumCullingDescriptorSets[frameInFlightIndex].update(writesDescriptorSet);
	}

	void createMaskFrustumCullingEntityDescriptorSet(uint32_t frameInFlightIndex) {
		maskFrustumCullingDescriptorSets[frameInFlightIndex].init(&frustumCulling.computePipeline, 0);

		VkDescriptorBufferInfo objectInfo = {};
		objectInfo.buffer = buffers.at(frameInFlightIndex).buffer;
		objectInfo.offset = 0;
		objectInfo.range = sizeof(ObjectUniformBufferObject);

		VkDescriptorBufferInfo inFrustumInfo = {};
		inFrustumInfo.buffer = frustumBuffers.at(frameInFlightIndex).buffer;
		inFrustumInfo.offset = 0;
		inFrustumInfo.range = 6 * 4 * sizeof(float);

		VkDescriptorBufferInfo outDrawIndirectInfo;
		outDrawIndirectInfo.buffer = maskCulledDrawIndirectBuffer.buffer;
		outDrawIndirectInfo.offset = 0;
		outDrawIndirectInfo.range = model->maskDrawCount * sizeof(VkDrawIndexedIndirectCommand);

		VkDescriptorBufferInfo outPerDrawInfo = {};
		outPerDrawInfo.buffer = maskCulledDrawIndirectInfoBuffer.buffer;
		outPerDrawInfo.offset = 0;
		outPerDrawInfo.range = model->maskDrawCount * sizeof(PerDraw);

		VkDescriptorBufferInfo outDrawCountInfo = {};
		outDrawCountInfo.buffer = maskCulledDrawCountBuffer.buffer;
		outDrawCountInfo.offset = 0;
		outDrawCountInfo.range = sizeof(uint32_t);

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet objectWriteDescriptorSet = {};
		objectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		objectWriteDescriptorSet.pNext = nullptr;
		objectWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		objectWriteDescriptorSet.dstBinding = 0;
		objectWriteDescriptorSet.dstArrayElement = 0;
		objectWriteDescriptorSet.descriptorCount = 1;
		objectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		objectWriteDescriptorSet.pImageInfo = nullptr;
		objectWriteDescriptorSet.pBufferInfo = &objectInfo;
		objectWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(objectWriteDescriptorSet);

		VkWriteDescriptorSet inFrustumWriteDescriptorSet = {};
		inFrustumWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		inFrustumWriteDescriptorSet.pNext = nullptr;
		inFrustumWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		inFrustumWriteDescriptorSet.dstBinding = 1;
		inFrustumWriteDescriptorSet.dstArrayElement = 0;
		inFrustumWriteDescriptorSet.descriptorCount = 1;
		inFrustumWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		inFrustumWriteDescriptorSet.pImageInfo = nullptr;
		inFrustumWriteDescriptorSet.pBufferInfo = &inFrustumInfo;
		inFrustumWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(inFrustumWriteDescriptorSet);

		VkWriteDescriptorSet outDrawIndirectWriteDescriptorSet = {};
		outDrawIndirectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outDrawIndirectWriteDescriptorSet.pNext = nullptr;
		outDrawIndirectWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		outDrawIndirectWriteDescriptorSet.dstBinding = 2;
		outDrawIndirectWriteDescriptorSet.dstArrayElement = 0;
		outDrawIndirectWriteDescriptorSet.descriptorCount = 1;
		outDrawIndirectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		outDrawIndirectWriteDescriptorSet.pImageInfo = nullptr;
		outDrawIndirectWriteDescriptorSet.pBufferInfo = &outDrawIndirectInfo;
		outDrawIndirectWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(outDrawIndirectWriteDescriptorSet);

		VkWriteDescriptorSet outPerDrawWriteDescriptorSet = {};
		outPerDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outPerDrawWriteDescriptorSet.pNext = nullptr;
		outPerDrawWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		outPerDrawWriteDescriptorSet.dstBinding = 3;
		outPerDrawWriteDescriptorSet.dstArrayElement = 0;
		outPerDrawWriteDescriptorSet.descriptorCount = 1;
		outPerDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		outPerDrawWriteDescriptorSet.pImageInfo = nullptr;
		outPerDrawWriteDescriptorSet.pBufferInfo = &outPerDrawInfo;
		outPerDrawWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(outPerDrawWriteDescriptorSet);

		VkWriteDescriptorSet outDrawCountWriteDescriptorSet = {};
		outDrawCountWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outDrawCountWriteDescriptorSet.pNext = nullptr;
		outDrawCountWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		outDrawCountWriteDescriptorSet.dstBinding = 4;
		outDrawCountWriteDescriptorSet.dstArrayElement = 0;
		outDrawCountWriteDescriptorSet.descriptorCount = 1;
		outDrawCountWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		outDrawCountWriteDescriptorSet.pImageInfo = nullptr;
		outDrawCountWriteDescriptorSet.pBufferInfo = &outDrawCountInfo;
		outDrawCountWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(outDrawCountWriteDescriptorSet);

		maskFrustumCullingDescriptorSets[frameInFlightIndex].update(writesDescriptorSet);
	}

	void createBlendFrustumCullingEntityDescriptorSet(uint32_t frameInFlightIndex) {
		blendFrustumCullingDescriptorSets[frameInFlightIndex].init(&frustumCulling.computePipeline, 0);

		VkDescriptorBufferInfo objectInfo = {};
		objectInfo.buffer = buffers.at(frameInFlightIndex).buffer;
		objectInfo.offset = 0;
		objectInfo.range = sizeof(ObjectUniformBufferObject);

		VkDescriptorBufferInfo inFrustumInfo = {};
		inFrustumInfo.buffer = frustumBuffers.at(frameInFlightIndex).buffer;
		inFrustumInfo.offset = 0;
		inFrustumInfo.range = 6 * 4 * sizeof(float);

		VkDescriptorBufferInfo outDrawIndirectInfo;
		outDrawIndirectInfo.buffer = blendCulledDrawIndirectBuffer.buffer;
		outDrawIndirectInfo.offset = 0;
		outDrawIndirectInfo.range = model->blendDrawCount * sizeof(VkDrawIndexedIndirectCommand);

		VkDescriptorBufferInfo outPerDrawInfo = {};
		outPerDrawInfo.buffer = blendCulledDrawIndirectInfoBuffer.buffer;
		outPerDrawInfo.offset = 0;
		outPerDrawInfo.range = model->blendDrawCount * sizeof(PerDraw);

		VkDescriptorBufferInfo outDrawCountInfo = {};
		outDrawCountInfo.buffer = blendCulledDrawCountBuffer.buffer;
		outDrawCountInfo.offset = 0;
		outDrawCountInfo.range = sizeof(uint32_t);

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet objectWriteDescriptorSet = {};
		objectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		objectWriteDescriptorSet.pNext = nullptr;
		objectWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		objectWriteDescriptorSet.dstBinding = 0;
		objectWriteDescriptorSet.dstArrayElement = 0;
		objectWriteDescriptorSet.descriptorCount = 1;
		objectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		objectWriteDescriptorSet.pImageInfo = nullptr;
		objectWriteDescriptorSet.pBufferInfo = &objectInfo;
		objectWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(objectWriteDescriptorSet);

		VkWriteDescriptorSet inFrustumWriteDescriptorSet = {};
		inFrustumWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		inFrustumWriteDescriptorSet.pNext = nullptr;
		inFrustumWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		inFrustumWriteDescriptorSet.dstBinding = 1;
		inFrustumWriteDescriptorSet.dstArrayElement = 0;
		inFrustumWriteDescriptorSet.descriptorCount = 1;
		inFrustumWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		inFrustumWriteDescriptorSet.pImageInfo = nullptr;
		inFrustumWriteDescriptorSet.pBufferInfo = &inFrustumInfo;
		inFrustumWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(inFrustumWriteDescriptorSet);

		VkWriteDescriptorSet outDrawIndirectWriteDescriptorSet = {};
		outDrawIndirectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outDrawIndirectWriteDescriptorSet.pNext = nullptr;
		outDrawIndirectWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		outDrawIndirectWriteDescriptorSet.dstBinding = 2;
		outDrawIndirectWriteDescriptorSet.dstArrayElement = 0;
		outDrawIndirectWriteDescriptorSet.descriptorCount = 1;
		outDrawIndirectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		outDrawIndirectWriteDescriptorSet.pImageInfo = nullptr;
		outDrawIndirectWriteDescriptorSet.pBufferInfo = &outDrawIndirectInfo;
		outDrawIndirectWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(outDrawIndirectWriteDescriptorSet);

		VkWriteDescriptorSet outPerDrawWriteDescriptorSet = {};
		outPerDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outPerDrawWriteDescriptorSet.pNext = nullptr;
		outPerDrawWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		outPerDrawWriteDescriptorSet.dstBinding = 3;
		outPerDrawWriteDescriptorSet.dstArrayElement = 0;
		outPerDrawWriteDescriptorSet.descriptorCount = 1;
		outPerDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		outPerDrawWriteDescriptorSet.pImageInfo = nullptr;
		outPerDrawWriteDescriptorSet.pBufferInfo = &outPerDrawInfo;
		outPerDrawWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(outPerDrawWriteDescriptorSet);

		VkWriteDescriptorSet outDrawCountWriteDescriptorSet = {};
		outDrawCountWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outDrawCountWriteDescriptorSet.pNext = nullptr;
		outDrawCountWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSets[frameInFlightIndex].descriptorSet;
		outDrawCountWriteDescriptorSet.dstBinding = 4;
		outDrawCountWriteDescriptorSet.dstArrayElement = 0;
		outDrawCountWriteDescriptorSet.descriptorCount = 1;
		outDrawCountWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		outDrawCountWriteDescriptorSet.pImageInfo = nullptr;
		outDrawCountWriteDescriptorSet.pBufferInfo = &outDrawCountInfo;
		outDrawCountWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(outDrawCountWriteDescriptorSet);

		blendFrustumCullingDescriptorSets[frameInFlightIndex].update(writesDescriptorSet);
	}

	void createDepthPrepassEntityDescriptorSet(uint32_t frameInFlightIndex) {
		depthPrepassDescriptorSets[frameInFlightIndex].init(&depthPrepass.opaqueGraphicsPipeline, 0);

		VkDescriptorBufferInfo objectInfo = {};
		objectInfo.buffer = buffers.at(frameInFlightIndex).buffer;
		objectInfo.offset = 0;
		objectInfo.range = sizeof(ObjectUniformBufferObject);

		VkDescriptorBufferInfo cameraInfo = {};
		cameraInfo.buffer = cameraBuffers.at(frameInFlightIndex).buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(CameraUniformBufferObject);

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet objectWriteDescriptorSet = {};
		objectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		objectWriteDescriptorSet.pNext = nullptr;
		objectWriteDescriptorSet.dstSet = depthPrepassDescriptorSets[frameInFlightIndex].descriptorSet;
		objectWriteDescriptorSet.dstBinding = 0;
		objectWriteDescriptorSet.dstArrayElement = 0;
		objectWriteDescriptorSet.descriptorCount = 1;
		objectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		objectWriteDescriptorSet.pImageInfo = nullptr;
		objectWriteDescriptorSet.pBufferInfo = &objectInfo;
		objectWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(objectWriteDescriptorSet);

		VkWriteDescriptorSet cameraWriteDescriptorSet = {};
		cameraWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraWriteDescriptorSet.pNext = nullptr;
		cameraWriteDescriptorSet.dstSet = depthPrepassDescriptorSets[frameInFlightIndex].descriptorSet;
		cameraWriteDescriptorSet.dstBinding = 1;
		cameraWriteDescriptorSet.dstArrayElement = 0;
		cameraWriteDescriptorSet.descriptorCount = 1;
		cameraWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraWriteDescriptorSet.pImageInfo = nullptr;
		cameraWriteDescriptorSet.pBufferInfo = &cameraInfo;
		cameraWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(cameraWriteDescriptorSet);

		depthPrepassDescriptorSets[frameInFlightIndex].update(writesDescriptorSet);
	}

	void createDepthPrepassMaskEntityDescriptorSet(uint32_t frameInFlightIndex) {
		depthPrepassMaskDescriptorSets[frameInFlightIndex].init(&depthPrepass.maskGraphicsPipeline, 0);

		VkDescriptorBufferInfo objectInfo = {};
		objectInfo.buffer = buffers.at(frameInFlightIndex).buffer;
		objectInfo.offset = 0;
		objectInfo.range = sizeof(ObjectUniformBufferObject);

		VkDescriptorBufferInfo cameraInfo = {};
		cameraInfo.buffer = cameraBuffers.at(frameInFlightIndex).buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(CameraUniformBufferObject);

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet objectWriteDescriptorSet = {};
		objectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		objectWriteDescriptorSet.pNext = nullptr;
		objectWriteDescriptorSet.dstSet = depthPrepassMaskDescriptorSets[frameInFlightIndex].descriptorSet;
		objectWriteDescriptorSet.dstBinding = 0;
		objectWriteDescriptorSet.dstArrayElement = 0;
		objectWriteDescriptorSet.descriptorCount = 1;
		objectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		objectWriteDescriptorSet.pImageInfo = nullptr;
		objectWriteDescriptorSet.pBufferInfo = &objectInfo;
		objectWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(objectWriteDescriptorSet);

		VkWriteDescriptorSet cameraWriteDescriptorSet = {};
		cameraWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraWriteDescriptorSet.pNext = nullptr;
		cameraWriteDescriptorSet.dstSet = depthPrepassMaskDescriptorSets[frameInFlightIndex].descriptorSet;
		cameraWriteDescriptorSet.dstBinding = 1;
		cameraWriteDescriptorSet.dstArrayElement = 0;
		cameraWriteDescriptorSet.descriptorCount = 1;
		cameraWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraWriteDescriptorSet.pImageInfo = nullptr;
		cameraWriteDescriptorSet.pBufferInfo = &cameraInfo;
		cameraWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(cameraWriteDescriptorSet);

		depthPrepassMaskDescriptorSets[frameInFlightIndex].update(writesDescriptorSet);
	}

	void createShadowEntityDescriptorSet(uint32_t frameInFlightIndex) {
		shadowDescriptorSets[frameInFlightIndex].init(&shadow.opaqueGraphicsPipeline, 0);

		VkDescriptorBufferInfo objectInfo = {};
		objectInfo.buffer = buffers.at(frameInFlightIndex).buffer;
		objectInfo.offset = 0;
		objectInfo.range = sizeof(ObjectUniformBufferObject);

		VkDescriptorBufferInfo shadowInfo = {};
		shadowInfo.buffer = shadow.buffers.at(frameInFlightIndex).buffer;
		shadowInfo.offset = 0;
		shadowInfo.range = sizeof(ShadowUniformBufferObject);

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet objectWriteDescriptorSet = {};
		objectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		objectWriteDescriptorSet.pNext = nullptr;
		objectWriteDescriptorSet.dstSet = shadowDescriptorSets[frameInFlightIndex].descriptorSet;
		objectWriteDescriptorSet.dstBinding = 0;
		objectWriteDescriptorSet.dstArrayElement = 0;
		objectWriteDescriptorSet.descriptorCount = 1;
		objectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		objectWriteDescriptorSet.pImageInfo = nullptr;
		objectWriteDescriptorSet.pBufferInfo = &objectInfo;
		objectWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(objectWriteDescriptorSet);

		VkWriteDescriptorSet shadowWriteDescriptorSet = {};
		shadowWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		shadowWriteDescriptorSet.pNext = nullptr;
		shadowWriteDescriptorSet.dstSet = shadowDescriptorSets[frameInFlightIndex].descriptorSet;
		shadowWriteDescriptorSet.dstBinding = 1;
		shadowWriteDescriptorSet.dstArrayElement = 0;
		shadowWriteDescriptorSet.descriptorCount = 1;
		shadowWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		shadowWriteDescriptorSet.pImageInfo = nullptr;
		shadowWriteDescriptorSet.pBufferInfo = &shadowInfo;
		shadowWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(shadowWriteDescriptorSet);

		shadowDescriptorSets[frameInFlightIndex].update(writesDescriptorSet);
	}

	void createShadowMaskEntityDescriptorSet(uint32_t frameInFlightIndex) {
		shadowMaskDescriptorSets[frameInFlightIndex].init(&shadow.maskGraphicsPipeline, 0);

		VkDescriptorBufferInfo objectInfo = {};
		objectInfo.buffer = buffers.at(frameInFlightIndex).buffer;
		objectInfo.offset = 0;
		objectInfo.range = sizeof(ObjectUniformBufferObject);

		VkDescriptorBufferInfo shadowInfo = {};
		shadowInfo.buffer = shadow.buffers.at(frameInFlightIndex).buffer;
		shadowInfo.offset = 0;
		shadowInfo.range = sizeof(ShadowUniformBufferObject);

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

		VkWriteDescriptorSet objectWriteDescriptorSet = {};
		objectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		objectWriteDescriptorSet.pNext = nullptr;
		objectWriteDescriptorSet.dstSet = shadowMaskDescriptorSets[frameInFlightIndex].descriptorSet;
		objectWriteDescriptorSet.dstBinding = 0;
		objectWriteDescriptorSet.dstArrayElement = 0;
		objectWriteDescriptorSet.descriptorCount = 1;
		objectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		objectWriteDescriptorSet.pImageInfo = nullptr;
		objectWriteDescriptorSet.pBufferInfo = &objectInfo;
		objectWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(objectWriteDescriptorSet);

		VkWriteDescriptorSet shadowWriteDescriptorSet = {};
		shadowWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		shadowWriteDescriptorSet.pNext = nullptr;
		shadowWriteDescriptorSet.dstSet = shadowMaskDescriptorSets[frameInFlightIndex].descriptorSet;
		shadowWriteDescriptorSet.dstBinding = 1;
		shadowWriteDescriptorSet.dstArrayElement = 0;
		shadowWriteDescriptorSet.descriptorCount = 1;
		shadowWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		shadowWriteDescriptorSet.pImageInfo = nullptr;
		shadowWriteDescriptorSet.pBufferInfo = &shadowInfo;
		shadowWriteDescriptorSet.pTexelBufferView = nullptr;
		writesDescriptorSet.push_back(shadowWriteDescriptorSet);

		shadowMaskDescriptorSets[frameInFlightIndex].update(writesDescriptorSet);
	}

	void cullOpaque(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex) {
		model->opaqueFrustumCullingDescriptorSet.bind(commandBuffer, &frustumCulling.computePipeline, 1);
		frustumCulling.computePipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &model->opaqueDrawCount);

		vkCmdDispatch(commandBuffer->commandBuffer, 256, 1, 1);
	}

	void cullMask(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex) {
		model->maskFrustumCullingDescriptorSet.bind(commandBuffer, &frustumCulling.computePipeline, 1);
		frustumCulling.computePipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &model->maskDrawCount);

		vkCmdDispatch(commandBuffer->commandBuffer, 256, 1, 1);
	}

	void cullBlend(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex) {
		model->blendFrustumCullingDescriptorSet.bind(commandBuffer, &frustumCulling.computePipeline, 1);
		frustumCulling.computePipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &model->blendDrawCount);

		vkCmdDispatch(commandBuffer->commandBuffer, 256, 1, 1);
	}

	void drawOpaque(CommandBuffer* commandBuffer, GraphicsPipeline* opaqueGraphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling) {
		if (bindTextures) {
			materialsDescriptorSets[frameInFlightIndex].bind(commandBuffer, opaqueGraphicsPipeline, 1);
			DescriptorSet* selectedDescriptorSet = culling ? &opaqueCulledDrawIndirectInfoDescriptorSet : &model->opaqueDrawIndirectInfoDescriptorSet;
			selectedDescriptorSet->bind(commandBuffer, opaqueGraphicsPipeline, 2);
		}
		VkBuffer selectedBuffer = culling ? opaqueCulledDrawIndirectBuffer.buffer : model->opaqueDrawIndirectBuffer.buffer;
		VkBuffer selectedDrawCount = culling ? opaqueCulledDrawCountBuffer.buffer : model->opaqueDrawCountBuffer.buffer;
		vkCmdDrawIndexedIndirectCount(commandBuffer->commandBuffer, selectedBuffer, 0, selectedDrawCount, 0, model->opaqueDrawCount, sizeof(VkDrawIndexedIndirectCommand));
	}

	void drawMask(CommandBuffer* commandBuffer, GraphicsPipeline* maskGraphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling) {
		if (bindTextures) {
			materialsDescriptorSets[frameInFlightIndex].bind(commandBuffer, maskGraphicsPipeline, 1);
			DescriptorSet* selectedDescriptorSet = culling ? &maskCulledDrawIndirectInfoDescriptorSet : &model->maskDrawIndirectInfoDescriptorSet;
			selectedDescriptorSet->bind(commandBuffer, maskGraphicsPipeline, 2);
		}
		VkBuffer selectedBuffer = culling ? maskCulledDrawIndirectBuffer.buffer : model->maskDrawIndirectBuffer.buffer;
		VkBuffer selectedDrawCount = culling ? maskCulledDrawCountBuffer.buffer : model->maskDrawCountBuffer.buffer;
		vkCmdDrawIndexedIndirectCount(commandBuffer->commandBuffer, selectedBuffer, 0, selectedDrawCount, 0, model->maskDrawCount, sizeof(VkDrawIndexedIndirectCommand));
	}

	void drawBlend(CommandBuffer* commandBuffer, GraphicsPipeline* blendGraphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling) {
		if (bindTextures) {
			materialsDescriptorSets[frameInFlightIndex].bind(commandBuffer, blendGraphicsPipeline, 1);
			DescriptorSet* selectedDescriptorSet = culling ? &blendCulledDrawIndirectInfoDescriptorSet : &model->blendDrawIndirectInfoDescriptorSet;
			selectedDescriptorSet->bind(commandBuffer, blendGraphicsPipeline, 2);
		}
		VkBuffer selectedBuffer = culling ? blendCulledDrawIndirectBuffer.buffer : model->blendDrawIndirectBuffer.buffer;
		VkBuffer selectedDrawCount = culling ? blendCulledDrawCountBuffer.buffer : model->blendDrawCountBuffer.buffer;
		vkCmdDrawIndexedIndirectCount(commandBuffer->commandBuffer, selectedBuffer, 0, selectedDrawCount, 0, model->blendDrawCount, sizeof(VkDrawIndexedIndirectCommand));
	}
};