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
		opaqueCulledDrawIndirectBuffers.resize(framesInFlight);
		opaqueCulledDrawIndirectInfoBuffers.resize(framesInFlight);
		opaqueCulledDrawIndirectInfoDescriptorSets.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++) {
			BufferTools::createIndirectBuffer(opaqueCulledDrawIndirectBuffers[i].buffer, opaqueDrawCount * sizeof(VkDrawIndexedIndirectCommand), &opaqueCulledDrawIndirectBuffers[i].memoryInfo);
			BufferTools::createStorageBuffer(opaqueCulledDrawIndirectInfoBuffers[i].buffer, opaqueDrawCount * sizeof(PerDraw), &opaqueCulledDrawIndirectInfoBuffers[i].memoryInfo);

			// Descriptor set allocation
			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
			descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocateInfo.pNext = nullptr;
			descriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
			descriptorSetAllocateInfo.descriptorSetCount = 1;
			descriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
			NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &descriptorSetAllocateInfo, &opaqueCulledDrawIndirectInfoDescriptorSets[i].descriptorSet));

			opaqueCulledDrawIndirectInfoDescriptorSets[i].descriptorPool = &perDrawDescriptorPool;
		}
		BufferTools::createIndirectBuffer(opaqueDrawIndirectBuffer.buffer, opaqueDrawCount * sizeof(VkDrawIndexedIndirectCommand), &opaqueDrawIndirectBuffer.memoryInfo);
		BufferTools::createStorageBuffer(opaqueDrawIndirectInfoBuffer.buffer, opaqueDrawCount * sizeof(PerDraw), &opaqueDrawIndirectInfoBuffer.memoryInfo);

		// Descriptor set allocation
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = 1;
		descriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
		NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &descriptorSetAllocateInfo, &opaqueDrawIndirectInfoDescriptorSet.descriptorSet));

		opaqueDrawIndirectInfoDescriptorSet.descriptorPool = &perDrawDescriptorPool;
		perDrawDescriptorPool.remainingSets -= (framesInFlight + 1);
	}

	if (gotMaskPrimitives) {
		maskCulledDrawIndirectBuffers.resize(framesInFlight);
		maskCulledDrawIndirectInfoBuffers.resize(framesInFlight);
		maskCulledDrawIndirectInfoDescriptorSets.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++) {
			BufferTools::createIndirectBuffer(maskCulledDrawIndirectBuffers[i].buffer, maskDrawCount * sizeof(VkDrawIndexedIndirectCommand), &maskCulledDrawIndirectBuffers[i].memoryInfo);
			BufferTools::createStorageBuffer(maskCulledDrawIndirectInfoBuffers[i].buffer, maskDrawCount * sizeof(PerDraw), &maskCulledDrawIndirectInfoBuffers[i].memoryInfo);

			// Descriptor set allocation
			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
			descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocateInfo.pNext = nullptr;
			descriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
			descriptorSetAllocateInfo.descriptorSetCount = 1;
			descriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
			NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &descriptorSetAllocateInfo, &maskCulledDrawIndirectInfoDescriptorSets[i].descriptorSet));

			maskCulledDrawIndirectInfoDescriptorSets[i].descriptorPool = &perDrawDescriptorPool;
		}
		BufferTools::createIndirectBuffer(maskDrawIndirectBuffer.buffer, maskDrawCount * sizeof(VkDrawIndexedIndirectCommand), &maskDrawIndirectBuffer.memoryInfo);
		BufferTools::createStorageBuffer(maskDrawIndirectInfoBuffer.buffer, maskDrawCount * sizeof(PerDraw), &maskDrawIndirectInfoBuffer.memoryInfo);

		// Descriptor set allocation
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = 1;
		descriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
		NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &descriptorSetAllocateInfo, &maskDrawIndirectInfoDescriptorSet.descriptorSet));

		maskDrawIndirectInfoDescriptorSet.descriptorPool = &perDrawDescriptorPool;
		perDrawDescriptorPool.remainingSets -= (framesInFlight + 1);
	}

	if (gotBlendPrimitives) {
		blendCulledDrawIndirectBuffers.resize(framesInFlight);
		blendCulledDrawIndirectInfoBuffers.resize(framesInFlight);
		blendCulledDrawIndirectInfoDescriptorSets.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++) {
			BufferTools::createIndirectBuffer(blendCulledDrawIndirectBuffers[i].buffer, blendDrawCount * sizeof(VkDrawIndexedIndirectCommand), &blendCulledDrawIndirectBuffers[i].memoryInfo);
			BufferTools::createStorageBuffer(blendCulledDrawIndirectInfoBuffers[i].buffer, blendDrawCount * sizeof(PerDraw), &blendCulledDrawIndirectInfoBuffers[i].memoryInfo);

			// Descriptor set allocation
			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
			descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocateInfo.pNext = nullptr;
			descriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
			descriptorSetAllocateInfo.descriptorSetCount = 1;
			descriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
			NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &descriptorSetAllocateInfo, &blendCulledDrawIndirectInfoDescriptorSets[i].descriptorSet));

			blendCulledDrawIndirectInfoDescriptorSets[i].descriptorPool = &perDrawDescriptorPool;
		}
		BufferTools::createIndirectBuffer(blendDrawIndirectBuffer.buffer, blendDrawCount * sizeof(VkDrawIndexedIndirectCommand), &blendDrawIndirectBuffer.memoryInfo);
		BufferTools::createStorageBuffer(blendDrawIndirectInfoBuffer.buffer, blendDrawCount * sizeof(PerDraw), &blendDrawIndirectInfoBuffer.memoryInfo);

		// Descriptor set allocation
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = perDrawDescriptorPool.descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = 1;
		descriptorSetAllocateInfo.pSetLayouts = &perDrawDescriptorSetLayout;
		NEIGE_VK_CHECK(vkAllocateDescriptorSets(logicalDevice.device, &descriptorSetAllocateInfo, &blendDrawIndirectInfoDescriptorSet.descriptorSet));

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

		opaqueDrawIndirectInfoBuffer.map(0, opaqueDrawCount * sizeof(PerDraw), &indirectData);
		memcpy(indirectData, opaqueDrawIndirectInfos.data(), opaqueDrawCount * sizeof(PerDraw));
		opaqueDrawIndirectInfoBuffer.unmap();
		
		// Descriptor Sets
		for (uint32_t i = 0; i < framesInFlight; i++) {
			VkDescriptorBufferInfo perDrawInfo = {};
			perDrawInfo.buffer = opaqueCulledDrawIndirectInfoBuffers[i].buffer;
			perDrawInfo.offset = 0;
			perDrawInfo.range = opaqueDrawCount * sizeof(PerDraw);

			std::vector<VkWriteDescriptorSet> writesDescriptorSet;

			VkWriteDescriptorSet perDrawWriteDescriptorSet = {};
			perDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			perDrawWriteDescriptorSet.pNext = nullptr;
			perDrawWriteDescriptorSet.dstSet = opaqueCulledDrawIndirectInfoDescriptorSets[i].descriptorSet;
			perDrawWriteDescriptorSet.dstBinding = 0;
			perDrawWriteDescriptorSet.dstArrayElement = 0;
			perDrawWriteDescriptorSet.descriptorCount = 1;
			perDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			perDrawWriteDescriptorSet.pImageInfo = nullptr;
			perDrawWriteDescriptorSet.pBufferInfo = &perDrawInfo;
			perDrawWriteDescriptorSet.pTexelBufferView = nullptr;
			writesDescriptorSet.push_back(perDrawWriteDescriptorSet);

			opaqueCulledDrawIndirectInfoDescriptorSets[i].update(writesDescriptorSet);
		}

		VkDescriptorBufferInfo perDrawInfo = {};
		perDrawInfo.buffer = opaqueDrawIndirectInfoBuffer.buffer;
		perDrawInfo.offset = 0;
		perDrawInfo.range = opaqueDrawCount * sizeof(PerDraw);

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

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
		writesDescriptorSet.push_back(perDrawWriteDescriptorSet);
		
		opaqueDrawIndirectInfoDescriptorSet.update(writesDescriptorSet);
	}

	if (gotMaskPrimitives) {
		maskDrawIndirectBuffer.map(0, maskDrawCount * sizeof(VkDrawIndexedIndirectCommand), &indirectData);
		memcpy(indirectData, maskDrawIndirectCommands.data(), maskDrawCount * sizeof(VkDrawIndexedIndirectCommand));
		maskDrawIndirectBuffer.unmap();

		maskDrawIndirectInfoBuffer.map(0, maskDrawCount * sizeof(PerDraw), &indirectData);
		memcpy(indirectData, maskDrawIndirectInfos.data(), maskDrawCount * sizeof(PerDraw));
		maskDrawIndirectInfoBuffer.unmap();

		// Descriptor Sets
		for (uint32_t i = 0; i < framesInFlight; i++) {
			VkDescriptorBufferInfo perDrawInfo = {};
			perDrawInfo.buffer = maskCulledDrawIndirectInfoBuffers[i].buffer;
			perDrawInfo.offset = 0;
			perDrawInfo.range = maskDrawCount * sizeof(PerDraw);

			std::vector<VkWriteDescriptorSet> writesDescriptorSet;

			VkWriteDescriptorSet perDrawWriteDescriptorSet = {};
			perDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			perDrawWriteDescriptorSet.pNext = nullptr;
			perDrawWriteDescriptorSet.dstSet = maskCulledDrawIndirectInfoDescriptorSets[i].descriptorSet;
			perDrawWriteDescriptorSet.dstBinding = 0;
			perDrawWriteDescriptorSet.dstArrayElement = 0;
			perDrawWriteDescriptorSet.descriptorCount = 1;
			perDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			perDrawWriteDescriptorSet.pImageInfo = nullptr;
			perDrawWriteDescriptorSet.pBufferInfo = &perDrawInfo;
			perDrawWriteDescriptorSet.pTexelBufferView = nullptr;
			writesDescriptorSet.push_back(perDrawWriteDescriptorSet);

			maskCulledDrawIndirectInfoDescriptorSets[i].update(writesDescriptorSet);
		}

		VkDescriptorBufferInfo perDrawInfo = {};
		perDrawInfo.buffer = maskDrawIndirectInfoBuffer.buffer;
		perDrawInfo.offset = 0;
		perDrawInfo.range = maskDrawCount * sizeof(PerDraw);

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;
		
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
		writesDescriptorSet.push_back(perDrawWriteDescriptorSet);

		maskDrawIndirectInfoDescriptorSet.update(writesDescriptorSet);
	}

	if (gotBlendPrimitives) {
		blendDrawIndirectBuffer.map(0, blendDrawCount * sizeof(VkDrawIndexedIndirectCommand), &indirectData);
		memcpy(indirectData, blendDrawIndirectCommands.data(), blendDrawCount * sizeof(VkDrawIndexedIndirectCommand));
		blendDrawIndirectBuffer.unmap();

		blendDrawIndirectInfoBuffer.map(0, blendDrawCount * sizeof(PerDraw), &indirectData);
		memcpy(indirectData, blendDrawIndirectInfos.data(), blendDrawCount * sizeof(PerDraw));
		blendDrawIndirectInfoBuffer.unmap();

		// Descriptor Sets
		for (uint32_t i = 0; i < framesInFlight; i++) {
			VkDescriptorBufferInfo perDrawInfo = {};
			perDrawInfo.buffer = blendCulledDrawIndirectInfoBuffers[i].buffer;
			perDrawInfo.offset = 0;
			perDrawInfo.range = blendDrawCount * sizeof(PerDraw);

			std::vector<VkWriteDescriptorSet> writesDescriptorSet;

			VkWriteDescriptorSet perDrawWriteDescriptorSet = {};
			perDrawWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			perDrawWriteDescriptorSet.pNext = nullptr;
			perDrawWriteDescriptorSet.dstSet = blendCulledDrawIndirectInfoDescriptorSets[i].descriptorSet;
			perDrawWriteDescriptorSet.dstBinding = 0;
			perDrawWriteDescriptorSet.dstArrayElement = 0;
			perDrawWriteDescriptorSet.descriptorCount = 1;
			perDrawWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			perDrawWriteDescriptorSet.pImageInfo = nullptr;
			perDrawWriteDescriptorSet.pBufferInfo = &perDrawInfo;
			perDrawWriteDescriptorSet.pTexelBufferView = nullptr;
			writesDescriptorSet.push_back(perDrawWriteDescriptorSet);

			blendCulledDrawIndirectInfoDescriptorSets[i].update(writesDescriptorSet);
		}

		VkDescriptorBufferInfo perDrawInfo = {};
		perDrawInfo.buffer = blendDrawIndirectInfoBuffer.buffer;
		perDrawInfo.offset = 0;
		perDrawInfo.range = blendDrawCount * sizeof(PerDraw);

		std::vector<VkWriteDescriptorSet> writesDescriptorSet;

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
		writesDescriptorSet.push_back(perDrawWriteDescriptorSet);

		blendDrawIndirectInfoDescriptorSet.update(writesDescriptorSet);
	}
}

