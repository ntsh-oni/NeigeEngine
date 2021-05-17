#include "Model.h"
#include "../resources/RendererResources.h"
#include "../resources/ShaderResources.h"

void Model::init(std::string filePath) {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	ModelLoader::load(filePath, &vertices, &indices, &meshes);

	for (Mesh& mesh : meshes) {
		if (mesh.opaquePrimitives.size() != 0) {
			gotOpaquePrimitives = true;
			opaqueDrawCount += static_cast<uint32_t>(mesh.opaquePrimitives.size());
		}
		if (mesh.maskPrimitives.size() != 0) {
			gotMaskPrimitives = true;
			maskDrawCount += static_cast<uint32_t>(mesh.maskPrimitives.size());
		}
		if (mesh.blendPrimitives.size() != 0) {
			gotBlendPrimitives = true;
			blendDrawCount += static_cast<uint32_t>(mesh.blendPrimitives.size());
		}
	}

	Buffer stagingVertexBuffer;
	VkDeviceSize size = vertices.size() * sizeof(Vertex);
	BufferTools::createStagingBuffer(stagingVertexBuffer.buffer, size, &stagingVertexBuffer.memoryInfo);
	void* vertexData;
	stagingVertexBuffer.map(0, size, &vertexData);
	memcpy(vertexData, vertices.data(), static_cast<size_t>(size));
	stagingVertexBuffer.unmap();
	BufferTools::createBuffer(vertexBuffer.buffer, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer.memoryInfo);
	BufferTools::copyBuffer(stagingVertexBuffer.buffer, vertexBuffer.buffer, size);
	stagingVertexBuffer.destroy();

	Buffer stagingIndexBuffer;
	size = indices.size() * sizeof(uint32_t);
	BufferTools::createStagingBuffer(stagingIndexBuffer.buffer, size, &stagingIndexBuffer.memoryInfo);
	void* indexData;
	stagingIndexBuffer.map(0, size, &indexData);
	memcpy(indexData, indices.data(), static_cast<size_t>(size));
	stagingIndexBuffer.unmap();
	BufferTools::createBuffer(indexBuffer.buffer, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &indexBuffer.memoryInfo);
	BufferTools::copyBuffer(stagingIndexBuffer.buffer, indexBuffer.buffer, size);
	stagingIndexBuffer.destroy();

	// Indirect
	if (gotOpaquePrimitives) {
		BufferTools::createBuffer(opaqueDrawIndirectBuffer.buffer, opaqueDrawCount * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &opaqueDrawIndirectBuffer.memoryInfo);
		BufferTools::createIndirectBuffer(opaqueDrawCountBuffer.buffer, sizeof(uint32_t), &opaqueDrawCountBuffer.memoryInfo);
		BufferTools::createStorageBuffer(opaqueDrawIndirectInfoBuffer.buffer, opaqueDrawCount * sizeof(PerDraw), &opaqueDrawIndirectInfoBuffer.memoryInfo);
		BufferTools::createStorageBuffer(opaqueAABBBuffer.buffer, opaqueDrawCount * sizeof(AABB), &opaqueAABBBuffer.memoryInfo);

		// Descriptor set allocation
		VkDescriptorSetAllocateInfo perDrawDescriptorSetAllocateInfo = {};
		perDrawDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		perDrawDescriptorSetAllocateInfo.pNext = nullptr;
		perDrawDescriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
		perDrawDescriptorSetAllocateInfo.descriptorSetCount = 1;
		perDrawDescriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
		NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &perDrawDescriptorSetAllocateInfo, &opaqueDrawIndirectInfoDescriptorSet.descriptorSet));

		opaqueDrawIndirectInfoDescriptorSet.descriptorPool = &perDrawDescriptorPool;
		perDrawDescriptorPool.remainingSets -= (framesInFlight + 1);

		// Frustum culling descriptor set allocation
		opaqueFrustumCullingDescriptorSet.init(&frustumCulling.computePipeline, 1);
	}

	if (gotMaskPrimitives) {
		BufferTools::createBuffer(maskDrawIndirectBuffer.buffer, maskDrawCount * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &maskDrawIndirectBuffer.memoryInfo);
		BufferTools::createIndirectBuffer(maskDrawCountBuffer.buffer, sizeof(uint32_t), &maskDrawCountBuffer.memoryInfo);
		BufferTools::createStorageBuffer(maskDrawIndirectInfoBuffer.buffer, maskDrawCount * sizeof(PerDraw), &maskDrawIndirectInfoBuffer.memoryInfo);
		BufferTools::createStorageBuffer(maskAABBBuffer.buffer, maskDrawCount * sizeof(AABB), &maskAABBBuffer.memoryInfo);

		// Descriptor set allocation
		VkDescriptorSetAllocateInfo perDrawDescriptorSetAllocateInfo = {};
		perDrawDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		perDrawDescriptorSetAllocateInfo.pNext = nullptr;
		perDrawDescriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
		perDrawDescriptorSetAllocateInfo.descriptorSetCount = 1;
		perDrawDescriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
		NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &perDrawDescriptorSetAllocateInfo, &maskDrawIndirectInfoDescriptorSet.descriptorSet));

		maskDrawIndirectInfoDescriptorSet.descriptorPool = &perDrawDescriptorPool;
		perDrawDescriptorPool.remainingSets -= (framesInFlight + 1);

		// Frustum culling descriptor set allocation
		maskFrustumCullingDescriptorSet.init(&frustumCulling.computePipeline, 1);
	}

	if (gotBlendPrimitives) {
		BufferTools::createBuffer(blendDrawIndirectBuffer.buffer, blendDrawCount * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &blendDrawIndirectBuffer.memoryInfo);
		BufferTools::createIndirectBuffer(blendDrawCountBuffer.buffer, sizeof(uint32_t), &blendDrawCountBuffer.memoryInfo);
		BufferTools::createStorageBuffer(blendDrawIndirectInfoBuffer.buffer, blendDrawCount * sizeof(PerDraw), &blendDrawIndirectInfoBuffer.memoryInfo);
		BufferTools::createStorageBuffer(blendAABBBuffer.buffer, blendDrawCount * sizeof(AABB), &blendAABBBuffer.memoryInfo);

		// Descriptor set allocation
		VkDescriptorSetAllocateInfo perDrawDescriptorSetAllocateInfo = {};
		perDrawDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		perDrawDescriptorSetAllocateInfo.pNext = nullptr;
		perDrawDescriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
		perDrawDescriptorSetAllocateInfo.descriptorSetCount = 1;
		perDrawDescriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
		NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &perDrawDescriptorSetAllocateInfo, &blendDrawIndirectInfoDescriptorSet.descriptorSet));

		blendDrawIndirectInfoDescriptorSet.descriptorPool = &perDrawDescriptorPool;
		perDrawDescriptorPool.remainingSets -= (framesInFlight + 1);

		// Frustum culling descriptor set allocation
		blendFrustumCullingDescriptorSet.init(&frustumCulling.computePipeline, 1);
	}

	void* data;

	std::vector<AABB> opaqueAABB;
	std::vector<AABB> maskAABB;
	std::vector<AABB> blendAABB;
	for (Mesh& mesh : meshes) {
		// Bones
		NEIGE_ASSERT(mesh.boneList.size() <= MAX_BONES, "A mesh has more than " + std::to_string(MAX_BONES) + " bones.");

		BufferTools::createUniformBuffer(mesh.boneBuffer.buffer, sizeof(BoneUniformBufferObject), &mesh.boneBuffer.memoryInfo);

		BoneUniformBufferObject bubo = {};
		for (size_t i = 0; i < mesh.boneList.size(); i++) {
			bubo.transformations[i] = mesh.boneList[i].transformation;
			bubo.inverseBindMatrices[i] = mesh.boneList[i].inverseBindMatrix;
		}

		mesh.boneBuffer.map(0, sizeof(BoneUniformBufferObject), &data);
		memcpy(data, &bubo, sizeof(BoneUniformBufferObject));
		mesh.boneBuffer.unmap();

		// Indirect commands
		for (Primitive& primitive : mesh.opaquePrimitives) {
			VkDrawIndexedIndirectCommand drawIndirectCommand = {};
			drawIndirectCommand.indexCount = primitive.indexCount;
			drawIndirectCommand.instanceCount = 1;
			drawIndirectCommand.firstIndex = mesh.indexOffset + primitive.firstIndex;
			drawIndirectCommand.vertexOffset = mesh.vertexOffset + primitive.vertexOffset;
			drawIndirectCommand.firstInstance = 0;
			opaqueDrawIndirectCommands.push_back(drawIndirectCommand);

			PerDraw perDraw = {};
			perDraw.materialIndex = static_cast<int>(primitive.materialIndex);
			perDraw.alphaCutoff = 0.0f;
			opaqueDrawIndirectInfos.push_back(perDraw);

			opaqueAABB.push_back(primitive.aabb);
		}
		for (size_t i = 0; i < mesh.maskPrimitives.size(); i++) {
			VkDrawIndexedIndirectCommand drawIndirectCommand = {};
			drawIndirectCommand.indexCount = mesh.maskPrimitives[i].indexCount;
			drawIndirectCommand.instanceCount = 1;
			drawIndirectCommand.firstIndex = mesh.indexOffset + mesh.maskPrimitives[i].firstIndex;
			drawIndirectCommand.vertexOffset = mesh.vertexOffset + mesh.maskPrimitives[i].vertexOffset;
			drawIndirectCommand.firstInstance = 0;
			maskDrawIndirectCommands.push_back(drawIndirectCommand);

			PerDraw perDraw = {};
			perDraw.materialIndex = static_cast<int>(mesh.maskPrimitives[i].materialIndex);
			perDraw.alphaCutoff = mesh.alphaCutoffs[i];
			maskDrawIndirectInfos.push_back(perDraw);

			maskAABB.push_back(mesh.maskPrimitives[i].aabb);
		}
		for (Primitive& primitive : mesh.blendPrimitives) {
			VkDrawIndexedIndirectCommand drawIndirectCommand = {};
			drawIndirectCommand.indexCount = primitive.indexCount;
			drawIndirectCommand.instanceCount = 1;
			drawIndirectCommand.firstIndex = mesh.indexOffset + primitive.firstIndex;
			drawIndirectCommand.vertexOffset = mesh.vertexOffset + primitive.vertexOffset;
			drawIndirectCommand.firstInstance = 0;
			blendDrawIndirectCommands.push_back(drawIndirectCommand);

			PerDraw perDraw = {};
			perDraw.materialIndex = static_cast<int>(primitive.materialIndex);
			perDraw.alphaCutoff = 0.0f;
			blendDrawIndirectInfos.push_back(perDraw);

			blendAABB.push_back(primitive.aabb);
		}

		// Model AABB
		if (mesh.aabb.min.x < aabb.min.x) {
			aabb.min.x = mesh.aabb.min.x;
		}
		if (mesh.aabb.max.x > aabb.max.x) {
			aabb.max.x = mesh.aabb.max.x;
		}
		if (mesh.aabb.min.y < aabb.min.y) {
			aabb.min.y = mesh.aabb.min.y;
		}
		if (mesh.aabb.max.y > aabb.max.y) {
			aabb.max.y = mesh.aabb.max.y;
		}
		if (mesh.aabb.min.z < aabb.min.z) {
			aabb.min.z = mesh.aabb.min.z;
		}
		if (mesh.aabb.max.z > aabb.max.z) {
			aabb.max.z = mesh.aabb.max.z;
		}
	}

	if (gotOpaquePrimitives) {
		// Buffers
		opaqueDrawIndirectBuffer.map(0, opaqueDrawCount * sizeof(VkDrawIndexedIndirectCommand), &data);
		memcpy(data, opaqueDrawIndirectCommands.data(), opaqueDrawCount * sizeof(VkDrawIndexedIndirectCommand));
		opaqueDrawIndirectBuffer.unmap();

		opaqueDrawCountBuffer.map(0, sizeof(uint32_t), &data);
		memcpy(data, &opaqueDrawCount, sizeof(uint32_t));
		opaqueDrawCountBuffer.unmap();

		opaqueDrawIndirectInfoBuffer.map(0, opaqueDrawCount * sizeof(PerDraw), &data);
		memcpy(data, opaqueDrawIndirectInfos.data(), opaqueDrawCount * sizeof(PerDraw));
		opaqueDrawIndirectInfoBuffer.unmap();

		opaqueAABBBuffer.map(0, opaqueDrawCount * sizeof(AABB), &data);
		memcpy(data, opaqueAABB.data(), opaqueDrawCount * sizeof(AABB));
		opaqueAABBBuffer.unmap();
		
		// Frustum culling descriptor sets update
		for (uint32_t i = 0; i < framesInFlight; i++) {
			VkDescriptorBufferInfo inDrawIndirectInfo = {};
			inDrawIndirectInfo.buffer = opaqueDrawIndirectBuffer.buffer;
			inDrawIndirectInfo.offset = 0;
			inDrawIndirectInfo.range = opaqueDrawCount * sizeof(VkDrawIndexedIndirectCommand);

			VkDescriptorBufferInfo inPerDrawInfo = {};
			inPerDrawInfo.buffer = opaqueDrawIndirectInfoBuffer.buffer;
			inPerDrawInfo.offset = 0;
			inPerDrawInfo.range = opaqueDrawCount * sizeof(PerDraw);

			VkDescriptorBufferInfo inAABBInfo = {};
			inAABBInfo.buffer = opaqueAABBBuffer.buffer;
			inAABBInfo.offset = 0;
			inAABBInfo.range = opaqueDrawCount * sizeof(AABB);

			VkDescriptorBufferInfo inFrustumInfo = {};
			inFrustumInfo.buffer = frustumBuffers[i].buffer;
			inFrustumInfo.offset = 0;
			inFrustumInfo.range = 6 * 4 * sizeof(float);

			std::vector<VkWriteDescriptorSet> frustumCullingWritesDescriptorSet;

			VkWriteDescriptorSet inDrawIndirectWriteDescriptorSet = {};
			inDrawIndirectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inDrawIndirectWriteDescriptorSet.pNext = nullptr;
			inDrawIndirectWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSet.descriptorSet;
			inDrawIndirectWriteDescriptorSet.dstBinding = 0;
			inDrawIndirectWriteDescriptorSet.dstArrayElement = 0;
			inDrawIndirectWriteDescriptorSet.descriptorCount = 1;
			inDrawIndirectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			inDrawIndirectWriteDescriptorSet.pImageInfo = nullptr;
			inDrawIndirectWriteDescriptorSet.pBufferInfo = &inDrawIndirectInfo;
			inDrawIndirectWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(inDrawIndirectWriteDescriptorSet);

			VkWriteDescriptorSet inPerDrawWriteDescriptorSet = {};
			inPerDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inPerDrawWriteDescriptorSet.pNext = nullptr;
			inPerDrawWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSet.descriptorSet;
			inPerDrawWriteDescriptorSet.dstBinding = 1;
			inPerDrawWriteDescriptorSet.dstArrayElement = 0;
			inPerDrawWriteDescriptorSet.descriptorCount = 1;
			inPerDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			inPerDrawWriteDescriptorSet.pImageInfo = nullptr;
			inPerDrawWriteDescriptorSet.pBufferInfo = &inPerDrawInfo;
			inPerDrawWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(inPerDrawWriteDescriptorSet);

			VkWriteDescriptorSet inAABBWriteDescriptorSet = {};
			inAABBWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inAABBWriteDescriptorSet.pNext = nullptr;
			inAABBWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSet.descriptorSet;
			inAABBWriteDescriptorSet.dstBinding = 2;
			inAABBWriteDescriptorSet.dstArrayElement = 0;
			inAABBWriteDescriptorSet.descriptorCount = 1;
			inAABBWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			inAABBWriteDescriptorSet.pImageInfo = nullptr;
			inAABBWriteDescriptorSet.pBufferInfo = &inAABBInfo;
			inAABBWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(inAABBWriteDescriptorSet);

			opaqueFrustumCullingDescriptorSet.update(frustumCullingWritesDescriptorSet);
		}

		// Base resources
		VkDescriptorBufferInfo perDrawInfo = {};
		perDrawInfo.buffer = opaqueDrawIndirectInfoBuffer.buffer;
		perDrawInfo.offset = 0;
		perDrawInfo.range = opaqueDrawCount * sizeof(PerDraw);

		std::vector<VkWriteDescriptorSet> perDrawWritesDescriptorSet;

		VkWriteDescriptorSet perDrawWriteDescriptorSet = {};
		perDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		perDrawWriteDescriptorSet.pNext = nullptr;
		perDrawWriteDescriptorSet.dstSet = opaqueDrawIndirectInfoDescriptorSet.descriptorSet;
		perDrawWriteDescriptorSet.dstBinding = 0;
		perDrawWriteDescriptorSet.dstArrayElement = 0;
		perDrawWriteDescriptorSet.descriptorCount = 1;
		perDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		perDrawWriteDescriptorSet.pImageInfo = nullptr;
		perDrawWriteDescriptorSet.pBufferInfo = &perDrawInfo;
		perDrawWriteDescriptorSet.pTexelBufferView = nullptr;
		perDrawWritesDescriptorSet.push_back(perDrawWriteDescriptorSet);
		
		opaqueDrawIndirectInfoDescriptorSet.update(perDrawWritesDescriptorSet);
	}

	if (gotMaskPrimitives) {
		// Buffers
		maskDrawIndirectBuffer.map(0, maskDrawCount * sizeof(VkDrawIndexedIndirectCommand), &data);
		memcpy(data, maskDrawIndirectCommands.data(), maskDrawCount * sizeof(VkDrawIndexedIndirectCommand));
		maskDrawIndirectBuffer.unmap();

		maskDrawCountBuffer.map(0, sizeof(uint32_t), &data);
		memcpy(data, &maskDrawCount, sizeof(uint32_t));
		maskDrawCountBuffer.unmap();

		maskDrawIndirectInfoBuffer.map(0, maskDrawCount * sizeof(PerDraw), &data);
		memcpy(data, maskDrawIndirectInfos.data(), maskDrawCount * sizeof(PerDraw));
		maskDrawIndirectInfoBuffer.unmap();

		maskAABBBuffer.map(0, maskDrawCount * sizeof(AABB), &data);
		memcpy(data, maskAABB.data(), maskDrawCount * sizeof(AABB));
		maskAABBBuffer.unmap();

		// Frustum culling descriptor sets update
		for (uint32_t i = 0; i < framesInFlight; i++) {
			VkDescriptorBufferInfo inDrawIndirectInfo = {};
			inDrawIndirectInfo.buffer = maskDrawIndirectBuffer.buffer;
			inDrawIndirectInfo.offset = 0;
			inDrawIndirectInfo.range = maskDrawCount * sizeof(VkDrawIndexedIndirectCommand);

			VkDescriptorBufferInfo inPerDrawInfo = {};
			inPerDrawInfo.buffer = maskDrawIndirectInfoBuffer.buffer;
			inPerDrawInfo.offset = 0;
			inPerDrawInfo.range = maskDrawCount * sizeof(PerDraw);

			VkDescriptorBufferInfo inAABBInfo = {};
			inAABBInfo.buffer = maskAABBBuffer.buffer;
			inAABBInfo.offset = 0;
			inAABBInfo.range = maskDrawCount * sizeof(AABB);

			VkDescriptorBufferInfo inFrustumInfo = {};
			inFrustumInfo.buffer = frustumBuffers[i].buffer;
			inFrustumInfo.offset = 0;
			inFrustumInfo.range = 6 * 4 * sizeof(float);

			std::vector<VkWriteDescriptorSet> frustumCullingWritesDescriptorSet;

			VkWriteDescriptorSet inDrawIndirectWriteDescriptorSet = {};
			inDrawIndirectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inDrawIndirectWriteDescriptorSet.pNext = nullptr;
			inDrawIndirectWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSet.descriptorSet;
			inDrawIndirectWriteDescriptorSet.dstBinding = 0;
			inDrawIndirectWriteDescriptorSet.dstArrayElement = 0;
			inDrawIndirectWriteDescriptorSet.descriptorCount = 1;
			inDrawIndirectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			inDrawIndirectWriteDescriptorSet.pImageInfo = nullptr;
			inDrawIndirectWriteDescriptorSet.pBufferInfo = &inDrawIndirectInfo;
			inDrawIndirectWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(inDrawIndirectWriteDescriptorSet);

			VkWriteDescriptorSet inPerDrawWriteDescriptorSet = {};
			inPerDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inPerDrawWriteDescriptorSet.pNext = nullptr;
			inPerDrawWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSet.descriptorSet;
			inPerDrawWriteDescriptorSet.dstBinding = 1;
			inPerDrawWriteDescriptorSet.dstArrayElement = 0;
			inPerDrawWriteDescriptorSet.descriptorCount = 1;
			inPerDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			inPerDrawWriteDescriptorSet.pImageInfo = nullptr;
			inPerDrawWriteDescriptorSet.pBufferInfo = &inPerDrawInfo;
			inPerDrawWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(inPerDrawWriteDescriptorSet);

			VkWriteDescriptorSet inAABBWriteDescriptorSet = {};
			inAABBWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inAABBWriteDescriptorSet.pNext = nullptr;
			inAABBWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSet.descriptorSet;
			inAABBWriteDescriptorSet.dstBinding = 2;
			inAABBWriteDescriptorSet.dstArrayElement = 0;
			inAABBWriteDescriptorSet.descriptorCount = 1;
			inAABBWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			inAABBWriteDescriptorSet.pImageInfo = nullptr;
			inAABBWriteDescriptorSet.pBufferInfo = &inAABBInfo;
			inAABBWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(inAABBWriteDescriptorSet);

			maskFrustumCullingDescriptorSet.update(frustumCullingWritesDescriptorSet);
		}

		// Base resources
		VkDescriptorBufferInfo perDrawInfo = {};
		perDrawInfo.buffer = maskDrawIndirectInfoBuffer.buffer;
		perDrawInfo.offset = 0;
		perDrawInfo.range = maskDrawCount * sizeof(PerDraw);

		std::vector<VkWriteDescriptorSet> perDrawWritesDescriptorSet;
		
		VkWriteDescriptorSet perDrawWriteDescriptorSet = {};
		perDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		perDrawWriteDescriptorSet.pNext = nullptr;
		perDrawWriteDescriptorSet.dstSet = maskDrawIndirectInfoDescriptorSet.descriptorSet;
		perDrawWriteDescriptorSet.dstBinding = 0;
		perDrawWriteDescriptorSet.dstArrayElement = 0;
		perDrawWriteDescriptorSet.descriptorCount = 1;
		perDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		perDrawWriteDescriptorSet.pImageInfo = nullptr;
		perDrawWriteDescriptorSet.pBufferInfo = &perDrawInfo;
		perDrawWriteDescriptorSet.pTexelBufferView = nullptr;
		perDrawWritesDescriptorSet.push_back(perDrawWriteDescriptorSet);

		maskDrawIndirectInfoDescriptorSet.update(perDrawWritesDescriptorSet);
	}

	if (gotBlendPrimitives) {
		// Buffers
		blendDrawIndirectBuffer.map(0, blendDrawCount * sizeof(VkDrawIndexedIndirectCommand), &data);
		memcpy(data, blendDrawIndirectCommands.data(), blendDrawCount * sizeof(VkDrawIndexedIndirectCommand));
		blendDrawIndirectBuffer.unmap();

		blendDrawCountBuffer.map(0, sizeof(uint32_t), &data);
		memcpy(data, &blendDrawCount, sizeof(uint32_t));
		blendDrawCountBuffer.unmap();

		blendDrawIndirectInfoBuffer.map(0, blendDrawCount * sizeof(PerDraw), &data);
		memcpy(data, blendDrawIndirectInfos.data(), blendDrawCount * sizeof(PerDraw));
		blendDrawIndirectInfoBuffer.unmap();

		blendAABBBuffer.map(0, blendDrawCount * sizeof(AABB), &data);
		memcpy(data, blendAABB.data(), blendDrawCount * sizeof(AABB));
		blendAABBBuffer.unmap();

		// Frustum culling descriptor sets update
		for (uint32_t i = 0; i < framesInFlight; i++) {
			VkDescriptorBufferInfo inDrawIndirectInfo = {};
			inDrawIndirectInfo.buffer = blendDrawIndirectBuffer.buffer;
			inDrawIndirectInfo.offset = 0;
			inDrawIndirectInfo.range = blendDrawCount * sizeof(VkDrawIndexedIndirectCommand);

			VkDescriptorBufferInfo inPerDrawInfo = {};
			inPerDrawInfo.buffer = blendDrawIndirectInfoBuffer.buffer;
			inPerDrawInfo.offset = 0;
			inPerDrawInfo.range = blendDrawCount * sizeof(PerDraw);

			VkDescriptorBufferInfo inAABBInfo = {};
			inAABBInfo.buffer = blendAABBBuffer.buffer;
			inAABBInfo.offset = 0;
			inAABBInfo.range = blendDrawCount * sizeof(AABB);

			VkDescriptorBufferInfo inFrustumInfo = {};
			inFrustumInfo.buffer = frustumBuffers[i].buffer;
			inFrustumInfo.offset = 0;
			inFrustumInfo.range = 6 * 4 * sizeof(float);

			std::vector<VkWriteDescriptorSet> frustumCullingWritesDescriptorSet;

			VkWriteDescriptorSet inDrawIndirectWriteDescriptorSet = {};
			inDrawIndirectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inDrawIndirectWriteDescriptorSet.pNext = nullptr;
			inDrawIndirectWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSet.descriptorSet;
			inDrawIndirectWriteDescriptorSet.dstBinding = 0;
			inDrawIndirectWriteDescriptorSet.dstArrayElement = 0;
			inDrawIndirectWriteDescriptorSet.descriptorCount = 1;
			inDrawIndirectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			inDrawIndirectWriteDescriptorSet.pImageInfo = nullptr;
			inDrawIndirectWriteDescriptorSet.pBufferInfo = &inDrawIndirectInfo;
			inDrawIndirectWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(inDrawIndirectWriteDescriptorSet);

			VkWriteDescriptorSet inPerDrawWriteDescriptorSet = {};
			inPerDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inPerDrawWriteDescriptorSet.pNext = nullptr;
			inPerDrawWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSet.descriptorSet;
			inPerDrawWriteDescriptorSet.dstBinding = 1;
			inPerDrawWriteDescriptorSet.dstArrayElement = 0;
			inPerDrawWriteDescriptorSet.descriptorCount = 1;
			inPerDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			inPerDrawWriteDescriptorSet.pImageInfo = nullptr;
			inPerDrawWriteDescriptorSet.pBufferInfo = &inPerDrawInfo;
			inPerDrawWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(inPerDrawWriteDescriptorSet);

			VkWriteDescriptorSet inAABBWriteDescriptorSet = {};
			inAABBWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inAABBWriteDescriptorSet.pNext = nullptr;
			inAABBWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSet.descriptorSet;
			inAABBWriteDescriptorSet.dstBinding = 2;
			inAABBWriteDescriptorSet.dstArrayElement = 0;
			inAABBWriteDescriptorSet.descriptorCount = 1;
			inAABBWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			inAABBWriteDescriptorSet.pImageInfo = nullptr;
			inAABBWriteDescriptorSet.pBufferInfo = &inAABBInfo;
			inAABBWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(inAABBWriteDescriptorSet);

			blendFrustumCullingDescriptorSet.update(frustumCullingWritesDescriptorSet);
		}

		// Base resources
		VkDescriptorBufferInfo perDrawInfo = {};
		perDrawInfo.buffer = blendDrawIndirectInfoBuffer.buffer;
		perDrawInfo.offset = 0;
		perDrawInfo.range = blendDrawCount * sizeof(PerDraw);

		std::vector<VkWriteDescriptorSet> perDrawWritesDescriptorSet;

		VkWriteDescriptorSet perDrawWriteDescriptorSet = {};
		perDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		perDrawWriteDescriptorSet.pNext = nullptr;
		perDrawWriteDescriptorSet.dstSet = blendDrawIndirectInfoDescriptorSet.descriptorSet;
		perDrawWriteDescriptorSet.dstBinding = 0;
		perDrawWriteDescriptorSet.dstArrayElement = 0;
		perDrawWriteDescriptorSet.descriptorCount = 1;
		perDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		perDrawWriteDescriptorSet.pImageInfo = nullptr;
		perDrawWriteDescriptorSet.pBufferInfo = &perDrawInfo;
		perDrawWriteDescriptorSet.pTexelBufferView = nullptr;
		perDrawWritesDescriptorSet.push_back(perDrawWriteDescriptorSet);

		blendDrawIndirectInfoDescriptorSet.update(perDrawWritesDescriptorSet);
	}
}

void Model::destroy() {
	vertexBuffer.destroy();
	indexBuffer.destroy();
	if (gotOpaquePrimitives) {
		opaqueDrawIndirectBuffer.destroy();
		opaqueDrawCountBuffer.destroy();
		opaqueDrawIndirectInfoBuffer.destroy();
		opaqueAABBBuffer.destroy();
	}
	if (gotMaskPrimitives) {
		maskDrawIndirectBuffer.destroy();
		maskDrawCountBuffer.destroy();
		maskDrawIndirectInfoBuffer.destroy();
		maskAABBBuffer.destroy();
	}
	if (gotBlendPrimitives) {
		blendDrawIndirectBuffer.destroy();
		blendDrawCountBuffer.destroy();
		blendDrawIndirectInfoBuffer.destroy();
		blendAABBBuffer.destroy();
	}
	for (Mesh& mesh : meshes) {
		mesh.boneBuffer.destroy();
	}
}

void Model::bindBuffers(CommandBuffer* commandBuffer) {
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(commandBuffer->commandBuffer, 0, 1, &vertexBuffer.buffer, &offset);
	vkCmdBindIndexBuffer(commandBuffer->commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
}