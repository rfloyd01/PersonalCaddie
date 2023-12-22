#pragma once

#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Graphics/Objects/3D/Meshes/ModelMesh.h>
//#include <Graphics/mesh.h>
//#include <Graphics/shader.h>
//#include <Graphics/stb_image.h>
#include <Math/glm.h>

#include "VolumeElement.h"

//global definitions
#define number_of_model_types 5;

//Classes, structs and enums defined in other header files
class Mesh;

//Structs and enums for this class
enum class ModelType
{
    //A list of the different types of objects that can be rendered
    //creating this enum class so that a single render map can be created for each
    //different mode. Storing everything in a single vector can make it tricky to know
    //which Model is kept at which location. By using a map, the golf club is now stored
    //in a location called club. So if I want to delete the club but keep everything else
    //on screen I can just delete all model data out from the [CLUB] location of the map

    CLUB = 0, //golf club to be rendered
    CHIP = 1, //sensor to be rendered during calibration mode
    BALL = 2, //golf ball to be rendered
    BACKGROUND = 3, //may want to render something that looks like a golf course in the future
    LINE_OBJECTS = 4 //may need to render things such as straight lines or circles for training purposes
};

struct Texture
{
    unsigned int id;
    std::string type;
    std::string path;
};

/*
The model class allows us to redner things that are much more complicated than the basic shapes of the
other volume element classes. Unlike those classes (such as the sphere and face) which have preset meshes,
the meshes of the model class are imported from actual models that were rendered in 3D using software such
as Blender, 3DSMax, etc. To help load the models correctly a library called ASSIMP is used to set both 
the meshes and materials.
*/

//Class definition
class Model : public VolumeElement
{
public:
    //PUBLIC FUNCTIONS
    //Constructors
    Model(std::string path)
    {
        loadModel(path);
    }
    Model();

    //Setup Functions
    void loadModel(std::string path);
    std::vector<std::vector<PNTVertex>> const& getVertices() { return m_vertices; }
    std::vector <std::vector<uint16_t>> const& getIndices() { return m_indices; }
    std::vector<std::vector<DirectX::XMFLOAT4>> const& getColors() { return m_colors; }
    std::vector<MaterialType> const& getTextures() { return m_textures; }
    void clearVertices() { m_vertices = {}; }
    void clearIndices() { m_indices = {}; }
    void clearColors() { m_colors = {}; }
    void clearTextures() { m_textures = {}; }
    //void setScale(glm::vec3 s);
    void setPosition(DirectX::XMFLOAT3 position);
    //void setRotation(glm::quat r);

    ////Rendering Functions
    //glm::vec3 getScale();
    //glm::vec3 getLocation();
    //glm::quat getRotation();
    //void Draw(Shader& shader);
    void translateAndRotateFace(DirectX::XMFLOAT3 location, DirectX::XMVECTOR quat);

    ////Get Functions
    //std::vector<glm::vec3> getBoundingBox();
    
private:
    //PRIVATE FUNCTIONS
    //Data Processing Functions
    void processNode(aiNode* node, const aiScene* scene);
    void processMeshVectors(aiMesh* mesh, const aiScene* scene);
    void loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    //unsigned int TextureFromFile(const char* path, const std::string& directory);

    //Mesh Variables: These are populated when the model is first loaded and then deleted after
    //the underlying mesh is created. The mesh isn't created at the same time as the Model since
    //it needs access to the DirectX device which requires the main thread and models can be loaded
    //asynchronously
    std::vector<std::vector<PNTVertex>> m_vertices;
    std::vector<std::vector<uint16_t>> m_indices;
    std::vector<std::vector<DirectX::XMFLOAT4>> m_colors; //each vector holds 3 XMFLOAT4 types, one each for ambient, diffuse and specular color (in that specific order)
    std::vector<MaterialType> m_textures;

    ////Collision Detection Functions
    //void setBoundingBox();
    //glm::vec3 transformVertex(glm::vec3 vertex); //transforms a vertex to match current scale, location and rotation matrices

    ////PRIVATE VARIABLES
    ////Data Vectors
    //std::vector<Mesh> meshes;
    std::string directory;
    //std::vector<Texture> textures_loaded;

    ////Rendering Variables
    //glm::vec3 model_scale = { 1.0, 1.0, 1.0 }; //by default everything should be the same size as the model loaded
    //glm::vec3 model_location = { 0.0, 0.0, 0.0 }; //by default all models should be loaded at the origin
    //glm::quat model_rotation = { 1.0, 0.0, 0.0, 0.0 }; //by default all models aren't rotated

    ////Size Variables
    //glm::vec3 min_coordinates = { 1000.0, 1000.0, 1000.0 };
    //glm::vec3 max_coordinates = { -1000.0, -1000.0, -1000.0 };
};

//Helper Functions
//ModelType modeltypeFromInt(int m);