#include "StaticMesh.hpp"

namespace engine
{
    StaticMesh::StaticMesh(const std::string &path) : filepath(path)
    {
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            throw std::runtime_error("ERROR::ASSIMP::" + std::string(import.GetErrorString()));
        }

        this->processNode(scene->mRootNode, scene);
    }
}