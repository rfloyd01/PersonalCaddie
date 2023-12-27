#include "pch.h"

#include <iostream>
#include <locale>
#include <codecvt>
#include <string>

#include "Model.h"
#include <Math/quaternion_functions.h>

using namespace DirectX;

//PUBLIC FUNCTIONS
//Constructors
Model::Model()
{
    //Set the scale to be 1x1x1
    m_scale = { 1.0f, 1.0f, 1.0f };
}

//Setup Functions
void Model::loadModel(std::string path)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
        aiProcess_ConvertToLeftHanded | aiProcess_PreTransformVertices); 

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring message = converter.from_bytes(import.GetErrorString());
        std::wstring err = L"ERROR::ASSIMP::" + message;
        OutputDebugString(&err[0]);
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);

    //after all nodes are processed calculate the hit box for the model
    //setBoundingBox();
}
//void Model::setScale(glm::vec3 s)
//{
//    model_scale = s;
//}
void Model::setPosition(DirectX::XMFLOAT3 position)
{
    m_position = position;
}
//void Model::setRotation(glm::quat r)
//{
//    model_rotation = r;
//}
//
////Rendering Functions
//glm::vec3 Model::getScale()
//{
//    return model_scale;
//}
//glm::vec3 Model::getLocation()
//{
//    return model_location;
//}
//glm::quat Model::getRotation()
//{
//    return model_rotation;
//}
//void Model::Draw(Shader& shader)
//{
//    for (unsigned int i = 0; i < meshes.size(); i++)
//        meshes[i].Draw(shader);
//}
//
////Get Functions
//std::vector<glm::vec3> Model::getBoundingBox()
//{
//    std::vector<glm::vec3> bb;
//    bb.push_back(transformVertex(min_coordinates));  bb.push_back(transformVertex(max_coordinates));
//    return bb;
//}
//
//PRIVATE FUNCTIONS
//Data Processing Functions
void Model::processNode(aiNode* node, const aiScene* scene)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMeshVectors(mesh, scene);
        //addMesh(std::make_shared<MeshObject>(processMesh(mesh, scene)));
        //m_meshes.push_back(processMesh(mesh, scene));
    }

    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

void Model::processMeshVectors(aiMesh* mesh, const aiScene* scene)
{
    std::vector<PNTVertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<DirectX::XMFLOAT4> colors;
    //std::vector<Texture> textures;

    //NOTE:
    //ASSIMP makes multiple copies of each vertex so that choosing the indices to make
    //each triangle is easier. Normally (in my experience) you only have one of each 
    //vertex and set the indices so that multiple vertices get rendered each time. As an
    //example if we wanted to render a square with vertices A, B, C, and D at [0, 0], [0, 1],
    //[1, 0] and [1, 1] respectively then we'd set up the indices vector like so (A, B, C, B, C, D)
    //which would in turn render two triangles to make the square. It seems that what ASSIMP 
    //does though is create the vertices A, B, C, D, E, F as [0, 0], [0, 1], [1, 0], [0, 1], [1, 0], [1, 1]
    //respectively and sets the index vector simply as (A, B, C, D, E, F). Because of this
    //the number of vertices is equal to the number of indices.


    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        PNTVertex vertex;
        // process vertex positions, normals and texture coordinates
        DirectX::XMFLOAT3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            DirectX::XMFLOAT2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.textureCoordinate = vec;
        }
        else vertex.textureCoordinate = { 0.0f, 0.0f };
        vertices.push_back(vertex);
    }
    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // process material
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiColor3D ambientColor(0.0f, 0.0f, 0.0f);
        aiColor3D diffuseColor(0.0f, 0.0f, 0.0f);
        aiColor3D specularColor(0.0f, 0.0f, 0.0f);

        material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
        colors.push_back(DirectX::XMFLOAT4(ambientColor.r, ambientColor.g, ambientColor.b, 1.0f));
        colors.push_back(DirectX::XMFLOAT4(diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.0f));
        colors.push_back(DirectX::XMFLOAT4(specularColor.r, specularColor.g, specularColor.b, 1.0f));

        //loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        //loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    }

    m_vertices.push_back(vertices);
    m_indices.push_back(indices);
    m_colors.push_back(colors);

    //For now just use default textures for each mesh until I figure out how
    //to read the .jpg files stored in the material object
    m_textures.push_back(MaterialType::DEFAULT);
}

