#pragma once

#include "context.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace engine
{
    struct Mesh
    {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec4> colors;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> normals;
        std::vector<glm::uint> indices;
    };

    class StaticMesh
    {
    public:
        StaticMesh(const std::string &path);
        ~StaticMesh() = default;

    private:
        std::string filepath;
        std::vector<Mesh> meshes;

        void processNode(aiNode *node, const aiScene *scene)
        {
            // 添加当前节点中的所有Mesh
            for (uint32_t i = 0; i < node->mNumMeshes; i++)
            {
                aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
                this->meshes.push_back(this->processMesh(mesh, scene));
            }
            // 递归处理该节点的子孙节点
            for (uint32_t i = 0; i < node->mNumChildren; i++)
            {
                this->processNode(node->mChildren[i], scene);
            }
        }

        Mesh processMesh(aiMesh *mesh, const aiScene *scene)
        {
            // mtl
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            aiColor3D color;
            material->Get(AI_MATKEY_COLOR_AMBIENT, color);

            Mesh result;

            for (uint32_t i = 0; i < mesh->mNumVertices; i++)
            {
                result.positions.push_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
                if (mesh->mColors[0])
                {
                    result.colors.push_back(glm::vec4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a));
                }
                else
                {
                    result.colors.push_back(glm::vec4(color.r, color.g, color.b, 1.0f));
                }
                if (mesh->mTextureCoords[0])
                {
                    result.texCoords.push_back(glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
                }
                else
                {
                    result.texCoords.push_back(glm::vec2(0.0f, 0.0f));
                }
                if (mesh->mNormals)
                {
                    result.normals.push_back(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
                }
                else
                {
                    result.normals.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
                }
            }
            for (uint32_t i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                for (uint32_t j = 0; j < face.mNumIndices; j++)
                {
                    result.indices.push_back(face.mIndices[j]);
                }
            }

            return result;
        }

    public:
        std::vector<Vertex> get_one_vertices()
        {
            std::vector<Vertex> vertices;
            for (auto &mesh : this->meshes)
            {
                for (int i = 0; i < mesh.positions.size(); i++)
                {
                    Vertex vertex;
                    vertex.position = mesh.positions[i];
                    vertex.color = mesh.colors[i];
                    vertex.texCoord = mesh.texCoords[i];
                    vertices.push_back(vertex);
                }
            }

            return vertices;
        }

        std::vector<uint32_t> get_one_indices()
        {
            std::vector<uint32_t> indices;
            uint32_t offset = 0;
            for (auto &mesh : this->meshes)
            {
                for (auto &index : mesh.indices)
                {
                    indices.push_back(index + offset);
                }
                offset += mesh.positions.size();
            }

            return indices;
        }
    };
}