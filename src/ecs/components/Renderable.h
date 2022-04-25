#pragma once
#include "../../graphics/resources/Buffer.h"
#include "../../graphics/pipelines/DescriptorSet.h"
#include "../../graphics/pipelines/GraphicsPipeline.h"
#include "../../graphics/resources/Samplers.h"
#include "../../graphics/resources/ShaderResources.h"
#include "../../graphics/structs/ShaderStructs.h"
#include <string>
#include <vector>
#include <iostream>

struct RenderableComponent {
	std::string modelPath = "";

	std::string vertexShaderPath = "";
	std::string fragmentShaderPath = "";
	std::string tesselationControlShaderPath = "";
	std::string tesselationEvaluationShaderPath = "";
	std::string geometryShaderPath = "";
};

struct Renderable {
	RenderableComponent component;

	// State
	bool loaded = false;
	bool destroyed = false;
	uint32_t destroyCounter = 0;

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
		lookupString = component.vertexShaderPath + component.fragmentShaderPath + component.tesselationControlShaderPath + component.tesselationEvaluationShaderPath + component.geometryShaderPath + std::to_string(static_cast<int>(topology));
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

		VkDescriptorBufferInfo objectInfo = {};
		VkDescriptorBufferInfo cameraInfo = {};
		VkDescriptorBufferInfo shadowInfo = {};
		VkDescriptorBufferInfo lightingInfo = {};
		VkDescriptorImageInfo irradianceInfo = {};
		VkDescriptorImageInfo prefilterInfo = {};
		VkDescriptorImageInfo brdfLUTInfo = {};
		std::vector<VkDescriptorImageInfo> directionalShadowMapsInfos;
		std::vector<VkDescriptorImageInfo> spotShadowMapsInfos;
		VkDescriptorBufferInfo timeInfo = {};

		descriptorSets[frameInFlightIndex].writesDescriptorSet.clear();
		descriptorSets[frameInFlightIndex].writesDescriptorSet.shrink_to_fit();

		for (size_t i = 0; i < graphicsPipeline->sets[0].bindings.size(); i++) {
			std::string bindingName = graphicsPipeline->sets[0].bindings[i].name;
			if (bindingName == "object") {
				objectInfo.buffer = buffers.at(frameInFlightIndex).buffer;
				objectInfo.offset = 0;
				objectInfo.range = sizeof(ObjectUniformBufferObject);

				descriptorSets[frameInFlightIndex].addWriteUniformBuffer(graphicsPipeline->sets[0].bindings[i].binding.binding, 1, &objectInfo);
			}
			else if (bindingName == "camera") {
				cameraInfo.buffer = cameraBuffers.at(frameInFlightIndex).buffer;
				cameraInfo.offset = 0;
				cameraInfo.range = sizeof(CameraUniformBufferObject);

				descriptorSets[frameInFlightIndex].addWriteUniformBuffer(graphicsPipeline->sets[0].bindings[i].binding.binding, 1, &cameraInfo);
			}
			else if (bindingName == "shadow") {
				shadowInfo.buffer = shadow.buffers.at(frameInFlightIndex).buffer;
				shadowInfo.offset = 0;
				shadowInfo.range = sizeof(ShadowUniformBufferObject);

				descriptorSets[frameInFlightIndex].addWriteUniformBuffer(graphicsPipeline->sets[0].bindings[i].binding.binding, 1, &shadowInfo);
			}
			else if (bindingName == "lights") {
				lightingInfo.buffer = lightingBuffers.at(frameInFlightIndex).buffer;
				lightingInfo.offset = 0;
				lightingInfo.range = sizeof(LightingUniformBufferObject);

				descriptorSets[frameInFlightIndex].addWriteUniformBuffer(graphicsPipeline->sets[0].bindings[i].binding.binding, 1, &lightingInfo);
			}
			else if (bindingName == "irradianceMap") {
				irradianceInfo.sampler = trilinearEdgeBlackSampler;
				irradianceInfo.imageView = envmap.diffuseIradianceImage.imageView;
				irradianceInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				descriptorSets[frameInFlightIndex].addWriteCombinedImageSampler(graphicsPipeline->sets[0].bindings[i].binding.binding, 1, &irradianceInfo);
			}
			else if (bindingName == "prefilterMap") {
				prefilterInfo.sampler = trilinearEdgeBlackSampler;
				prefilterInfo.imageView = envmap.prefilterImage.imageView;
				prefilterInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				descriptorSets[frameInFlightIndex].addWriteCombinedImageSampler(graphicsPipeline->sets[0].bindings[i].binding.binding, 1, &prefilterInfo);
			}
			else if (bindingName == "brdfLUT") {
				brdfLUTInfo.sampler = trilinearEdgeBlackSampler;
				brdfLUTInfo.imageView = envmap.brdfConvolutionImage.imageView;
				brdfLUTInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				descriptorSets[frameInFlightIndex].addWriteCombinedImageSampler(graphicsPipeline->sets[0].bindings[i].binding.binding, 1, &brdfLUTInfo);
			}
			else if (bindingName == "directionalLightsShadowMaps") {
				directionalShadowMapsInfos.resize(MAX_DIR_LIGHTS);

				for (int j = 0; j < MAX_DIR_LIGHTS; j++) {
					directionalShadowMapsInfos[j].sampler = shadowSampler;
					directionalShadowMapsInfos[j].imageView = (j < shadow.directionalImages.size()) ? shadow.directionalImages[j].imageView : shadow.defaultShadow.imageView;
					directionalShadowMapsInfos[j].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				}

				descriptorSets[frameInFlightIndex].addWriteCombinedImageSampler(graphicsPipeline->sets[0].bindings[i].binding.binding, MAX_DIR_LIGHTS, directionalShadowMapsInfos.data());
			}
			else if (bindingName == "spotLightsShadowMaps") {
				spotShadowMapsInfos.resize(MAX_SPOT_LIGHTS);

				for (int j = 0; j < MAX_SPOT_LIGHTS; j++) {
					spotShadowMapsInfos[j].sampler = shadowSampler;
					spotShadowMapsInfos[j].imageView = (j < shadow.spotImages.size()) ? shadow.spotImages[j].imageView : shadow.defaultShadow.imageView;
					spotShadowMapsInfos[j].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				}

				descriptorSets[frameInFlightIndex].addWriteCombinedImageSampler(graphicsPipeline->sets[0].bindings[i].binding.binding, MAX_SPOT_LIGHTS, spotShadowMapsInfos.data());
			}
			else if (bindingName == "time") {
				timeInfo.buffer = timeBuffers.at(frameInFlightIndex).buffer;
				timeInfo.offset = 0;
				timeInfo.range = sizeof(float);

				descriptorSets[frameInFlightIndex].addWriteUniformBuffer(graphicsPipeline->sets[0].bindings[i].binding.binding, 1, &timeInfo);
			}
		}
		descriptorSets.at(frameInFlightIndex).update();
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

