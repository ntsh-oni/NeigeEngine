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
		// GPU resources
		BufferTools::createBuffer(opaqueCulledDrawIndirectBuffer.buffer, opaqueDrawCount * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &opaqueCulledDrawIndirectBuffer.memoryInfo);
		BufferTools::createBuffer(opaqueCulledDrawCountBuffer.buffer, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &opaqueCulledDrawCountBuffer.memoryInfo);
		BufferTools::createStorageBuffer(opaqueCulledDrawIndirectInfoBuffer.buffer, opaqueDrawCount * sizeof(PerDraw), &opaqueCulledDrawIndirectInfoBuffer.memoryInfo);

		indirectBuffers.push_back(opaqueCulledDrawIndirectBuffer.buffer);
		drawCountBuffers.push_back(opaqueCulledDrawCountBuffer.buffer);
		perDrawBuffers.push_back(opaqueCulledDrawIndirectInfoBuffer.buffer);

		// Descriptor set allocation
		VkDescriptorSetAllocateInfo perDrawCulledDescriptorSetAllocateInfo = {};
		perDrawCulledDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		perDrawCulledDescriptorSetAllocateInfo.pNext = nullptr;
		perDrawCulledDescriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
		perDrawCulledDescriptorSetAllocateInfo.descriptorSetCount = 1;
		perDrawCulledDescriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
		NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &perDrawCulledDescriptorSetAllocateInfo, &opaqueCulledDrawIndirectInfoDescriptorSet.descriptorSet));

		opaqueCulledDrawIndirectInfoDescriptorSet.descriptorPool = &perDrawDescriptorPool;

		opaqueAABBBuffers.resize(framesInFlight);
		opaqueFrustumCullingDescriptorSets.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++) {
			// AABB buffer
			BufferTools::createStorageBuffer(opaqueAABBBuffers[i].buffer, opaqueDrawCount * sizeof(AABB), &opaqueAABBBuffers[i].memoryInfo);

			// Frustum culling descriptor set allocation
			opaqueFrustumCullingDescriptorSets[i].init(&frustumCulling.computePipeline, 0);
		}
		
		// Base resources
		BufferTools::createBuffer(opaqueDrawIndirectBuffer.buffer, opaqueDrawCount * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &opaqueDrawIndirectBuffer.memoryInfo);
		BufferTools::createIndirectBuffer(opaqueDrawCountBuffer.buffer, sizeof(uint32_t), &opaqueDrawCountBuffer.memoryInfo);
		BufferTools::createStorageBuffer(opaqueDrawIndirectInfoBuffer.buffer, opaqueDrawCount * sizeof(PerDraw), &opaqueDrawIndirectInfoBuffer.memoryInfo);

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
	}

	if (gotMaskPrimitives) {
		// GPU resources
		BufferTools::createBuffer(maskCulledDrawIndirectBuffer.buffer, maskDrawCount * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &maskCulledDrawIndirectBuffer.memoryInfo);
		BufferTools::createBuffer(maskCulledDrawCountBuffer.buffer, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &maskCulledDrawCountBuffer.memoryInfo);
		BufferTools::createStorageBuffer(maskCulledDrawIndirectInfoBuffer.buffer, maskDrawCount * sizeof(PerDraw), &maskCulledDrawIndirectInfoBuffer.memoryInfo);

		indirectBuffers.push_back(maskCulledDrawIndirectBuffer.buffer);
		drawCountBuffers.push_back(maskCulledDrawCountBuffer.buffer);
		perDrawBuffers.push_back(maskCulledDrawIndirectInfoBuffer.buffer);

		// Descriptor set allocation
		VkDescriptorSetAllocateInfo perDrawCulledDescriptorSetAllocateInfo = {};
		perDrawCulledDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		perDrawCulledDescriptorSetAllocateInfo.pNext = nullptr;
		perDrawCulledDescriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
		perDrawCulledDescriptorSetAllocateInfo.descriptorSetCount = 1;
		perDrawCulledDescriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
		NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &perDrawCulledDescriptorSetAllocateInfo, &maskCulledDrawIndirectInfoDescriptorSet.descriptorSet));

		maskCulledDrawIndirectInfoDescriptorSet.descriptorPool = &perDrawDescriptorPool;

		maskAABBBuffers.resize(framesInFlight);
		maskFrustumCullingDescriptorSets.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++) {
			// AABB buffer
			BufferTools::createStorageBuffer(maskAABBBuffers[i].buffer, maskDrawCount * sizeof(AABB), &maskAABBBuffers[i].memoryInfo);

			// Frustum culling descriptor set allocation
			maskFrustumCullingDescriptorSets[i].init(&frustumCulling.computePipeline, 0);
		}

		// Base resources
		BufferTools::createBuffer(maskDrawIndirectBuffer.buffer, maskDrawCount * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &maskDrawIndirectBuffer.memoryInfo);
		BufferTools::createIndirectBuffer(maskDrawCountBuffer.buffer, sizeof(uint32_t), &maskDrawCountBuffer.memoryInfo);
		BufferTools::createStorageBuffer(maskDrawIndirectInfoBuffer.buffer, maskDrawCount * sizeof(PerDraw), &maskDrawIndirectInfoBuffer.memoryInfo);

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
	}

	if (gotBlendPrimitives) {
		// GPU resources
		BufferTools::createBuffer(blendCulledDrawIndirectBuffer.buffer, blendDrawCount * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &blendCulledDrawIndirectBuffer.memoryInfo);
		BufferTools::createBuffer(blendCulledDrawCountBuffer.buffer, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &blendCulledDrawCountBuffer.memoryInfo);
		BufferTools::createStorageBuffer(blendCulledDrawIndirectInfoBuffer.buffer, blendDrawCount * sizeof(PerDraw), &blendCulledDrawIndirectInfoBuffer.memoryInfo);

		indirectBuffers.push_back(blendCulledDrawIndirectBuffer.buffer);
		drawCountBuffers.push_back(blendCulledDrawCountBuffer.buffer);
		perDrawBuffers.push_back(blendCulledDrawIndirectInfoBuffer.buffer);

		// Descriptor set allocation
		VkDescriptorSetAllocateInfo perDrawCulledDescriptorSetAllocateInfo = {};
		perDrawCulledDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		perDrawCulledDescriptorSetAllocateInfo.pNext = nullptr;
		perDrawCulledDescriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
		perDrawCulledDescriptorSetAllocateInfo.descriptorSetCount = 1;
		perDrawCulledDescriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
		NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &perDrawCulledDescriptorSetAllocateInfo, &blendCulledDrawIndirectInfoDescriptorSet.descriptorSet));

		blendCulledDrawIndirectInfoDescriptorSet.descriptorPool = &perDrawDescriptorPool;

		blendAABBBuffers.resize(framesInFlight);
		blendFrustumCullingDescriptorSets.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++) {
			BufferTools::createStorageBuffer(blendAABBBuffers[i].buffer, blendDrawCount * sizeof(AABB), &blendAABBBuffers[i].memoryInfo);

			// Frustum culling descriptor set allocation
			blendFrustumCullingDescriptorSets[i].init(&frustumCulling.computePipeline, 0);
		}

		// Base resources
		BufferTools::createBuffer(blendDrawIndirectBuffer.buffer, blendDrawCount * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &blendDrawIndirectBuffer.memoryInfo);
		BufferTools::createIndirectBuffer(blendDrawCountBuffer.buffer, sizeof(uint32_t), &blendDrawCountBuffer.memoryInfo);
		BufferTools::createStorageBuffer(blendDrawIndirectInfoBuffer.buffer, blendDrawCount * sizeof(PerDraw), &blendDrawIndirectInfoBuffer.memoryInfo);

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
	}

	for (Mesh& mesh : meshes) {
		// Bones
		NEIGE_ASSERT(mesh.boneList.size() <= MAX_BONES, "A mesh has more than " + std::to_string(MAX_BONES) + " bones.");

		BufferTools::createUniformBuffer(mesh.boneBuffer.buffer, sizeof(BoneUniformBufferObject), &mesh.boneBuffer.memoryInfo);

		BoneUniformBufferObject bubo = {};
		for (size_t i = 0; i < mesh.boneList.size(); i++) {
			bubo.transformations[i] = mesh.boneList[i].transformation;
			bubo.inverseBindMatrices[i] = mesh.boneList[i].inverseBindMatrix;
		}

		void* data;
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

	void* indirectData;

	if (gotOpaquePrimitives) {
		// Buffers
		opaqueDrawIndirectBuffer.map(0, opaqueDrawCount * sizeof(VkDrawIndexedIndirectCommand), &indirectData);
		memcpy(indirectData, opaqueDrawIndirectCommands.data(), opaqueDrawCount * sizeof(VkDrawIndexedIndirectCommand));
		opaqueDrawIndirectBuffer.unmap();

		opaqueDrawCountBuffer.map(0, sizeof(uint32_t), &indirectData);
		memcpy(indirectData, &opaqueDrawCount, sizeof(uint32_t));
		opaqueDrawCountBuffer.unmap();

		opaqueDrawIndirectInfoBuffer.map(0, opaqueDrawCount * sizeof(PerDraw), &indirectData);
		memcpy(indirectData, opaqueDrawIndirectInfos.data(), opaqueDrawCount * sizeof(PerDraw));
		opaqueDrawIndirectInfoBuffer.unmap();
		
		// Descriptor sets
		// GPU
		VkDescriptorBufferInfo perCulledDrawInfo = {};
		perCulledDrawInfo.buffer = opaqueCulledDrawIndirectInfoBuffer.buffer;
		perCulledDrawInfo.offset = 0;
		perCulledDrawInfo.range = opaqueDrawCount * sizeof(PerDraw);

		std::vector<VkWriteDescriptorSet> perDrawCulledWritesDescriptorSet;

		VkWriteDescriptorSet perDrawCulledWriteDescriptorSet = {};
		perDrawCulledWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		perDrawCulledWriteDescriptorSet.pNext = nullptr;
		perDrawCulledWriteDescriptorSet.dstSet = opaqueCulledDrawIndirectInfoDescriptorSet.descriptorSet;
		perDrawCulledWriteDescriptorSet.dstBinding = 0;
		perDrawCulledWriteDescriptorSet.dstArrayElement = 0;
		perDrawCulledWriteDescriptorSet.descriptorCount = 1;
		perDrawCulledWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		perDrawCulledWriteDescriptorSet.pImageInfo = nullptr;
		perDrawCulledWriteDescriptorSet.pBufferInfo = &perCulledDrawInfo;
		perDrawCulledWriteDescriptorSet.pTexelBufferView = nullptr;
		perDrawCulledWritesDescriptorSet.push_back(perDrawCulledWriteDescriptorSet);

		opaqueCulledDrawIndirectInfoDescriptorSet.update(perDrawCulledWritesDescriptorSet);

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
			inAABBInfo.buffer = opaqueAABBBuffers[i].buffer;
			inAABBInfo.offset = 0;
			inAABBInfo.range = opaqueDrawCount * sizeof(AABB);

			VkDescriptorBufferInfo inFrustumInfo = {};
			inFrustumInfo.buffer = frustumBuffers[i].buffer;
			inFrustumInfo.offset = 0;
			inFrustumInfo.range = 6 * 4 * sizeof(float);

			VkDescriptorBufferInfo outDrawIndirectInfo = {};
			outDrawIndirectInfo.buffer = opaqueCulledDrawIndirectBuffer.buffer;
			outDrawIndirectInfo.offset = 0;
			outDrawIndirectInfo.range = opaqueDrawCount * sizeof(VkDrawIndexedIndirectCommand);

			VkDescriptorBufferInfo outPerDrawInfo = {};
			outPerDrawInfo.buffer = opaqueCulledDrawIndirectInfoBuffer.buffer;
			outPerDrawInfo.offset = 0;
			outPerDrawInfo.range = opaqueDrawCount * sizeof(PerDraw);

			VkDescriptorBufferInfo outDrawCountInfo = {};
			outDrawCountInfo.buffer = opaqueCulledDrawCountBuffer.buffer;
			outDrawCountInfo.offset = 0;
			outDrawCountInfo.range = sizeof(uint32_t);

			std::vector<VkWriteDescriptorSet> frustumCullingWritesDescriptorSet;

			VkWriteDescriptorSet inDrawIndirectWriteDescriptorSet = {};
			inDrawIndirectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inDrawIndirectWriteDescriptorSet.pNext = nullptr;
			inDrawIndirectWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSets[i].descriptorSet;
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
			inPerDrawWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSets[i].descriptorSet;
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
			inAABBWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSets[i].descriptorSet;
			inAABBWriteDescriptorSet.dstBinding = 2;
			inAABBWriteDescriptorSet.dstArrayElement = 0;
			inAABBWriteDescriptorSet.descriptorCount = 1;
			inAABBWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			inAABBWriteDescriptorSet.pImageInfo = nullptr;
			inAABBWriteDescriptorSet.pBufferInfo = &inAABBInfo;
			inAABBWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(inAABBWriteDescriptorSet);

			VkWriteDescriptorSet inFrustumWriteDescriptorSet = {};
			inFrustumWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inFrustumWriteDescriptorSet.pNext = nullptr;
			inFrustumWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSets[i].descriptorSet;
			inFrustumWriteDescriptorSet.dstBinding = 3;
			inFrustumWriteDescriptorSet.dstArrayElement = 0;
			inFrustumWriteDescriptorSet.descriptorCount = 1;
			inFrustumWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			inFrustumWriteDescriptorSet.pImageInfo = nullptr;
			inFrustumWriteDescriptorSet.pBufferInfo = &inFrustumInfo;
			inFrustumWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(inFrustumWriteDescriptorSet);

			VkWriteDescriptorSet outDrawIndirectWriteDescriptorSet = {};
			outDrawIndirectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			outDrawIndirectWriteDescriptorSet.pNext = nullptr;
			outDrawIndirectWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSets[i].descriptorSet;
			outDrawIndirectWriteDescriptorSet.dstBinding = 4;
			outDrawIndirectWriteDescriptorSet.dstArrayElement = 0;
			outDrawIndirectWriteDescriptorSet.descriptorCount = 1;
			outDrawIndirectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			outDrawIndirectWriteDescriptorSet.pImageInfo = nullptr;
			outDrawIndirectWriteDescriptorSet.pBufferInfo = &outDrawIndirectInfo;
			outDrawIndirectWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(outDrawIndirectWriteDescriptorSet);

			VkWriteDescriptorSet outPerDrawWriteDescriptorSet = {};
			outPerDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			outPerDrawWriteDescriptorSet.pNext = nullptr;
			outPerDrawWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSets[i].descriptorSet;
			outPerDrawWriteDescriptorSet.dstBinding = 5;
			outPerDrawWriteDescriptorSet.dstArrayElement = 0;
			outPerDrawWriteDescriptorSet.descriptorCount = 1;
			outPerDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			outPerDrawWriteDescriptorSet.pImageInfo = nullptr;
			outPerDrawWriteDescriptorSet.pBufferInfo = &outPerDrawInfo;
			outPerDrawWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(outPerDrawWriteDescriptorSet);

			VkWriteDescriptorSet outDrawCountWriteDescriptorSet = {};
			outDrawCountWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			outDrawCountWriteDescriptorSet.pNext = nullptr;
			outDrawCountWriteDescriptorSet.dstSet = opaqueFrustumCullingDescriptorSets[i].descriptorSet;
			outDrawCountWriteDescriptorSet.dstBinding = 6;
			outDrawCountWriteDescriptorSet.dstArrayElement = 0;
			outDrawCountWriteDescriptorSet.descriptorCount = 1;
			outDrawCountWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			outDrawCountWriteDescriptorSet.pImageInfo = nullptr;
			outDrawCountWriteDescriptorSet.pBufferInfo = &outDrawCountInfo;
			outDrawCountWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(outDrawCountWriteDescriptorSet);

			opaqueFrustumCullingDescriptorSets[i].update(frustumCullingWritesDescriptorSet);
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
		maskDrawIndirectBuffer.map(0, maskDrawCount * sizeof(VkDrawIndexedIndirectCommand), &indirectData);
		memcpy(indirectData, maskDrawIndirectCommands.data(), maskDrawCount * sizeof(VkDrawIndexedIndirectCommand));
		maskDrawIndirectBuffer.unmap();

		maskDrawCountBuffer.map(0, sizeof(uint32_t), &indirectData);
		memcpy(indirectData, &maskDrawCount, sizeof(uint32_t));
		maskDrawCountBuffer.unmap();

		maskDrawIndirectInfoBuffer.map(0, maskDrawCount * sizeof(PerDraw), &indirectData);
		memcpy(indirectData, maskDrawIndirectInfos.data(), maskDrawCount * sizeof(PerDraw));
		maskDrawIndirectInfoBuffer.unmap();

		// Descriptor sets
		// GPU
		VkDescriptorBufferInfo perDrawCulledInfo = {};
		perDrawCulledInfo.buffer = maskCulledDrawIndirectInfoBuffer.buffer;
		perDrawCulledInfo.offset = 0;
		perDrawCulledInfo.range = maskDrawCount * sizeof(PerDraw);

		std::vector<VkWriteDescriptorSet> perDrawCulledWritesDescriptorSet;

		VkWriteDescriptorSet perDrawCulledWriteDescriptorSet = {};
		perDrawCulledWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		perDrawCulledWriteDescriptorSet.pNext = nullptr;
		perDrawCulledWriteDescriptorSet.dstSet = maskCulledDrawIndirectInfoDescriptorSet.descriptorSet;
		perDrawCulledWriteDescriptorSet.dstBinding = 0;
		perDrawCulledWriteDescriptorSet.dstArrayElement = 0;
		perDrawCulledWriteDescriptorSet.descriptorCount = 1;
		perDrawCulledWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		perDrawCulledWriteDescriptorSet.pImageInfo = nullptr;
		perDrawCulledWriteDescriptorSet.pBufferInfo = &perDrawCulledInfo;
		perDrawCulledWriteDescriptorSet.pTexelBufferView = nullptr;
		perDrawCulledWritesDescriptorSet.push_back(perDrawCulledWriteDescriptorSet);

		maskCulledDrawIndirectInfoDescriptorSet.update(perDrawCulledWritesDescriptorSet);

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
			inAABBInfo.buffer = maskAABBBuffers[i].buffer;
			inAABBInfo.offset = 0;
			inAABBInfo.range = maskDrawCount * sizeof(AABB);

			VkDescriptorBufferInfo inFrustumInfo = {};
			inFrustumInfo.buffer = frustumBuffers[i].buffer;
			inFrustumInfo.offset = 0;
			inFrustumInfo.range = 6 * 4 * sizeof(float);

			VkDescriptorBufferInfo outDrawIndirectInfo = {};
			outDrawIndirectInfo.buffer = maskCulledDrawIndirectBuffer.buffer;
			outDrawIndirectInfo.offset = 0;
			outDrawIndirectInfo.range = maskDrawCount * sizeof(VkDrawIndexedIndirectCommand);

			VkDescriptorBufferInfo outPerDrawInfo = {};
			outPerDrawInfo.buffer = maskCulledDrawIndirectInfoBuffer.buffer;
			outPerDrawInfo.offset = 0;
			outPerDrawInfo.range = maskDrawCount * sizeof(PerDraw);

			VkDescriptorBufferInfo outDrawCountInfo = {};
			outDrawCountInfo.buffer = maskCulledDrawCountBuffer.buffer;
			outDrawCountInfo.offset = 0;
			outDrawCountInfo.range = sizeof(uint32_t);

			std::vector<VkWriteDescriptorSet> frustumCullingWritesDescriptorSet;

			VkWriteDescriptorSet inDrawIndirectWriteDescriptorSet = {};
			inDrawIndirectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inDrawIndirectWriteDescriptorSet.pNext = nullptr;
			inDrawIndirectWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSets[i].descriptorSet;
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
			inPerDrawWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSets[i].descriptorSet;
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
			inAABBWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSets[i].descriptorSet;
			inAABBWriteDescriptorSet.dstBinding = 2;
			inAABBWriteDescriptorSet.dstArrayElement = 0;
			inAABBWriteDescriptorSet.descriptorCount = 1;
			inAABBWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			inAABBWriteDescriptorSet.pImageInfo = nullptr;
			inAABBWriteDescriptorSet.pBufferInfo = &inAABBInfo;
			inAABBWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(inAABBWriteDescriptorSet);

			VkWriteDescriptorSet inFrustumWriteDescriptorSet = {};
			inFrustumWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inFrustumWriteDescriptorSet.pNext = nullptr;
			inFrustumWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSets[i].descriptorSet;
			inFrustumWriteDescriptorSet.dstBinding = 3;
			inFrustumWriteDescriptorSet.dstArrayElement = 0;
			inFrustumWriteDescriptorSet.descriptorCount = 1;
			inFrustumWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			inFrustumWriteDescriptorSet.pImageInfo = nullptr;
			inFrustumWriteDescriptorSet.pBufferInfo = &inFrustumInfo;
			inFrustumWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(inFrustumWriteDescriptorSet);

			VkWriteDescriptorSet outDrawIndirectWriteDescriptorSet = {};
			outDrawIndirectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			outDrawIndirectWriteDescriptorSet.pNext = nullptr;
			outDrawIndirectWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSets[i].descriptorSet;
			outDrawIndirectWriteDescriptorSet.dstBinding = 4;
			outDrawIndirectWriteDescriptorSet.dstArrayElement = 0;
			outDrawIndirectWriteDescriptorSet.descriptorCount = 1;
			outDrawIndirectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			outDrawIndirectWriteDescriptorSet.pImageInfo = nullptr;
			outDrawIndirectWriteDescriptorSet.pBufferInfo = &outDrawIndirectInfo;
			outDrawIndirectWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(outDrawIndirectWriteDescriptorSet);

			VkWriteDescriptorSet outPerDrawWriteDescriptorSet = {};
			outPerDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			outPerDrawWriteDescriptorSet.pNext = nullptr;
			outPerDrawWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSets[i].descriptorSet;
			outPerDrawWriteDescriptorSet.dstBinding = 5;
			outPerDrawWriteDescriptorSet.dstArrayElement = 0;
			outPerDrawWriteDescriptorSet.descriptorCount = 1;
			outPerDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			outPerDrawWriteDescriptorSet.pImageInfo = nullptr;
			outPerDrawWriteDescriptorSet.pBufferInfo = &outPerDrawInfo;
			outPerDrawWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(outPerDrawWriteDescriptorSet);

			VkWriteDescriptorSet outDrawCountWriteDescriptorSet = {};
			outDrawCountWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			outDrawCountWriteDescriptorSet.pNext = nullptr;
			outDrawCountWriteDescriptorSet.dstSet = maskFrustumCullingDescriptorSets[i].descriptorSet;
			outDrawCountWriteDescriptorSet.dstBinding = 6;
			outDrawCountWriteDescriptorSet.dstArrayElement = 0;
			outDrawCountWriteDescriptorSet.descriptorCount = 1;
			outDrawCountWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			outDrawCountWriteDescriptorSet.pImageInfo = nullptr;
			outDrawCountWriteDescriptorSet.pBufferInfo = &outDrawCountInfo;
			outDrawCountWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumCullingWritesDescriptorSet.push_back(outDrawCountWriteDescriptorSet);

			maskFrustumCullingDescriptorSets[i].update(frustumCullingWritesDescriptorSet);
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
		blendDrawIndirectBuffer.map(0, blendDrawCount * sizeof(VkDrawIndexedIndirectCommand), &indirectData);
		memcpy(indirectData, blendDrawIndirectCommands.data(), blendDrawCount * sizeof(VkDrawIndexedIndirectCommand));
		blendDrawIndirectBuffer.unmap();

		blendDrawCountBuffer.map(0, sizeof(uint32_t), &indirectData);
		memcpy(indirectData, &blendDrawCount, sizeof(uint32_t));
		blendDrawCountBuffer.unmap();

		blendDrawIndirectInfoBuffer.map(0, blendDrawCount * sizeof(PerDraw), &indirectData);
		memcpy(indirectData, blendDrawIndirectInfos.data(), blendDrawCount * sizeof(PerDraw));
		blendDrawIndirectInfoBuffer.unmap();

		// Descriptor sets
		// GPU
		VkDescriptorBufferInfo perDrawCulledInfo = {};
		perDrawCulledInfo.buffer = blendCulledDrawIndirectInfoBuffer.buffer;
		perDrawCulledInfo.offset = 0;
		perDrawCulledInfo.range = blendDrawCount * sizeof(PerDraw);

		std::vector<VkWriteDescriptorSet> perDrawCulledWritesDescriptorSet;

		VkWriteDescriptorSet perDrawCulledWriteDescriptorSet = {};
		perDrawCulledWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		perDrawCulledWriteDescriptorSet.pNext = nullptr;
		perDrawCulledWriteDescriptorSet.dstSet = blendCulledDrawIndirectInfoDescriptorSet.descriptorSet;
		perDrawCulledWriteDescriptorSet.dstBinding = 0;
		perDrawCulledWriteDescriptorSet.dstArrayElement = 0;
		perDrawCulledWriteDescriptorSet.descriptorCount = 1;
		perDrawCulledWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		perDrawCulledWriteDescriptorSet.pImageInfo = nullptr;
		perDrawCulledWriteDescriptorSet.pBufferInfo = &perDrawCulledInfo;
		perDrawCulledWriteDescriptorSet.pTexelBufferView = nullptr;
		perDrawCulledWritesDescriptorSet.push_back(perDrawCulledWriteDescriptorSet);

		blendCulledDrawIndirectInfoDescriptorSet.update(perDrawCulledWritesDescriptorSet);

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
			inAABBInfo.buffer = blendAABBBuffers[i].buffer;
			inAABBInfo.offset = 0;
			inAABBInfo.range = blendDrawCount * sizeof(AABB);

			VkDescriptorBufferInfo inFrustumInfo = {};
			inFrustumInfo.buffer = frustumBuffers[i].buffer;
			inFrustumInfo.offset = 0;
			inFrustumInfo.range = 6 * 4 * sizeof(float);

			VkDescriptorBufferInfo outDrawIndirectInfo = {};
			outDrawIndirectInfo.buffer = blendCulledDrawIndirectBuffer.buffer;
			outDrawIndirectInfo.offset = 0;
			outDrawIndirectInfo.range = blendDrawCount * sizeof(VkDrawIndexedIndirectCommand);

			VkDescriptorBufferInfo outPerDrawInfo = {};
			outPerDrawInfo.buffer = blendCulledDrawIndirectInfoBuffer.buffer;
			outPerDrawInfo.offset = 0;
			outPerDrawInfo.range = blendDrawCount * sizeof(PerDraw);

			VkDescriptorBufferInfo outDrawCountInfo = {};
			outDrawCountInfo.buffer = blendCulledDrawCountBuffer.buffer;
			outDrawCountInfo.offset = 0;
			outDrawCountInfo.range = sizeof(uint32_t);

			std::vector<VkWriteDescriptorSet> frustumWritesDescriptorSet;

			VkWriteDescriptorSet inDrawIndirectWriteDescriptorSet = {};
			inDrawIndirectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inDrawIndirectWriteDescriptorSet.pNext = nullptr;
			inDrawIndirectWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSets[i].descriptorSet;
			inDrawIndirectWriteDescriptorSet.dstBinding = 0;
			inDrawIndirectWriteDescriptorSet.dstArrayElement = 0;
			inDrawIndirectWriteDescriptorSet.descriptorCount = 1;
			inDrawIndirectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			inDrawIndirectWriteDescriptorSet.pImageInfo = nullptr;
			inDrawIndirectWriteDescriptorSet.pBufferInfo = &inDrawIndirectInfo;
			inDrawIndirectWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumWritesDescriptorSet.push_back(inDrawIndirectWriteDescriptorSet);

			VkWriteDescriptorSet inPerDrawWriteDescriptorSet = {};
			inPerDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inPerDrawWriteDescriptorSet.pNext = nullptr;
			inPerDrawWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSets[i].descriptorSet;
			inPerDrawWriteDescriptorSet.dstBinding = 1;
			inPerDrawWriteDescriptorSet.dstArrayElement = 0;
			inPerDrawWriteDescriptorSet.descriptorCount = 1;
			inPerDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			inPerDrawWriteDescriptorSet.pImageInfo = nullptr;
			inPerDrawWriteDescriptorSet.pBufferInfo = &inPerDrawInfo;
			inPerDrawWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumWritesDescriptorSet.push_back(inPerDrawWriteDescriptorSet);

			VkWriteDescriptorSet inAABBWriteDescriptorSet = {};
			inAABBWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inAABBWriteDescriptorSet.pNext = nullptr;
			inAABBWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSets[i].descriptorSet;
			inAABBWriteDescriptorSet.dstBinding = 2;
			inAABBWriteDescriptorSet.dstArrayElement = 0;
			inAABBWriteDescriptorSet.descriptorCount = 1;
			inAABBWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			inAABBWriteDescriptorSet.pImageInfo = nullptr;
			inAABBWriteDescriptorSet.pBufferInfo = &inAABBInfo;
			inAABBWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumWritesDescriptorSet.push_back(inAABBWriteDescriptorSet);

			VkWriteDescriptorSet inFrustumWriteDescriptorSet = {};
			inFrustumWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			inFrustumWriteDescriptorSet.pNext = nullptr;
			inFrustumWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSets[i].descriptorSet;
			inFrustumWriteDescriptorSet.dstBinding = 3;
			inFrustumWriteDescriptorSet.dstArrayElement = 0;
			inFrustumWriteDescriptorSet.descriptorCount = 1;
			inFrustumWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			inFrustumWriteDescriptorSet.pImageInfo = nullptr;
			inFrustumWriteDescriptorSet.pBufferInfo = &inFrustumInfo;
			inFrustumWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumWritesDescriptorSet.push_back(inFrustumWriteDescriptorSet);

			VkWriteDescriptorSet outDrawIndirectWriteDescriptorSet = {};
			outDrawIndirectWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			outDrawIndirectWriteDescriptorSet.pNext = nullptr;
			outDrawIndirectWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSets[i].descriptorSet;
			outDrawIndirectWriteDescriptorSet.dstBinding = 4;
			outDrawIndirectWriteDescriptorSet.dstArrayElement = 0;
			outDrawIndirectWriteDescriptorSet.descriptorCount = 1;
			outDrawIndirectWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			outDrawIndirectWriteDescriptorSet.pImageInfo = nullptr;
			outDrawIndirectWriteDescriptorSet.pBufferInfo = &outDrawIndirectInfo;
			outDrawIndirectWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumWritesDescriptorSet.push_back(outDrawIndirectWriteDescriptorSet);

			VkWriteDescriptorSet outPerDrawWriteDescriptorSet = {};
			outPerDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			outPerDrawWriteDescriptorSet.pNext = nullptr;
			outPerDrawWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSets[i].descriptorSet;
			outPerDrawWriteDescriptorSet.dstBinding = 5;
			outPerDrawWriteDescriptorSet.dstArrayElement = 0;
			outPerDrawWriteDescriptorSet.descriptorCount = 1;
			outPerDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			outPerDrawWriteDescriptorSet.pImageInfo = nullptr;
			outPerDrawWriteDescriptorSet.pBufferInfo = &outPerDrawInfo;
			outPerDrawWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumWritesDescriptorSet.push_back(outPerDrawWriteDescriptorSet);

			VkWriteDescriptorSet outDrawCountWriteDescriptorSet = {};
			outDrawCountWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			outDrawCountWriteDescriptorSet.pNext = nullptr;
			outDrawCountWriteDescriptorSet.dstSet = blendFrustumCullingDescriptorSets[i].descriptorSet;
			outDrawCountWriteDescriptorSet.dstBinding = 6;
			outDrawCountWriteDescriptorSet.dstArrayElement = 0;
			outDrawCountWriteDescriptorSet.descriptorCount = 1;
			outDrawCountWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			outDrawCountWriteDescriptorSet.pImageInfo = nullptr;
			outDrawCountWriteDescriptorSet.pBufferInfo = &outDrawCountInfo;
			outDrawCountWriteDescriptorSet.pTexelBufferView = nullptr;
			frustumWritesDescriptorSet.push_back(outDrawCountWriteDescriptorSet);

			blendFrustumCullingDescriptorSets[i].update(frustumWritesDescriptorSet);
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
		// GPU
		opaqueCulledDrawIndirectBuffer.destroy();
		opaqueCulledDrawCountBuffer.destroy();
		opaqueCulledDrawIndirectInfoBuffer.destroy();
		for (uint32_t i = 0; i < framesInFlight; i++) {
			opaqueAABBBuffers[i].destroy();
		}

		// Base resource
		opaqueDrawIndirectBuffer.destroy();
		opaqueDrawCountBuffer.destroy();
		opaqueDrawIndirectInfoBuffer.destroy();
	}
	if (gotMaskPrimitives) {
		// GPU
		maskCulledDrawIndirectBuffer.destroy();
		maskCulledDrawCountBuffer.destroy();
		maskCulledDrawIndirectInfoBuffer.destroy();
		for (uint32_t i = 0; i < framesInFlight; i++) {
			maskAABBBuffers[i].destroy();
		}
		
		// Base resource
		maskDrawIndirectBuffer.destroy();
		maskDrawCountBuffer.destroy();
		maskDrawIndirectInfoBuffer.destroy();
	}
	if (gotBlendPrimitives) {
		// GPU
		blendCulledDrawIndirectBuffer.destroy();
		blendCulledDrawCountBuffer.destroy();
		blendCulledDrawIndirectInfoBuffer.destroy();
		for (uint32_t i = 0; i < framesInFlight; i++) {
			blendAABBBuffers[i].destroy();
		}
		
		// Base resource
		blendDrawIndirectBuffer.destroy();
		blendDrawCountBuffer.destroy();
		blendDrawIndirectInfoBuffer.destroy();
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

void Model::cullOpaque(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex) {
	opaqueFrustumCullingDescriptorSets[frameInFlightIndex].bind(commandBuffer, &frustumCulling.computePipeline, 0);
	frustumCulling.computePipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &opaqueDrawCount);

	vkCmdDispatch(commandBuffer->commandBuffer, 256, 1, 1);
}

void Model::cullMask(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex) {
	maskFrustumCullingDescriptorSets[frameInFlightIndex].bind(commandBuffer, &frustumCulling.computePipeline, 0);
	frustumCulling.computePipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &maskDrawCount);

	vkCmdDispatch(commandBuffer->commandBuffer, 256, 1, 1);
}

