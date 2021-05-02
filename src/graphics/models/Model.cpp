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

		// AABB
		// Mesh AABB
		mesh.aabb = { glm::vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()), glm::vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()) };

		for (Primitive& primitive : mesh.opaquePrimitives) {
			if (primitive.aabb.min.x < mesh.aabb.min.x) {
				mesh.aabb.min.x = primitive.aabb.min.x;
			}
			if (primitive.aabb.max.x > mesh.aabb.max.x) {
				mesh.aabb.max.x = primitive.aabb.max.x;
			}
			if (primitive.aabb.min.y < mesh.aabb.min.y) {
				mesh.aabb.min.y = primitive.aabb.min.y;
			}
			if (primitive.aabb.max.y > mesh.aabb.max.y) {
				mesh.aabb.max.y = primitive.aabb.max.y;
			}
			if (primitive.aabb.min.z < mesh.aabb.min.z) {
				mesh.aabb.min.z = primitive.aabb.min.z;
			}
			if (primitive.aabb.max.z > mesh.aabb.max.z) {
				mesh.aabb.max.z = primitive.aabb.max.z;
			}
		}
		for (Primitive& primitive : mesh.maskPrimitives) {
			if (primitive.aabb.min.x < mesh.aabb.min.x) {
				mesh.aabb.min.x = primitive.aabb.min.x;
			}
			if (primitive.aabb.max.x > mesh.aabb.max.x) {
				mesh.aabb.max.x = primitive.aabb.max.x;
			}
			if (primitive.aabb.min.y < mesh.aabb.min.y) {
				mesh.aabb.min.y = primitive.aabb.min.y;
			}
			if (primitive.aabb.max.y > mesh.aabb.max.y) {
				mesh.aabb.max.y = primitive.aabb.max.y;
			}
			if (primitive.aabb.min.z < mesh.aabb.min.z) {
				mesh.aabb.min.z = primitive.aabb.min.z;
			}
			if (primitive.aabb.max.z > mesh.aabb.max.z) {
				mesh.aabb.max.z = primitive.aabb.max.z;
			}
		}
		for (Primitive& primitive : mesh.blendPrimitives) {
			if (primitive.aabb.min.x < mesh.aabb.min.x) {
				mesh.aabb.min.x = primitive.aabb.min.x;
			}
			if (primitive.aabb.max.x > mesh.aabb.max.x) {
				mesh.aabb.max.x = primitive.aabb.max.x;
			}
			if (primitive.aabb.min.y < mesh.aabb.min.y) {
				mesh.aabb.min.y = primitive.aabb.min.y;
			}
			if (primitive.aabb.max.y > mesh.aabb.max.y) {
				mesh.aabb.max.y = primitive.aabb.max.y;
			}
			if (primitive.aabb.min.z < mesh.aabb.min.z) {
				mesh.aabb.min.z = primitive.aabb.min.z;
			}
			if (primitive.aabb.max.z > mesh.aabb.max.z) {
				mesh.aabb.max.z = primitive.aabb.max.z;
			}
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