		opaqueFrustumCullingDescriptorSets[frameInFlightIndex].writesDescriptorSet.clear();
		opaqueFrustumCullingDescriptorSets[frameInFlightIndex].writesDescriptorSet.shrink_to_fit();

		opaqueFrustumCullingDescriptorSets[frameInFlightIndex].addWriteUniformBuffer(0, 1, &objectInfo);

		opaqueFrustumCullingDescriptorSets[frameInFlightIndex].addWriteUniformBuffer(1, 1, &inFrustumInfo);

		opaqueFrustumCullingDescriptorSets[frameInFlightIndex].addWriteStorageBuffer(2, 1, &outDrawIndirectInfo);

		opaqueFrustumCullingDescriptorSets[frameInFlightIndex].addWriteStorageBuffer(3, 1, &outPerDrawInfo);

		opaqueFrustumCullingDescriptorSets[frameInFlightIndex].addWriteStorageBuffer(4, 1, &outDrawCountInfo);

		opaqueFrustumCullingDescriptorSets[frameInFlightIndex].update();
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

		maskFrustumCullingDescriptorSets[frameInFlightIndex].writesDescriptorSet.clear();
		maskFrustumCullingDescriptorSets[frameInFlightIndex].writesDescriptorSet.shrink_to_fit();

		maskFrustumCullingDescriptorSets[frameInFlightIndex].addWriteUniformBuffer(0, 1, &objectInfo);

		maskFrustumCullingDescriptorSets[frameInFlightIndex].addWriteUniformBuffer(1, 1, &inFrustumInfo);

		maskFrustumCullingDescriptorSets[frameInFlightIndex].addWriteStorageBuffer(2, 1, &outDrawIndirectInfo);

		maskFrustumCullingDescriptorSets[frameInFlightIndex].addWriteStorageBuffer(3, 1, &outPerDrawInfo);

		maskFrustumCullingDescriptorSets[frameInFlightIndex].addWriteStorageBuffer(4, 1, &outDrawCountInfo);