void Model::translateAndRotateFace(DirectX::XMFLOAT3 location, DirectX::XMVECTOR quat)
{
    //Order of operations is scale, rotate then translate
    XMStoreFloat4x4(
        &m_modelMatrix,
        XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) *
        XMMatrixRotationQuaternion(quat) *
        XMMatrixTranslation(location.x, location.y, location.z)
    );
}

void Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    //for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    //{
    //    aiString str;
    //    mat->GetTexture(type, i, &str);
    //    bool skip = false;
    //    for (unsigned int j = 0; j < m_textures.size(); j++)
    //    {
    //        //Make sure we don't load the same texture multiple times
    //        if (std::strcmp(m_textures[j].path.data(), str.C_Str()) == 0)
    //        {
    //            skip = true;
    //            break;
    //        }
    //    }
    //    if (!skip)
    //    {   // if texture hasn't been loaded already, load it
    //        Texture texture;
    //        //texture.id = TextureFromFile(str.C_Str(), directory);
    //        texture.type = typeName;
    //        texture.path = str.C_Str();
    //        m_textures.push_back(texture);
    //    }
    //}
}

//unsigned int Model::TextureFromFile(const char* path, const std::string& directory)
//{
//    std::string filename = std::string(path);
//    filename = directory + '/' + filename;
//
//    unsigned int textureID;
//    glGenTextures(1, &textureID);
//
//    int width, height, nrComponents;
//    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
//    if (data)
//    {
//        GLenum format;
//        if (nrComponents == 1)
//            format = GL_RED;
//        else if (nrComponents == 3)
//            format = GL_RGB;
//        else if (nrComponents == 4)
//            format = GL_RGBA;
//
//        glBindTexture(GL_TEXTURE_2D, textureID);
//        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//        glGenerateMipmap(GL_TEXTURE_2D);
//
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//        stbi_image_free(data);
//    }
//    else
//    {
//        std::cout << "Texture failed to load at path: " << path << std::endl;
//        stbi_image_free(data);
//    }
//
//    return textureID;
//}

////Collision Detection Functions
//void Model::setBoundingBox()
//{
//    //go through all vertices in mesh to find min and max values for x, y, z
//    //should only need to call this function when a model is first created
//    for (int i = 0; i < meshes.size(); i++)
//    {
//        for (int j = 0; j < meshes[i].vertices.size(); j++)
//        {
//            //only care about the Position vector within the Vertex object
//            float x_pos = meshes[i].vertices[j].Position[0];
//            float y_pos = meshes[i].vertices[j].Position[1];
//            float z_pos = meshes[i].vertices[j].Position[2];
//
//            //check x
//            if (x_pos > max_coordinates[0]) max_coordinates[0] = x_pos;
//            else if (x_pos < min_coordinates[0]) min_coordinates[0] = x_pos;
//
//            //check y
//            if (y_pos > max_coordinates[1]) max_coordinates[1] = y_pos;
//            else if (y_pos < min_coordinates[1]) min_coordinates[1] = y_pos;
//
//            //check z
//            if (z_pos > max_coordinates[2]) max_coordinates[2] = z_pos;
//            else if (z_pos < min_coordinates[2]) min_coordinates[2] = z_pos;
//        }
//    }
//}
//glm::vec3 Model::transformVertex(glm::vec3 vertex)
//{
//    //order of operations for vertex transform is translate, rotate, then scale
//    vertex *= model_location;
//    QuatRotate(model_rotation, vertex);
//    vertex *= model_scale;
//
//    return vertex;
//}
//
////Helper Functions
//ModelType modeltypeFromInt(int m)
//{
//    if (m == 0) return ModelType::CLUB;
//    else if (m == 1) return ModelType::CHIP;
//    else if (m == 2) return ModelType::BALL;
//    else if (m == 3) return ModelType::BACKGROUND;
//    else if (m == 4) return ModelType::LINE_OBJECTS;
//}