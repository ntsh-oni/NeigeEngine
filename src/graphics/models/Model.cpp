#include "Model.h"
#include "../resources/RendererResources.h"
#include "../resources/ShaderResources.h"

void Model::init(std::string filePath) {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	ModelLoader::load(filePath, &vertices, &indices, &meshes);

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
		mesh.boneBuffers.resize(framesInFlight);

		for (Buffer& buffer : mesh.boneBuffers) {
			NEIGE_ASSERT(mesh.boneList.size() <= MAX_BONES, "A mesh has more than " + std::to_string(MAX_BONES) + " bones.");

			BufferTools::createUniformBuffer(buffer.buffer, sizeof(BoneUniformBufferObject), &buffer.memoryInfo);

			BoneUniformBufferObject bubo = {};
			for (size_t i = 0; i < mesh.boneList.size(); i++) {
				bubo.transformations[i] = mesh.boneList[i].transformation;
				bubo.inverseBindMatrices[i] = mesh.boneList[i].inverseBindMatrix;
			}

			void* data;
			buffer.map(0, sizeof(BoneUniformBufferObject), &data);
			memcpy(data, &bubo, sizeof(BoneUniformBufferObject));
			buffer.unmap();
		}
	}
}

void Model::destroy() {
	vertexBuffer.destroy();
	indexBuffer.destroy();
	for (Mesh& mesh : meshes) {
		for (Buffer& buffer : mesh.boneBuffers) {
			buffer.destroy();
		}
	}
}