		maskFrustumCullingDescriptorSets[frameInFlightIndex].update();
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

		blendFrustumCullingDescriptorSets[frameInFlightIndex].writesDescriptorSet.clear();
		blendFrustumCullingDescriptorSets[frameInFlightIndex].writesDescriptorSet.shrink_to_fit();

		blendFrustumCullingDescriptorSets[frameInFlightIndex].addWriteUniformBuffer(0, 1, &objectInfo);

		blendFrustumCullingDescriptorSets[frameInFlightIndex].addWriteUniformBuffer(1, 1, &inFrustumInfo);

		blendFrustumCullingDescriptorSets[frameInFlightIndex].addWriteStorageBuffer(2, 1, &outDrawIndirectInfo);

		blendFrustumCullingDescriptorSets[frameInFlightIndex].addWriteStorageBuffer(3, 1, &outPerDrawInfo);

		blendFrustumCullingDescriptorSets[frameInFlightIndex].addWriteStorageBuffer(4, 1, &outDrawCountInfo);

		blendFrustumCullingDescriptorSets[frameInFlightIndex].update();
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

		depthPrepassDescriptorSets[frameInFlightIndex].writesDescriptorSet.clear();
		depthPrepassDescriptorSets[frameInFlightIndex].writesDescriptorSet.shrink_to_fit();

		depthPrepassDescriptorSets[frameInFlightIndex].addWriteUniformBuffer(0, 1, &objectInfo);

		depthPrepassDescriptorSets[frameInFlightIndex].addWriteUniformBuffer(1, 1, &cameraInfo);

		depthPrepassDescriptorSets[frameInFlightIndex].update();
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

		depthPrepassMaskDescriptorSets[frameInFlightIndex].writesDescriptorSet.clear();
		depthPrepassMaskDescriptorSets[frameInFlightIndex].writesDescriptorSet.shrink_to_fit();

		depthPrepassMaskDescriptorSets[frameInFlightIndex].addWriteUniformBuffer(0, 1, &objectInfo);

		depthPrepassMaskDescriptorSets[frameInFlightIndex].addWriteUniformBuffer(1, 1, &cameraInfo);

		depthPrepassMaskDescriptorSets[frameInFlightIndex].update();
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

		shadowDescriptorSets[frameInFlightIndex].writesDescriptorSet.clear();
		shadowDescriptorSets[frameInFlightIndex].writesDescriptorSet.shrink_to_fit();

		shadowDescriptorSets[frameInFlightIndex].addWriteUniformBuffer(0, 1, &objectInfo);

		shadowDescriptorSets[frameInFlightIndex].addWriteUniformBuffer(1, 1, &shadowInfo);

		shadowDescriptorSets[frameInFlightIndex].update();
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

		shadowMaskDescriptorSets[frameInFlightIndex].writesDescriptorSet.clear();
		shadowMaskDescriptorSets[frameInFlightIndex].writesDescriptorSet.shrink_to_fit();

		shadowMaskDescriptorSets[frameInFlightIndex].addWriteUniformBuffer(0, 1, &objectInfo);

		shadowMaskDescriptorSets[frameInFlightIndex].addWriteUniformBuffer(1, 1, &shadowInfo);

		shadowMaskDescriptorSets[frameInFlightIndex].update();
	}

	void cullOpaque(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex) {
		model->opaqueFrustumCullingDescriptorSet.bind(commandBuffer, &frustumCulling.computePipeline, 1);
		frustumCulling.computePipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &model->opaqueDrawCount);

		uint32_t groups = static_cast<uint32_t>((model->opaqueDrawCount + 64 - 1) / 64);

		vkCmdDispatch(commandBuffer->commandBuffer, groups, 1, 1);
	}

	void cullMask(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex) {
		model->maskFrustumCullingDescriptorSet.bind(commandBuffer, &frustumCulling.computePipeline, 1);
		frustumCulling.computePipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &model->maskDrawCount);

		uint32_t groups = static_cast<uint32_t>((model->maskDrawCount + 64 - 1) / 64);

		vkCmdDispatch(commandBuffer->commandBuffer, groups, 1, 1);
	}

	void cullBlend(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex) {
		model->blendFrustumCullingDescriptorSet.bind(commandBuffer, &frustumCulling.computePipeline, 1);
		frustumCulling.computePipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &model->blendDrawCount);

		uint32_t groups = static_cast<uint32_t>((model->blendDrawCount + 64 - 1) / 64);

		vkCmdDispatch(commandBuffer->commandBuffer, groups, 1, 1);
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