void Model::cullBlend(CommandBuffer* commandBuffer, uint32_t frameInFlightIndex) {
	blendFrustumCullingDescriptorSets[frameInFlightIndex].bind(commandBuffer, &frustumCulling.computePipeline, 0);
	frustumCulling.computePipeline.pushConstant(commandBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &blendDrawCount);

	vkCmdDispatch(commandBuffer->commandBuffer, 256, 1, 1);
}

void Model::drawOpaque(CommandBuffer* commandBuffer, GraphicsPipeline* opaqueGraphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling) {
	if (bindTextures) {
		materialsDescriptorSet.bind(commandBuffer, opaqueGraphicsPipeline, 1);
		DescriptorSet* selectedDescriptorSet = culling ? &opaqueCulledDrawIndirectInfoDescriptorSet : &opaqueDrawIndirectInfoDescriptorSet;
		selectedDescriptorSet->bind(commandBuffer, opaqueGraphicsPipeline, 2);
	}
	VkBuffer selectedBuffer = culling ? opaqueCulledDrawIndirectBuffer.buffer : opaqueDrawIndirectBuffer.buffer;
	VkBuffer selectedDrawCount = culling ? opaqueCulledDrawCountBuffer.buffer : opaqueDrawCountBuffer.buffer;
	vkCmdDrawIndexedIndirectCount(commandBuffer->commandBuffer, selectedBuffer, 0, selectedDrawCount, 0, opaqueDrawCount, sizeof(VkDrawIndexedIndirectCommand));
}