void Model::bindBuffers(CommandBuffer* commandBuffer) {
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(commandBuffer->commandBuffer, 0, 1, &vertexBuffer.buffer, &offset);
	vkCmdBindIndexBuffer(commandBuffer->commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
}

void Model::drawOpaque(CommandBuffer* commandBuffer, GraphicsPipeline* graphicsPipeline, uint32_t frameInFlightIndex, bool bindTextures) {
	for (Mesh& mesh : meshes) {
		for (size_t i = 0; i < mesh.opaquePrimitives.size(); i++) {
			if (bindTextures) {
				mesh.descriptorSets.at(graphicsPipeline).at(i).at(frameInFlightIndex).bind(commandBuffer, 1);
			}
			vkCmdDrawIndexed(commandBuffer->commandBuffer, mesh.opaquePrimitives[i].indexCount, 1, mesh.indexOffset + mesh.opaquePrimitives[i].firstIndex, mesh.vertexOffset + mesh.opaquePrimitives[i].vertexOffset, 0);
		}
	}
}

void Model::drawMask(CommandBuffer* commandBuffer, GraphicsPipeline* graphicsPipeline, uint32_t frameInFlightIndex, bool bindTextures, uint32_t pushConstantOffset) {
	for (Mesh& mesh : meshes) {
		for (size_t i = 0; i < mesh.maskPrimitives.size(); i++) {
			if (bindTextures) {
				mesh.descriptorSets.at(graphicsPipeline).at(i).at(frameInFlightIndex).bind(commandBuffer, 1);
			}
			graphicsPipeline->pushConstant(commandBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, pushConstantOffset, sizeof(float), &mesh.alphaCutoffs[i]);
			vkCmdDrawIndexed(commandBuffer->commandBuffer, mesh.maskPrimitives[i].indexCount, 1, mesh.indexOffset + mesh.maskPrimitives[i].firstIndex, mesh.vertexOffset + mesh.maskPrimitives[i].vertexOffset, 0);
		}
	}
}

void Model::drawBlend(CommandBuffer* commandBuffer, GraphicsPipeline* graphicsPipeline, uint32_t frameInFlightIndex, bool bindTextures) {
	for (Mesh& mesh : meshes) {
		for (size_t i = 0; i < mesh.blendPrimitives.size(); i++) {
			if (bindTextures) {
				mesh.descriptorSets.at(graphicsPipeline).at(i).at(frameInFlightIndex).bind(commandBuffer, 1);
			}
			vkCmdDrawIndexed(commandBuffer->commandBuffer, mesh.blendPrimitives[i].indexCount, 1, mesh.indexOffset + mesh.blendPrimitives[i].firstIndex, mesh.vertexOffset + mesh.blendPrimitives[i].vertexOffset, 0);
		}
	}
}

void Model::createOpaqueDescriptorSets(GraphicsPipeline* opaqueGraphicsPipeline) {
	for (Mesh& mesh : meshes) {
		std::vector<std::vector<DescriptorSet>> descriptorSets;
		descriptorSets.resize(mesh.opaquePrimitives.size());
		for (size_t i = 0; i < mesh.opaquePrimitives.size(); i++) {
			descriptorSets.at(i).resize(framesInFlight);
			for (uint32_t j = 0; j < framesInFlight; j++) {
				descriptorSets.at(i).at(j).init(opaqueGraphicsPipeline, 1);

				std::vector<VkWriteDescriptorSet> writesDescriptorSet;
				VkDescriptorImageInfo diffuseInfo = {};
				VkDescriptorImageInfo normalInfo = {};
				VkDescriptorImageInfo metallicRoughnessInfo = {};
				VkDescriptorImageInfo emissiveInfo = {};
				VkDescriptorImageInfo occlusionInfo = {};
				VkDescriptorBufferInfo bonesInfo = {};

				for (size_t k = 0; k < opaqueGraphicsPipeline->sets[1].bindings.size(); k++) {
					std::string bindingName = opaqueGraphicsPipeline->sets[1].bindings[k].name;
					if (bindingName == "colorMap") {
						std::string diffuseKey = materials[mesh.opaquePrimitives[i].materialIndex].diffuseKey;
						if (diffuseKey == "") {
							diffuseKey = "defaultDiffuse";
						}

						diffuseInfo.sampler = textures.at(diffuseKey).imageSampler;
						diffuseInfo.imageView = textures.at(diffuseKey).imageView;
						diffuseInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet diffuseWriteDescriptorSet = {};
						diffuseWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						diffuseWriteDescriptorSet.pNext = nullptr;
						diffuseWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						diffuseWriteDescriptorSet.dstBinding = opaqueGraphicsPipeline->sets[1].bindings[k].binding.binding;
						diffuseWriteDescriptorSet.dstArrayElement = 0;
						diffuseWriteDescriptorSet.descriptorCount = 1;
						diffuseWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						diffuseWriteDescriptorSet.pImageInfo = &diffuseInfo;
						diffuseWriteDescriptorSet.pBufferInfo = nullptr;
						diffuseWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(diffuseWriteDescriptorSet);
					}
					else if (bindingName == "normalMap") {
						std::string normalKey = materials[mesh.opaquePrimitives[i].materialIndex].normalKey;
						if (normalKey == "") {
							normalKey = "defaultNormal";
						}

						normalInfo.sampler = textures.at(normalKey).imageSampler;
						normalInfo.imageView = textures.at(normalKey).imageView;
						normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet normalWriteDescriptorSet = {};
						normalWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						normalWriteDescriptorSet.pNext = nullptr;
						normalWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						normalWriteDescriptorSet.dstBinding = opaqueGraphicsPipeline->sets[1].bindings[k].binding.binding;
						normalWriteDescriptorSet.dstArrayElement = 0;
						normalWriteDescriptorSet.descriptorCount = 1;
						normalWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						normalWriteDescriptorSet.pImageInfo = &normalInfo;
						normalWriteDescriptorSet.pBufferInfo = nullptr;
						normalWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(normalWriteDescriptorSet);
					}
					else if (bindingName == "metallicRoughnessMap") {
						std::string metallicRoughnessKey = materials[mesh.opaquePrimitives[i].materialIndex].metallicRoughnessKey;
						if (metallicRoughnessKey == "") {
							metallicRoughnessKey = "defaultMetallicRoughness";
						}
						
						metallicRoughnessInfo.sampler = textures.at(metallicRoughnessKey).imageSampler;
						metallicRoughnessInfo.imageView = textures.at(metallicRoughnessKey).imageView;
						metallicRoughnessInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet metallicRoughnessWriteDescriptorSet = {};
						metallicRoughnessWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						metallicRoughnessWriteDescriptorSet.pNext = nullptr;
						metallicRoughnessWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						metallicRoughnessWriteDescriptorSet.dstBinding = opaqueGraphicsPipeline->sets[1].bindings[k].binding.binding;
						metallicRoughnessWriteDescriptorSet.dstArrayElement = 0;
						metallicRoughnessWriteDescriptorSet.descriptorCount = 1;
						metallicRoughnessWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						metallicRoughnessWriteDescriptorSet.pImageInfo = &metallicRoughnessInfo;
						metallicRoughnessWriteDescriptorSet.pBufferInfo = nullptr;
						metallicRoughnessWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(metallicRoughnessWriteDescriptorSet);
					}
					else if (bindingName == "emissiveMap") {
						std::string emissiveKey = materials[mesh.opaquePrimitives[i].materialIndex].emissiveKey;
						if (emissiveKey == "") {
							emissiveKey = "defaultEmissive";
						}
						
						emissiveInfo.sampler = textures.at(emissiveKey).imageSampler;
						emissiveInfo.imageView = textures.at(emissiveKey).imageView;
						emissiveInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet emissiveWriteDescriptorSet = {};
						emissiveWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						emissiveWriteDescriptorSet.pNext = nullptr;
						emissiveWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						emissiveWriteDescriptorSet.dstBinding = opaqueGraphicsPipeline->sets[1].bindings[k].binding.binding;
						emissiveWriteDescriptorSet.dstArrayElement = 0;
						emissiveWriteDescriptorSet.descriptorCount = 1;
						emissiveWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						emissiveWriteDescriptorSet.pImageInfo = &emissiveInfo;
						emissiveWriteDescriptorSet.pBufferInfo = nullptr;
						emissiveWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(emissiveWriteDescriptorSet);
					}
					else if (bindingName == "occlusionMap") {
						std::string occlusionKey = materials[mesh.opaquePrimitives[i].materialIndex].occlusionKey;
						if (occlusionKey == "") {
							occlusionKey = "defaultOcclusion";
						}

						occlusionInfo.sampler = textures.at(occlusionKey).imageSampler;
						occlusionInfo.imageView = textures.at(occlusionKey).imageView;
						occlusionInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet occlusionWriteDescriptorSet = {};
						occlusionWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						occlusionWriteDescriptorSet.pNext = nullptr;
						occlusionWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						occlusionWriteDescriptorSet.dstBinding = opaqueGraphicsPipeline->sets[1].bindings[k].binding.binding;
						occlusionWriteDescriptorSet.dstArrayElement = 0;
						occlusionWriteDescriptorSet.descriptorCount = 1;
						occlusionWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						occlusionWriteDescriptorSet.pImageInfo = &occlusionInfo;
						occlusionWriteDescriptorSet.pBufferInfo = nullptr;
						occlusionWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(occlusionWriteDescriptorSet);
					}
					else if (bindingName == "bones") {
						bonesInfo.buffer = mesh.boneBuffers.at(j).buffer;
						bonesInfo.offset = 0;
						bonesInfo.range = sizeof(BoneUniformBufferObject);

						VkWriteDescriptorSet bonesWriteDescriptorSet = {};
						bonesWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						bonesWriteDescriptorSet.pNext = nullptr;
						bonesWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						bonesWriteDescriptorSet.dstBinding = opaqueGraphicsPipeline->sets[1].bindings[k].binding.binding;
						bonesWriteDescriptorSet.dstArrayElement = 0;
						bonesWriteDescriptorSet.descriptorCount = 1;
						bonesWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						bonesWriteDescriptorSet.pImageInfo = nullptr;
						bonesWriteDescriptorSet.pBufferInfo = &bonesInfo;
						bonesWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(bonesWriteDescriptorSet);
					}
				}

				descriptorSets.at(i).at(j).update(writesDescriptorSet);
			}
		}
		mesh.descriptorSets.emplace(opaqueGraphicsPipeline, descriptorSets);
	}
}

void Model::createMaskDescriptorSets(GraphicsPipeline* maskGraphicsPipeline) {
	for (Mesh& mesh : meshes) {
		std::vector<std::vector<DescriptorSet>> descriptorSets;
		descriptorSets.resize(mesh.maskPrimitives.size());
		for (size_t i = 0; i < mesh.maskPrimitives.size(); i++) {
			descriptorSets.at(i).resize(framesInFlight);
			for (uint32_t j = 0; j < framesInFlight; j++) {
				descriptorSets.at(i).at(j).init(maskGraphicsPipeline, 1);

				std::vector<VkWriteDescriptorSet> writesDescriptorSet;
				VkDescriptorImageInfo diffuseInfo = {};
				VkDescriptorImageInfo normalInfo = {};
				VkDescriptorImageInfo metallicRoughnessInfo = {};
				VkDescriptorImageInfo emissiveInfo = {};
				VkDescriptorImageInfo occlusionInfo = {};
				VkDescriptorBufferInfo bonesInfo = {};

				for (size_t k = 0; k < maskGraphicsPipeline->sets[1].bindings.size(); k++) {
					std::string bindingName = maskGraphicsPipeline->sets[1].bindings[k].name;
					if (bindingName == "colorMap") {
						std::string diffuseKey = materials[mesh.maskPrimitives[i].materialIndex].diffuseKey;
						if (diffuseKey == "") {
							diffuseKey = "defaultDiffuse";
						}

						diffuseInfo.sampler = textures.at(diffuseKey).imageSampler;
						diffuseInfo.imageView = textures.at(diffuseKey).imageView;
						diffuseInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet diffuseWriteDescriptorSet = {};
						diffuseWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						diffuseWriteDescriptorSet.pNext = nullptr;
						diffuseWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						diffuseWriteDescriptorSet.dstBinding = maskGraphicsPipeline->sets[1].bindings[k].binding.binding;
						diffuseWriteDescriptorSet.dstArrayElement = 0;
						diffuseWriteDescriptorSet.descriptorCount = 1;
						diffuseWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						diffuseWriteDescriptorSet.pImageInfo = &diffuseInfo;
						diffuseWriteDescriptorSet.pBufferInfo = nullptr;
						diffuseWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(diffuseWriteDescriptorSet);
					}
					else if (bindingName == "normalMap") {
						std::string normalKey = materials[mesh.maskPrimitives[i].materialIndex].normalKey;
						if (normalKey == "") {
							normalKey = "defaultNormal";
						}

						normalInfo.sampler = textures.at(normalKey).imageSampler;
						normalInfo.imageView = textures.at(normalKey).imageView;
						normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet normalWriteDescriptorSet = {};
						normalWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						normalWriteDescriptorSet.pNext = nullptr;
						normalWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						normalWriteDescriptorSet.dstBinding = maskGraphicsPipeline->sets[1].bindings[k].binding.binding;
						normalWriteDescriptorSet.dstArrayElement = 0;
						normalWriteDescriptorSet.descriptorCount = 1;
						normalWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						normalWriteDescriptorSet.pImageInfo = &normalInfo;
						normalWriteDescriptorSet.pBufferInfo = nullptr;
						normalWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(normalWriteDescriptorSet);
					}
					else if (bindingName == "metallicRoughnessMap") {
						std::string metallicRoughnessKey = materials[mesh.maskPrimitives[i].materialIndex].metallicRoughnessKey;
						if (metallicRoughnessKey == "") {
							metallicRoughnessKey = "defaultMetallicRoughness";
						}

						metallicRoughnessInfo.sampler = textures.at(metallicRoughnessKey).imageSampler;
						metallicRoughnessInfo.imageView = textures.at(metallicRoughnessKey).imageView;
						metallicRoughnessInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet metallicRoughnessWriteDescriptorSet = {};
						metallicRoughnessWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						metallicRoughnessWriteDescriptorSet.pNext = nullptr;
						metallicRoughnessWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						metallicRoughnessWriteDescriptorSet.dstBinding = maskGraphicsPipeline->sets[1].bindings[k].binding.binding;
						metallicRoughnessWriteDescriptorSet.dstArrayElement = 0;
						metallicRoughnessWriteDescriptorSet.descriptorCount = 1;
						metallicRoughnessWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						metallicRoughnessWriteDescriptorSet.pImageInfo = &metallicRoughnessInfo;
						metallicRoughnessWriteDescriptorSet.pBufferInfo = nullptr;
						metallicRoughnessWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(metallicRoughnessWriteDescriptorSet);
					}
					else if (bindingName == "emissiveMap") {
						std::string emissiveKey = materials[mesh.maskPrimitives[i].materialIndex].emissiveKey;
						if (emissiveKey == "") {
							emissiveKey = "defaultEmissive";
						}

						emissiveInfo.sampler = textures.at(emissiveKey).imageSampler;
						emissiveInfo.imageView = textures.at(emissiveKey).imageView;
						emissiveInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet emissiveWriteDescriptorSet = {};
						emissiveWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						emissiveWriteDescriptorSet.pNext = nullptr;
						emissiveWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						emissiveWriteDescriptorSet.dstBinding = maskGraphicsPipeline->sets[1].bindings[k].binding.binding;
						emissiveWriteDescriptorSet.dstArrayElement = 0;
						emissiveWriteDescriptorSet.descriptorCount = 1;
						emissiveWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						emissiveWriteDescriptorSet.pImageInfo = &emissiveInfo;
						emissiveWriteDescriptorSet.pBufferInfo = nullptr;
						emissiveWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(emissiveWriteDescriptorSet);
					}
					else if (bindingName == "occlusionMap") {
						std::string occlusionKey = materials[mesh.maskPrimitives[i].materialIndex].occlusionKey;
						if (occlusionKey == "") {
							occlusionKey = "defaultOcclusion";
						}

						occlusionInfo.sampler = textures.at(occlusionKey).imageSampler;
						occlusionInfo.imageView = textures.at(occlusionKey).imageView;
						occlusionInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet occlusionWriteDescriptorSet = {};
						occlusionWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						occlusionWriteDescriptorSet.pNext = nullptr;
						occlusionWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						occlusionWriteDescriptorSet.dstBinding = maskGraphicsPipeline->sets[1].bindings[k].binding.binding;
						occlusionWriteDescriptorSet.dstArrayElement = 0;
						occlusionWriteDescriptorSet.descriptorCount = 1;
						occlusionWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						occlusionWriteDescriptorSet.pImageInfo = &occlusionInfo;
						occlusionWriteDescriptorSet.pBufferInfo = nullptr;
						occlusionWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(occlusionWriteDescriptorSet);
					}
					else if (bindingName == "bones") {
						bonesInfo.buffer = mesh.boneBuffers.at(j).buffer;
						bonesInfo.offset = 0;
						bonesInfo.range = sizeof(BoneUniformBufferObject);

						VkWriteDescriptorSet bonesWriteDescriptorSet = {};
						bonesWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						bonesWriteDescriptorSet.pNext = nullptr;
						bonesWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						bonesWriteDescriptorSet.dstBinding = maskGraphicsPipeline->sets[1].bindings[k].binding.binding;
						bonesWriteDescriptorSet.dstArrayElement = 0;
						bonesWriteDescriptorSet.descriptorCount = 1;
						bonesWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						bonesWriteDescriptorSet.pImageInfo = nullptr;
						bonesWriteDescriptorSet.pBufferInfo = &bonesInfo;
						bonesWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(bonesWriteDescriptorSet);
					}
				}

				descriptorSets.at(i).at(j).update(writesDescriptorSet);
			}
		}
		mesh.descriptorSets.emplace(maskGraphicsPipeline, descriptorSets);
	}
}

void Model::createDiscardMaskDescriptorSets(GraphicsPipeline* discardGraphicsPipeline) {
	for (Mesh& mesh : meshes) {
		std::vector<std::vector<DescriptorSet>> descriptorSets;
		descriptorSets.resize(mesh.maskPrimitives.size());
		for (size_t i = 0; i < mesh.maskPrimitives.size(); i++) {
			descriptorSets.at(i).resize(framesInFlight);
			for (uint32_t j = 0; j < framesInFlight; j++) {
				descriptorSets.at(i).at(j).init(discardGraphicsPipeline, 1);

				std::vector<VkWriteDescriptorSet> writesDescriptorSet;

				std::string diffuseKey = materials[mesh.maskPrimitives[i].materialIndex].diffuseKey;
				VkDescriptorImageInfo diffuseInfo = {};
				diffuseInfo.sampler = textures.at(diffuseKey).imageSampler;
				diffuseInfo.imageView = textures.at(diffuseKey).imageView;
				diffuseInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				VkWriteDescriptorSet diffuseWriteDescriptorSet = {};
				diffuseWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				diffuseWriteDescriptorSet.pNext = nullptr;
				diffuseWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
				diffuseWriteDescriptorSet.dstBinding = 0;
				diffuseWriteDescriptorSet.dstArrayElement = 0;
				diffuseWriteDescriptorSet.descriptorCount = 1;
				diffuseWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				diffuseWriteDescriptorSet.pImageInfo = &diffuseInfo;
				diffuseWriteDescriptorSet.pBufferInfo = nullptr;
				diffuseWriteDescriptorSet.pTexelBufferView = nullptr;
				writesDescriptorSet.push_back(diffuseWriteDescriptorSet);

				descriptorSets.at(i).at(j).update(writesDescriptorSet);
			}
		}
		mesh.descriptorSets.emplace(discardGraphicsPipeline, descriptorSets);
	}
}


void Model::createBlendDescriptorSets(GraphicsPipeline* blendGraphicsPipeline) {
	for (Mesh& mesh : meshes) {
		std::vector<std::vector<DescriptorSet>> descriptorSets;
		descriptorSets.resize(mesh.blendPrimitives.size());
		for (size_t i = 0; i < mesh.blendPrimitives.size(); i++) {
			descriptorSets.at(i).resize(framesInFlight);
			for (uint32_t j = 0; j < framesInFlight; j++) {
				descriptorSets.at(i).at(j).init(blendGraphicsPipeline, 1);

				std::vector<VkWriteDescriptorSet> writesDescriptorSet;
				VkDescriptorImageInfo diffuseInfo = {};
				VkDescriptorImageInfo normalInfo = {};
				VkDescriptorImageInfo metallicRoughnessInfo = {};
				VkDescriptorImageInfo emissiveInfo = {};
				VkDescriptorImageInfo occlusionInfo = {};
				VkDescriptorBufferInfo bonesInfo = {};

				for (size_t k = 0; k < blendGraphicsPipeline->sets[1].bindings.size(); k++) {
					std::string bindingName = blendGraphicsPipeline->sets[1].bindings[k].name;
					if (bindingName == "colorMap") {
						std::string diffuseKey = materials[mesh.blendPrimitives[i].materialIndex].diffuseKey;
						if (diffuseKey == "") {
							diffuseKey = "defaultDiffuse";
						}

						diffuseInfo.sampler = textures.at(diffuseKey).imageSampler;
						diffuseInfo.imageView = textures.at(diffuseKey).imageView;
						diffuseInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet diffuseWriteDescriptorSet = {};
						diffuseWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						diffuseWriteDescriptorSet.pNext = nullptr;
						diffuseWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						diffuseWriteDescriptorSet.dstBinding = blendGraphicsPipeline->sets[1].bindings[k].binding.binding;
						diffuseWriteDescriptorSet.dstArrayElement = 0;
						diffuseWriteDescriptorSet.descriptorCount = 1;
						diffuseWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						diffuseWriteDescriptorSet.pImageInfo = &diffuseInfo;
						diffuseWriteDescriptorSet.pBufferInfo = nullptr;
						diffuseWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(diffuseWriteDescriptorSet);
					}
					else if (bindingName == "normalMap") {
						std::string normalKey = materials[mesh.blendPrimitives[i].materialIndex].normalKey;
						if (normalKey == "") {
							normalKey = "defaultNormal";
						}

						normalInfo.sampler = textures.at(normalKey).imageSampler;
						normalInfo.imageView = textures.at(normalKey).imageView;
						normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet normalWriteDescriptorSet = {};
						normalWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						normalWriteDescriptorSet.pNext = nullptr;
						normalWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						normalWriteDescriptorSet.dstBinding = blendGraphicsPipeline->sets[1].bindings[k].binding.binding;
						normalWriteDescriptorSet.dstArrayElement = 0;
						normalWriteDescriptorSet.descriptorCount = 1;
						normalWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						normalWriteDescriptorSet.pImageInfo = &normalInfo;
						normalWriteDescriptorSet.pBufferInfo = nullptr;
						normalWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(normalWriteDescriptorSet);
					}
					else if (bindingName == "metallicRoughnessMap") {
						std::string metallicRoughnessKey = materials[mesh.blendPrimitives[i].materialIndex].metallicRoughnessKey;
						if (metallicRoughnessKey == "") {
							metallicRoughnessKey = "defaultMetallicRoughness";
						}

						metallicRoughnessInfo.sampler = textures.at(metallicRoughnessKey).imageSampler;
						metallicRoughnessInfo.imageView = textures.at(metallicRoughnessKey).imageView;
						metallicRoughnessInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet metallicRoughnessWriteDescriptorSet = {};
						metallicRoughnessWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						metallicRoughnessWriteDescriptorSet.pNext = nullptr;
						metallicRoughnessWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						metallicRoughnessWriteDescriptorSet.dstBinding = blendGraphicsPipeline->sets[1].bindings[k].binding.binding;
						metallicRoughnessWriteDescriptorSet.dstArrayElement = 0;
						metallicRoughnessWriteDescriptorSet.descriptorCount = 1;
						metallicRoughnessWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						metallicRoughnessWriteDescriptorSet.pImageInfo = &metallicRoughnessInfo;
						metallicRoughnessWriteDescriptorSet.pBufferInfo = nullptr;
						metallicRoughnessWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(metallicRoughnessWriteDescriptorSet);
					}
					else if (bindingName == "emissiveMap") {
						std::string emissiveKey = materials[mesh.blendPrimitives[i].materialIndex].emissiveKey;
						if (emissiveKey == "") {
							emissiveKey = "defaultEmissive";
						}

						emissiveInfo.sampler = textures.at(emissiveKey).imageSampler;
						emissiveInfo.imageView = textures.at(emissiveKey).imageView;
						emissiveInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet emissiveWriteDescriptorSet = {};
						emissiveWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						emissiveWriteDescriptorSet.pNext = nullptr;
						emissiveWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						emissiveWriteDescriptorSet.dstBinding = blendGraphicsPipeline->sets[1].bindings[k].binding.binding;
						emissiveWriteDescriptorSet.dstArrayElement = 0;
						emissiveWriteDescriptorSet.descriptorCount = 1;
						emissiveWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						emissiveWriteDescriptorSet.pImageInfo = &emissiveInfo;
						emissiveWriteDescriptorSet.pBufferInfo = nullptr;
						emissiveWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(emissiveWriteDescriptorSet);
					}
					else if (bindingName == "occlusionMap") {
						std::string occlusionKey = materials[mesh.blendPrimitives[i].materialIndex].occlusionKey;
						if (occlusionKey == "") {
							occlusionKey = "defaultOcclusion";
						}

						occlusionInfo.sampler = textures.at(occlusionKey).imageSampler;
						occlusionInfo.imageView = textures.at(occlusionKey).imageView;
						occlusionInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

						VkWriteDescriptorSet occlusionWriteDescriptorSet = {};
						occlusionWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						occlusionWriteDescriptorSet.pNext = nullptr;
						occlusionWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						occlusionWriteDescriptorSet.dstBinding = blendGraphicsPipeline->sets[1].bindings[k].binding.binding;
						occlusionWriteDescriptorSet.dstArrayElement = 0;
						occlusionWriteDescriptorSet.descriptorCount = 1;
						occlusionWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						occlusionWriteDescriptorSet.pImageInfo = &occlusionInfo;
						occlusionWriteDescriptorSet.pBufferInfo = nullptr;
						occlusionWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(occlusionWriteDescriptorSet);
					}
					else if (bindingName == "bones") {
						bonesInfo.buffer = mesh.boneBuffers.at(j).buffer;
						bonesInfo.offset = 0;
						bonesInfo.range = sizeof(BoneUniformBufferObject);

						VkWriteDescriptorSet bonesWriteDescriptorSet = {};
						bonesWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						bonesWriteDescriptorSet.pNext = nullptr;
						bonesWriteDescriptorSet.dstSet = descriptorSets.at(i).at(j).descriptorSet;
						bonesWriteDescriptorSet.dstBinding = blendGraphicsPipeline->sets[1].bindings[k].binding.binding;
						bonesWriteDescriptorSet.dstArrayElement = 0;
						bonesWriteDescriptorSet.descriptorCount = 1;
						bonesWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						bonesWriteDescriptorSet.pImageInfo = nullptr;
						bonesWriteDescriptorSet.pBufferInfo = &bonesInfo;
						bonesWriteDescriptorSet.pTexelBufferView = nullptr;
						writesDescriptorSet.push_back(bonesWriteDescriptorSet);
					}
				}

				descriptorSets.at(i).at(j).update(writesDescriptorSet);
			}
		}
		mesh.descriptorSets.emplace(blendGraphicsPipeline, descriptorSets);
	}
}