void Model::destroy() {
	vertexBuffer.destroy();
	indexBuffer.destroy();
	if (gotOpaquePrimitives) {
		for (uint32_t i = 0; i < framesInFlight; i++) {
			opaqueCulledDrawIndirectBuffers[i].destroy();
			opaqueCulledDrawIndirectInfoBuffers[i].destroy();
		}
		opaqueDrawIndirectBuffer.destroy();
		opaqueDrawIndirectInfoBuffer.destroy();
	}
	if (gotMaskPrimitives) {
		for (uint32_t i = 0; i < framesInFlight; i++) {
			maskCulledDrawIndirectBuffers[i].destroy();
			maskCulledDrawIndirectInfoBuffers[i].destroy();
		}
		maskDrawIndirectBuffer.destroy();
		maskDrawIndirectInfoBuffer.destroy();
	}
	if (gotBlendPrimitives) {
		for (uint32_t i = 0; i < framesInFlight; i++) {
			blendCulledDrawIndirectBuffers[i].destroy();
			blendCulledDrawIndirectInfoBuffers[i].destroy();
		}
		blendDrawIndirectBuffer.destroy();
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

void Model::drawOpaque(CommandBuffer* commandBuffer, GraphicsPipeline* graphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling) {
	if (bindTextures) {
		materialsDescriptorSet.bind(commandBuffer, graphicsPipeline, 1);
		DescriptorSet* selectedDescriptorSet = culling ? &opaqueCulledDrawIndirectInfoDescriptorSets[frameInFlightIndex] : &opaqueDrawIndirectInfoDescriptorSet;
		selectedDescriptorSet->bind(commandBuffer, graphicsPipeline, 2);
	}
	VkBuffer selectedBuffer = culling ? opaqueCulledDrawIndirectBuffers[frameInFlightIndex].buffer : opaqueDrawIndirectBuffer.buffer;
	uint32_t selectedDrawCount = culling ? opaqueCulledDrawCount : opaqueDrawCount;
	vkCmdDrawIndexedIndirect(commandBuffer->commandBuffer, selectedBuffer, 0, selectedDrawCount, sizeof(VkDrawIndexedIndirectCommand));
}

void Model::drawMask(CommandBuffer* commandBuffer, GraphicsPipeline* graphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling, uint32_t pushConstantOffset) {
	if (bindTextures) {
		materialsDescriptorSet.bind(commandBuffer, graphicsPipeline, 1);
		DescriptorSet* selectedDescriptorSet = culling ? &maskCulledDrawIndirectInfoDescriptorSets[frameInFlightIndex] : &maskDrawIndirectInfoDescriptorSet;
		selectedDescriptorSet->bind(commandBuffer, graphicsPipeline, 2);
	}
	VkBuffer selectedBuffer = culling ? maskCulledDrawIndirectBuffers[frameInFlightIndex].buffer : maskDrawIndirectBuffer.buffer;
	uint32_t selectedDrawCount = culling ? maskCulledDrawCount : maskDrawCount;
	vkCmdDrawIndexedIndirect(commandBuffer->commandBuffer, selectedBuffer, 0, selectedDrawCount, sizeof(VkDrawIndexedIndirectCommand));
}

void Model::drawBlend(CommandBuffer* commandBuffer, GraphicsPipeline* graphicsPipeline, bool bindTextures, uint32_t frameInFlightIndex, bool culling) {
	if (bindTextures) {
		materialsDescriptorSet.bind(commandBuffer, graphicsPipeline, 1);
		DescriptorSet* selectedDescriptorSet = culling ? &blendCulledDrawIndirectInfoDescriptorSets[frameInFlightIndex] : &blendDrawIndirectInfoDescriptorSet;
		selectedDescriptorSet->bind(commandBuffer, graphicsPipeline, 2);
	}
	VkBuffer selectedBuffer = culling ? blendCulledDrawIndirectBuffers[frameInFlightIndex].buffer : blendDrawIndirectBuffer.buffer;
	uint32_t selectedDrawCount = culling ? blendCulledDrawCount : blendDrawCount;
	vkCmdDrawIndexedIndirect(commandBuffer->commandBuffer, selectedBuffer, 0, selectedDrawCount, sizeof(VkDrawIndexedIndirectCommand));
}