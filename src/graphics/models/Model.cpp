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
		}
		if (mesh.maskPrimitives.size() != 0) {
			gotMaskPrimitives = true;
		}
		if (mesh.blendPrimitives.size() != 0) {
			gotBlendPrimitives = true;
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

	for (Mesh& mesh : meshes) {
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
	}
}

void Model::destroy() {
	vertexBuffer.destroy();
	indexBuffer.destroy();
	for (Mesh& mesh : meshes) {
		mesh.boneBuffer.destroy();
	}
}

void Model::bindBuffers(CommandBuffer* commandBuffer) {
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(commandBuffer->commandBuffer, 0, 1, &vertexBuffer.buffer, &offset);
	vkCmdBindIndexBuffer(commandBuffer->commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
}

void Model::drawOpaque(CommandBuffer* commandBuffer, GraphicsPipeline* graphicsPipeline, bool bindTextures) {
	if (bindTextures) {
		materialsDescriptorSet.bind(commandBuffer, graphicsPipeline, 1);
	}
	for (Mesh& mesh : meshes) {
		for (size_t i = 0; i < mesh.opaquePrimitives.size(); i++) {
			if (bindTextures) {
				int materialIndices[5] = { materials.at(mesh.opaquePrimitives[i].materialIndex).diffuseIndex, materials.at(mesh.opaquePrimitives[i].materialIndex).normalIndex, materials.at(mesh.opaquePrimitives[i].materialIndex).metallicRoughnessIndex, materials.at(mesh.opaquePrimitives[i].materialIndex).emissiveIndex, materials.at(mesh.opaquePrimitives[i].materialIndex).occlusionIndex };
				graphicsPipeline->pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 5 * sizeof(int), &materialIndices);
			}
			vkCmdDrawIndexed(commandBuffer->commandBuffer, mesh.opaquePrimitives[i].indexCount, 1, mesh.indexOffset + mesh.opaquePrimitives[i].firstIndex, mesh.vertexOffset + mesh.opaquePrimitives[i].vertexOffset, 0);
		}
	}
}

void Model::drawMask(CommandBuffer* commandBuffer, GraphicsPipeline* graphicsPipeline, bool bindTextures, uint32_t pushConstantOffset) {
	if (bindTextures) {
		materialsDescriptorSet.bind(commandBuffer, graphicsPipeline, 1);
	}
	for (Mesh& mesh : meshes) {
		for (size_t i = 0; i < mesh.maskPrimitives.size(); i++) {
			if (bindTextures) {
				int materialIndices[5] = { materials.at(mesh.maskPrimitives[i].materialIndex).diffuseIndex, materials.at(mesh.maskPrimitives[i].materialIndex).normalIndex, materials.at(mesh.maskPrimitives[i].materialIndex).metallicRoughnessIndex, materials.at(mesh.maskPrimitives[i].materialIndex).emissiveIndex, materials.at(mesh.maskPrimitives[i].materialIndex).occlusionIndex };
				graphicsPipeline->pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, pushConstantOffset, 5 * sizeof(int), &materialIndices);
				graphicsPipeline->pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, pushConstantOffset + (5 * sizeof(int)), sizeof(float), &mesh.alphaCutoffs[i]);
			}
			else {
				graphicsPipeline->pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, pushConstantOffset, sizeof(float), &mesh.alphaCutoffs[i]);
			}
			vkCmdDrawIndexed(commandBuffer->commandBuffer, mesh.maskPrimitives[i].indexCount, 1, mesh.indexOffset + mesh.maskPrimitives[i].firstIndex, mesh.vertexOffset + mesh.maskPrimitives[i].vertexOffset, 0);
		}
	}
}

void Model::drawBlend(CommandBuffer* commandBuffer, GraphicsPipeline* graphicsPipeline, bool bindTextures) {
	if (bindTextures) {
		materialsDescriptorSet.bind(commandBuffer, graphicsPipeline, 1);
	}
	for (Mesh& mesh : meshes) {
		for (size_t i = 0; i < mesh.blendPrimitives.size(); i++) {
			if (bindTextures) {
				int materialIndices[5] = { materials.at(mesh.blendPrimitives[i].materialIndex).diffuseIndex, materials.at(mesh.blendPrimitives[i].materialIndex).normalIndex, materials.at(mesh.blendPrimitives[i].materialIndex).metallicRoughnessIndex, materials.at(mesh.blendPrimitives[i].materialIndex).emissiveIndex, materials.at(mesh.blendPrimitives[i].materialIndex).occlusionIndex };
				graphicsPipeline->pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 5 * sizeof(int), &materialIndices);
			}
			vkCmdDrawIndexed(commandBuffer->commandBuffer, mesh.blendPrimitives[i].indexCount, 1, mesh.indexOffset + mesh.blendPrimitives[i].firstIndex, mesh.vertexOffset + mesh.blendPrimitives[i].vertexOffset, 0);
		}
	}
}