void Model::drawMask(CommandBuffer* commandBuffer, GraphicsPipeline* maskGraphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling, uint32_t pushConstantOffset) {
	if (bindTextures) {
		materialsDescriptorSet.bind(commandBuffer, maskGraphicsPipeline, 1);
		DescriptorSet* selectedDescriptorSet = culling ? &maskCulledDrawIndirectInfoDescriptorSet : &maskDrawIndirectInfoDescriptorSet;
		selectedDescriptorSet->bind(commandBuffer, maskGraphicsPipeline, 2);
	}
	VkBuffer selectedBuffer = culling ? maskCulledDrawIndirectBuffer.buffer : maskDrawIndirectBuffer.buffer;
	VkBuffer selectedDrawCount = culling ? maskCulledDrawCountBuffer.buffer : maskDrawCountBuffer.buffer;
	vkCmdDrawIndexedIndirectCount(commandBuffer->commandBuffer, selectedBuffer, 0, selectedDrawCount, 0, maskDrawCount, sizeof(VkDrawIndexedIndirectCommand));
}

void Model::drawBlend(CommandBuffer* commandBuffer, GraphicsPipeline* blendGraphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling) {
	if (bindTextures) {
		materialsDescriptorSet.bind(commandBuffer, blendGraphicsPipeline, 1);
		DescriptorSet* selectedDescriptorSet = culling ? &blendCulledDrawIndirectInfoDescriptorSet : &blendDrawIndirectInfoDescriptorSet;
		selectedDescriptorSet->bind(commandBuffer, blendGraphicsPipeline, 2);
	}
	VkBuffer selectedBuffer = culling ? blendCulledDrawIndirectBuffer.buffer : blendDrawIndirectBuffer.buffer;
	VkBuffer selectedDrawCount = culling ? blendCulledDrawCountBuffer.buffer : blendDrawCountBuffer.buffer;
	vkCmdDrawIndexedIndirectCount(commandBuffer->commandBuffer, selectedBuffer, 0, selectedDrawCount, 0, blendDrawCount, sizeof(VkDrawIndexedIndirectCommand));
}