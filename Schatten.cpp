#include <iostream>
#include <random>
#include <array>
#include <direct.h>
#include <thread>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <chrono>
#include "assets/impl/ShaderAsset.h"
#include "assets/impl/TextureAsset.h"
#include <cmath> // For std::isnan

#include "components/Entity.h"

#include "renderer/Renderer.h"

#include "renderer/StandartRenderPipeline.h"


#include "renderer/GraphNode.h"

#include "renderer/Systems/MeshRendererSystem.h"

#include "components/CameraComponent.h"
#include "components/CameraController.h"
#include "gl/GL_CALL.h"
#include "gl/Texture/Sampler.h"
#include "opengl/Mesh.h"
#include "opengl/ShaderFactory.h"
#include "opengl/VertexArrayObject.h"
#include "renderer/Systems/TerrainRenderSystem.h"

#include "sqlite/sqlite3.h"


#include "utility/ThreadPool.h"

using namespace component;
using namespace graphics;

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 uv;
};

class Stopwatch
{
    
    std::chrono::high_resolution_clock::time_point begin;
public:
    void start()
    {
        begin = std::chrono::high_resolution_clock::now();
    }
    template<typename T>
    size_t elapsed()
    {
        return std::chrono::duration_cast<T>( std::chrono::high_resolution_clock::now()- begin).count();
    }
};


struct BasicVertex {
    glm::vec3 position;
};
std::shared_ptr<Mesh> unitCube() {

    std::vector<LayoutDescriptor> layout;

    layout.emplace_back(attributePointer(Position, BasicVertex, position));

    std::shared_ptr<Mesh> mesh = Mesh::Create(sizeof(BasicVertex), layout);


    std::vector<BasicVertex> vertices;
    vertices.emplace_back(glm::vec3(-1, -1, -1));
    vertices.emplace_back(glm::vec3(1, -1, -1));
    vertices.emplace_back(glm::vec3(-1, 1, -1));
    vertices.emplace_back(glm::vec3(1, 1, -1));
    vertices.emplace_back(glm::vec3(-1, -1, 1));
    vertices.emplace_back(glm::vec3(1, -1, 1));
    vertices.emplace_back(glm::vec3(-1, 1, 1));
    vertices.emplace_back(glm::vec3(1, 1, 1)); 

    std::vector<glm::uvec3> triangles;


    triangles.emplace_back(1, 0, 2);
    triangles.emplace_back(2, 3, 1);

    // Back face (z = 1)
    triangles.emplace_back(4, 5, 7);
    triangles.emplace_back(7, 6, 4);

    // Left face (x = -1)
    triangles.emplace_back(0, 4, 6);
    triangles.emplace_back(6, 2, 0);

    // Right face (x = 1)
    triangles.emplace_back(5, 1, 3);
    triangles.emplace_back(3, 7, 5);

    // Top face (y = 1)
    triangles.emplace_back(2, 6, 7);
    triangles.emplace_back(7, 3, 2);

    // Bottom face (y = -1)
    triangles.emplace_back(5, 4, 0);
    triangles.emplace_back(0, 1, 5);

    mesh->SetPolygonMode(TriangleMode::Triangle, FillMode::Fill, CullFace::Front);
    mesh->SetIndexData(triangles);
    mesh->SetVertexData(vertices);
    mesh->Sync();
    

    return mesh;
}



constexpr static std::array<glm::mat3, 6> faces{
    // Top Face
    glm::mat3(glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0)),
    // Bottom Face
    glm::mat3(glm::vec3(0, 0, -1), glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
    // Right Face
    glm::mat3(glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0)),
    // Left Face
    glm::mat3(glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), glm::vec3(-1, 0, 0)),
    // Front Face
    glm::mat3(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
    // Back Face
    glm::mat3(glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1)),
};



Vertex CalculateVertex(glm::vec2 planePos, glm::mat3 upDirection)
{
    std::array<glm::vec2, 3> uvs{//the uvs cannot be used for the tangent you have to use the position purly
        planePos,
        planePos + glm::vec2(0.1,0.0),
        planePos + glm::vec2(0,0.1),
    };

    std::array<glm::vec3, 3> samples;
    for (size_t i = 0; i < 3; i++)
    {
        samples[i] = upDirection * glm::vec3(uvs[i], 1);
        samples[i] = samples[i];
    }
    glm::vec3 edge0 = samples[0] - samples[1];
    glm::vec3 edge1 = samples[0] - samples[2];

    glm::vec3 normal = normalize(glm::cross(edge0, edge1));
    glm::vec3 tangent = normalize(edge0);
    tangent = tangent - normal * glm::dot(tangent, normal);

    return Vertex(samples[0], normal, tangent, glm::vec2(0));
}


void GeneratePlane(Vertex* vertices, uint32_t* indices, glm::mat3 face, int& t, int& v, int resolution, float height = 1) {


    float interval = 1.0f / (resolution - 1) * 2;
    for (int y = 0; y < resolution; y++)
    {
        for (int x = 0; x < resolution; x++)
        {
            vertices[v] = CalculateVertex(glm::vec2(static_cast<float>(x) * interval - 1.0, static_cast<float>(y) * interval - 1.0), face);

            if (x != resolution - 1 && y != resolution - 1)
            {
                indices[t++] = (v);
                indices[t++] = (v + resolution + 1);
                indices[t++] = (v + resolution);
                indices[t++] = (v);
                indices[t++] = (v + 1);
                indices[t++] = (v + resolution + 1);
            }
            v++;
        }
    }
}

void GeneratePlane(int32_t resolution, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, int face = 0)
{
    int resm1 = resolution - 1;

    vertices.resize(resolution * resolution);
    indices.resize(resm1 * resm1 * 6);

    int v = 0;
    int t = 0;
    GeneratePlane(vertices.data(), indices.data(), faces[face], v, t, resolution, 0);
}

void GenerateCube(int resolution, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{

    int resm1 = resolution - 1;

    vertices.resize(resolution * resolution * 6);
    indices.resize(resm1 * resm1 * 36);

    int v = 0;
    int t = 0;
    for (int i = 0; i < 6; i++)
    {
        GeneratePlane(vertices.data(), indices.data(), faces[i], v, t, resolution);
    }
}

void GenerateSphere(int resolution, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
    GenerateCube(resolution, vertices, indices);
}

std::shared_ptr<Mesh> unitSphere() {

    std::vector<LayoutDescriptor> layout;

    layout.emplace_back(attributePointer(Position, Vertex, position));

    std::shared_ptr<Mesh> mesh = Mesh::Create(sizeof(Vertex), layout);


    std::vector<Vertex> vertices;
    std::vector<uint32_t> triangles;
    GenerateSphere(32, vertices, triangles);

    mesh->SetPolygonMode(TriangleMode::Triangle, FillMode::Fill, CullFace::None);
    mesh->SetIndexData(triangles);
    mesh->SetVertexData(vertices);
    mesh->Sync();

    return mesh;
}
std::shared_ptr<Mesh> unitPlane(int res) {

    std::vector<LayoutDescriptor> layout;

    layout.emplace_back(attributePointer(Position, Vertex, position));

    std::shared_ptr<Mesh> mesh = Mesh::Create(sizeof(Vertex), layout);


    std::vector<Vertex> vertices;
    std::vector<uint32_t> triangles;
    GeneratePlane(res, vertices, triangles);

    mesh->SetPolygonMode(TriangleMode::Triangle, FillMode::Fill, CullFace::None);
    mesh->SetIndexData(triangles);
    mesh->SetVertexData(vertices);
    mesh->Sync();

    return mesh;
}

static glm::dvec3 cubeSphereProjection(glm::dvec3 worldPosition, double radius, double blend = 1)
{
    worldPosition = worldPosition / radius;

    glm::dvec3 squared = { worldPosition.x * worldPosition.x, worldPosition.y * worldPosition.y, worldPosition.z * worldPosition.z };

    double x = worldPosition.x * sqrt(1.0 - (squared.y - squared.z) / 2.0 + (squared.y * squared.z) / 3.0) * radius;
    double y = worldPosition.y * sqrt(1.0 - (squared.z - squared.x) / 2.0 + (squared.z * squared.x) / 3.0) * radius;
    double z = worldPosition.z * sqrt(1.0 - (squared.x - squared.y) / 2.0 + (squared.x * squared.y) / 3.0) * radius;


    return glm::dvec3(x, y, z) * blend + worldPosition * (1.0 - blend) * radius;
}






struct Node
{
   

    Node* parent = nullptr;
    std::array<std::unique_ptr<Node>, 4> children;
    float x0;
    float y0;
    float x1;
    float y1;

    float x = 572;
    float y = 572;
    float z = 572;

    Entity mesh;
    int32_t depth = 0;
    size_t id = 0;


    void Update(float cam_x, float cam_y, float cam_z, float lod, const std::function<void(Node*)>& drawFunc)
    {

        bool _split = need_split(cam_x, cam_y, cam_z, lod) || !parent;
        if ((_split) && children[0])
        {
            for (const auto& child : children)
            {
                child->Update(cam_x, cam_y, cam_z, lod, drawFunc);
            }
        }
        if (_split && !children[0])
        {
            split(drawFunc);
        }
        if (!_split)
        {
            if (!mesh.valid())
            {
                enqueue(drawFunc);
            }
        }


    }
    void OnDraw(float centerX, float centerY, float centerZ)
    {
        clear_parent_with_mesh();
        dispose_children();
        x = centerX;
        y = centerY;
        z = centerZ;

    }
    void DrawGizmo() const
    {
        if (mesh.valid())
        {
            glm::vec3 position = { x,y,z };
            Transform transform;
            transform.position() = position;
            renderer::Gizmos::Instance().DrawCube(transform.model(), glm::vec4(1));

        }

        for (const auto& child : children)
        {
            if (child)
            {
                child->DrawGizmo();
            }
        }
    }

    void split(const std::function<void(Node*)>& drawFunc)
    {
        for (int32_t quadrant = 0; quadrant < 4; quadrant++) {

            float size = x1 - x0;
            float child_extends_x = size / 4.0f;
            float child_extends_y = size / 4.0f;

            float center_x = (x0 + x1) / 2.0f;
            float center_y = (y0 + y1) / 2.0f;

            float sign_x = static_cast<float>((quadrant % 2) * 2 - 1);
            float sign_y = static_cast<float>((quadrant >= 2) * 2 - 1);

            float child_center_x = center_x + child_extends_x * sign_x * 1.0f;
            float child_center_y = center_y + child_extends_y * sign_y * 1.0f;

            children[quadrant] = std::make_unique_for_overwrite<Node>();
            Node& child = *children[quadrant];
            child.parent = this;
            child.x0 = child_center_x - child_extends_x;
            child.y0 = child_center_y - child_extends_y;
            child.x1 = child_center_x + child_extends_x;
            child.y1 = child_center_y + child_extends_y;
            child.enqueue(drawFunc);

            child.depth = depth + 1;
            child.id = id | static_cast<size_t>(quadrant) << (depth * 2);
        }
    }

    std::string printId()const
    {
        std::stringstream ss;
        for (size_t i = 0; i < depth; i++)
        {
            ss << (id >> i * 2 & 0b11) << "::";
        }
        return ss.str();
    }


    void enqueue(const std::function<void(Node*)>& drawFunc)
    {
        drawFunc(this);
    }
    bool need_split(float cam_x, float cam_y, float cam_z, float lod)const
    {
        float dx = x - cam_x;
        float dy = y - cam_y;
        float dz = z - cam_z;
        float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

        const float size = (x1 - x0);
        return size * lod > distance && size > 0.5;
    }

    void clear_parent_with_mesh()
    {
        int32_t drawn_siblings = !mesh.valid();
        for (const auto& sibling : parent->children)
        {
            drawn_siblings += sibling->mesh.valid();
        }
        if (drawn_siblings == 4)
        {
            parent->destroyMesh();
        }
    }
    void destroyMesh()
    {

        mesh.destroy();
    }
    void dispose_children()
    {
        for (auto& child : children)
        {
            child.reset();
        }
    }



    ~Node() = default;
};

struct MeshGenerationJob: Async::IJob<MeshGenerationJob>
{
    struct Task
    {
        glm::vec4 right;
        glm::vec4 forward;
        glm::dvec4 unpack;
        glm::dvec4 pack;
        double radius;
        double noiseScale;
        double noiseHeight;
        double nodeExtends;
    };


    Node* node = nullptr;
    std::shared_ptr<Mesh> mesh = nullptr;
    std::shared_ptr<shader::Material> material = nullptr;
    Entity entity;
    glm::dvec3 position = glm::dvec3(0);
    glm::dmat3 tbn = glm::dmat3(1);
    double size = 0;
    std::atomic<int32_t>* jobCount = nullptr;

    MeshGenerationJob() = default;
    MeshGenerationJob(glm::dmat3 tbn, double size, Entity&& entity, Node* node, std::shared_ptr<Mesh> mesh, std::shared_ptr<shader::Material> material)
    {
        this->tbn = tbn;
        this->size = size;
        this->entity = std::move(entity);
        this->mesh = mesh;
        this->material = material;
        this->node = node;
    }
    Async::Task execute()
    {
        jobCount->fetch_add(1);
        return Async::AsyncTask(&MeshGenerationJob::GenerateMesh, this);
    }
    Async::Task GenerateMesh()
    {
        glm::dvec3 forward = this->tbn[0];
        glm::dvec3 right = this->tbn[1];
        glm::dvec3 up = this->tbn[2];

        double extends = (node->y1 - node->y0);
        Task task;
        task.forward = glm::dvec4(forward, 0) * extends;
        task.right = glm::dvec4(right, 0) * extends;
        task.radius = size;
        task.noiseScale = 0.05*task.radius;
        task.noiseHeight = 0.03;
        task.nodeExtends = extends;

        glm::dvec2 localCenter = glm::dvec2(node->x0 + node->x1, node->y1 + node->y0);
        localCenter /= 2.0;

        glm::dvec3 est_center = glm::dvec3(forward) * localCenter.x + glm::dvec3(right) * localCenter.y +
            up * size;

        task.unpack = glm::dvec4(est_center, 0);
        task.pack = glm::dvec4(cubeSphereProjection(est_center, task.radius), 0);




        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        CreatePlane(16, task, vertices, indices);

        mesh->SetPolygonMode(TriangleMode::Triangle, FillMode::Fill, CullFace::Front);
        mesh->SetVertexData(vertices);
        mesh->SetIndexData(indices);
        mesh->RecalculateBounds<Vertex>();
        position = glm::vec3(task.pack);
        return Async::SyncTask(&MeshGenerationJob::SyncMesh, this);
    }
    Async::Task SyncMesh()
    {
        MeshFilter& mf = entity.get_or_emplace<MeshFilter>();
        MeshRenderer& mr = entity.get_or_emplace<MeshRenderer>();
        Transform& transform = entity.get_or_emplace<Transform>();
        mesh->Sync();
        mf.SetMesh(mesh);
        mr.SetMaterial(material);
        transform.position() = glm::vec3(position);

        glm::dvec3 true_center = position + glm::dvec3(mesh->boundingBox.GetCenter());
        node->OnDraw(true_center.x, true_center.y, true_center.z);
        node->mesh = std::move(entity);
        jobCount->fetch_add(-1);
        return exit();
    }

    static double triangleCoverage(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
        glm::vec3 ab = b - a;
        glm::vec3 ac = c - a;
        glm::vec3 up = glm::cross(ab, ac);
        double area = glm::length(up) * 0.5;

        if (std::isnan(area)) {
            return 1.0;
        }
        else {
            return area;
        }
    }


   


    template<typename Vertex>
    static void CreatePlane(uint32_t res, Task& task, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
    {
  
        for (uint32_t y = 0; y < res; y++)
        {
            for (uint32_t x = 0; x < res; x++)
            {
                if (x != res - 1 && y != res - 1)
                {
                    indices.emplace_back(vertices.size());
                    indices.emplace_back(vertices.size() + res + 1);
                    indices.emplace_back(vertices.size() + 1);
                    indices.emplace_back(vertices.size());
                    indices.emplace_back(vertices.size() + res);
                    indices.emplace_back(vertices.size() + res + 1);
                }


                auto uvToWorldPosition = [&](glm::vec2 _uv)
                    {
                        glm::dvec3 worldPosition = glm::dvec3(task.forward * (_uv.x - 0.5f) + task.right * (_uv.y - 0.5f));
                        worldPosition += glm::dvec3(task.unpack);
                        return worldPosition;
                    };

                auto applyNoise = [&](glm::dvec3 _worldPosition)
                    {
                        double noise0 = glm::perlin(_worldPosition / task.noiseScale);
                        double noise1 = glm::perlin(_worldPosition / task.noiseScale*2.0)/2.0;
                        double noise2 = glm::perlin(_worldPosition / task.noiseScale * 4.0) / 4.0;
                        double noise3 = glm::perlin(_worldPosition / task.noiseScale * 8.0) / 8.0;
                        double noise4 = glm::perlin(_worldPosition / task.noiseScale * 16.0) / 16.0;
                        double noise5 = glm::perlin(_worldPosition / task.noiseScale * 32.0) / 32.0;



                        _worldPosition *= ((noise0+noise1+noise2+noise3+noise4+noise5) * task.noiseHeight/2.0) + (1.0- task.noiseHeight);
                        return _worldPosition;
                    };


                auto getPosition = [&](glm::dvec2 uv, double radius)
                    {
                        glm::dvec3 worldPosition = uvToWorldPosition(uv);
                        worldPosition = cubeSphereProjection(worldPosition, radius, 1.0);
                        worldPosition = applyNoise(worldPosition);
                        return worldPosition;
                    };

                glm::vec2 uv{ x,y };
                uv /= static_cast<double>(res - 1);


                glm::dvec3 normalPosition = getPosition(uv - glm::vec2(0.01, 0.01), task.radius);

                glm::dvec3 tangentPosition = getPosition(uv + glm::vec2(0.01, 0.0), task.radius);

                glm::dvec3 bitangentPosition = getPosition(uv + glm::vec2(0.0, 0.01), task.radius);

                glm::dvec3 worldPosition = (normalPosition + tangentPosition + bitangentPosition) / 3.0;

                glm::vec3 packedPosition = glm::vec3(worldPosition - glm::dvec3(task.pack));

                glm::dvec3 tangentVector = tangentPosition - worldPosition;
                glm::dvec3 bitangentVector = bitangentPosition - worldPosition;

                tangentVector = glm::normalize(tangentVector);
                bitangentVector = glm::normalize(bitangentVector);

                glm::vec3 normal = glm::normalize(glm::cross(bitangentVector, tangentVector));
                glm::vec3 tangent = glm::vec3(tangentVector);

                Vertex vertex;

                vertex.position = packedPosition;
                vertex.normal = normal;
                vertex.tangent = tangent;
                vertex.uv = glm::vec2(uv);
                vertices.emplace_back(vertex);
            }
        }
    }

};


struct AsyncTextureLoad: Async::IJob<AsyncTextureLoad>
{
	asset::LoadedTexture textureAsset;
    std::shared_ptr<Texture> texture;

    std::function< std::shared_ptr<Texture>()> createTexture;
    std::function<asset::LoadedTexture()> loadPixels;
    std::function<std::shared_ptr<Texture>(AsyncTextureLoad* asset)> sync;



    Async::Task execute()
    {
        return Async::AsyncTask(&AsyncTextureLoad::load, this);
    }
    Async::Task load()
    {
        textureAsset = loadPixels();
        return Async::SyncTask(&AsyncTextureLoad::syncronize, this);
    }
    Async::Task syncronize()
    {
        sync(this);
        return exit();
    }
};


class QuadTree
{
    std::unique_ptr<Node> root;
    entt::registry* entities;
    std::queue<Node*> drawQueue;
    std::shared_ptr<shader::Material> simpleMaterial;
    glm::mat3 tbn = glm::mat3(1.0f);
    double size = 0;
    bool IsSphere = false;
    std::unique_ptr<std::atomic_int32_t> activeJobs;


   
   
public:

    QuadTree() = default;

    void Init(entt::registry* registry, const glm::mat3& _tbn, float _size, bool _IsSphere, std::shared_ptr<shader::Material> _simpleMaterial)
    {
        tbn = _tbn;
        size = _size;
        IsSphere = _IsSphere;
        root = std::make_unique_for_overwrite<Node>();
        root->parent = nullptr;
        root->x0 = -size;
        root->y0 = -size;
        root->x1 = size;
        root->y1 = size;
        activeJobs = std::make_unique<std::atomic_int32_t>(0);



        this->entities = registry;
        simpleMaterial = std::move(_simpleMaterial);
    }

    void Update(float cam_x, float cam_y, float cam_z, float lod)
    {
        if (drawQueue.empty() && activeJobs->load() == 0)
        {
            auto func = [this](Node* node) {drawQueue.push(node); };
            root->Update(cam_x, cam_y, cam_z, lod, func);
        }
        WorkOnQueue(1);
    }



    void WorkOnQueue(int max_it)
    {
        while (!drawQueue.empty())
        {
            Node* node = drawQueue.front();
            
            std::vector<LayoutDescriptor> layoutDescriptor{
                attributePointer(Position, Vertex, position),
                attributePointer(Normal, Vertex, normal),
                attributePointer(Tangent, Vertex, tangent),
                attributePointer(UV, Vertex, position),
            };

            std::shared_ptr<Mesh> mesh = Mesh::Create(sizeof(Vertex), layoutDescriptor);

            drawQueue.pop();
            
			MeshGenerationJob* job = new MeshGenerationJob();
            job->mesh = mesh;
            job->entity = Entity(entities);
            job->tbn = tbn;
            job->size = size;
            job->material = simpleMaterial;
            job->node = node;
            job->jobCount = activeJobs.get();
            job->execute();

            if (max_it-- == 0)
            {
                break;
            }
        }
    }
    Node& getRoot()
    {
        return *root;
    }
};


glm::ivec3 stringToVec3(std::string_view dir) {

    if (dir == "+x") return glm::ivec3(1, 0, 0);
    if (dir == "-x") return glm::ivec3(-1, 0, 0);
    if (dir == "+y") return glm::ivec3(0, 1, 0);
    if (dir == "-y") return glm::ivec3(0, -1, 0);
    if (dir == "+z") return glm::ivec3(0, 0, 1);
    if (dir == "-z") return glm::ivec3(0, 0, -1);
    return glm::ivec3(0); // Default case, should ideally never happen
}


// Function to compile shader and create program
GLuint compileAndLinkShaders(const char* vertexShaderSrc, const char* fragmentShaderSrc) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vertexShader);
    // Check for shader compile errors...

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fragmentShader);
    // Check for shader compile errors...

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Check for linking errors...

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
GLuint compileComputeShader(const char* computeShaderSrc) {
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &computeShaderSrc, nullptr);
    glCompileShader(computeShader);
    // Check for shader compile errors...


    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, computeShader);
    glLinkProgram(shaderProgram);
    // Check for linking errors...

    glDeleteShader(computeShader);

    return shaderProgram;
}

static glm::dvec3 cubeSphereProjection(glm::dvec3 worldPosition)
{

    glm::dvec3 squared = { worldPosition.x * worldPosition.x, worldPosition.y * worldPosition.y, worldPosition.z * worldPosition.z };

    double x = worldPosition.x * sqrt(1.0 - (squared.y + squared.z) / 2.0 + (squared.y * squared.z) / 3.0);
    double y = worldPosition.y * sqrt(1.0 - (squared.z + squared.x) / 2.0 + (squared.z * squared.x) / 3.0);
    double z = worldPosition.z * sqrt(1.0 - (squared.x + squared.y) / 2.0 + (squared.x * squared.y) / 3.0);


    return glm::dvec3(x, y, z);
}


static glm::dvec3 cubeSphereProjectioInverseFabiani(glm::dvec3 cam)
{
    cam = normalize(cam);
    glm::dvec3 squared = cam * cam;

    double b = 2 * (squared.x - squared.y) - 3;
    double c = 6 * squared.y;

    double y_sqr = (-b - sqrt(b * b - 4 * c)) / 2.0;
    double x_sqr = (squared.x - squared.y) + y_sqr;
    double z_sqr = 1;

    return sqrt(glm::dvec3(x_sqr, y_sqr, z_sqr))*sign(cam);
}
int main()
{
    Async::SetWorkers(25);

    char currentDirectory[1024];

    FreeImage_Initialise();

    if (_getcwd(currentDirectory, sizeof(currentDirectory)) != nullptr) {
        // The currentDirectory variable now contains the path to the current working directory.
        std::cout << "Project path: " << currentDirectory << std::endl;
    }
    else {
        // Handle the case where getcwd failed.
        std::cerr << "Error getting current working directory." << std::endl;
    }
    std::string projPath(currentDirectory);

    path::Shortcuts::emplace("$ProjectPath", [projPath](std::string& str) {
        path::Shortcuts::replace(str, "$ProjectPath", projPath);
        });
    path::Shortcuts::emplace("$Resources", [](std::string& str) {
        path::Shortcuts::replace(str, "$Resources", "$ProjectPath\\Resources");
        });
    path::Shortcuts::emplace("$Shaders", [](std::string& str) {
        path::Shortcuts::replace(str, "$Shaders", "$ProjectPath\\Resources\\Shaders");
        });
    path::Shortcuts::emplace("$Textures", [](std::string& str) {
        path::Shortcuts::replace(str, "$Textures", "$ProjectPath\\Resources\\Textures");
        });
    path::Shortcuts::emplace("$ComputeShaders", [](std::string& str) {
        path::Shortcuts::replace(str, "$ComputeShaders", "$ProjectPath\\Resources\\ComputeShaders");
        });
    
   


    //OpenGLLogger::Instance().Init(asset::AssetPath("\\Debug\\open_gl_call.db"));

    renderer::RenderPipeline pipeline;
    scene::SceneClass scene;
    scene.BindPipeline(&pipeline);


    auto meshRenderSystem = scene.renderSystems.emplace<renderer::MeshRenderSystem>();
    auto terrainRenderSystem = scene.renderSystems.emplace<renderer::TerrainRenderSystem>();


    auto cameraComponentSystem = scene.componentSystems.emplace<component::CameraComponent::System>();
    auto cameraControllerSystem = scene.componentSystems.emplace<component::DirectionalLightController::System>();
    auto lightControllerSystem = scene.componentSystems.emplace<component::CameraControllerSystem>();


    scene.Initialize();




    Entity cameraEntity = scene.CreateEntity();
    auto cameraComponent = &cameraEntity.get_or_emplace<CameraComponent>();
    auto cameraController = &cameraEntity.get_or_emplace<CameraController>();
    auto transformComponent = &cameraEntity.get_or_emplace<Transform>();

    cameraComponent->initialize(90, 0.1f, 100000.0f, "Main", 100, pipeline.GetRenderTexture("GeometryTarget"));
    cameraController->initialize(0.01f, 15.0f, 0.9f);
    transformComponent->Move(glm::vec3(1, 0, 0));


    Sampler defaultSampler;
    defaultSampler.wrapR = WrapMode::REPEAT;
    defaultSampler.wrapS = WrapMode::REPEAT;
    defaultSampler.wrapT = WrapMode::REPEAT;
    defaultSampler.minFilter = MinFilter::NEAREST_MIPMAP_LINEAR;
    defaultSampler.magFilter = MagFilter::LINEAR;


    auto material = shader::ShaderFactory::MakeMaterial(
        shader::ShaderType::Vertex, "$Shaders\\Default.vert",
        shader::ShaderType::Fragment, "$Shaders\\Default.frag",
        shader::ShaderType::Geometry, "$Shaders\\Default.geo"
    );
    auto transform = scene.screen.GetWindowSize();
    glm::vec2 wh = transform->wh();
    material->BindValue("u_resolution", glm::value_ptr(wh));




    std::vector<graphics::LayoutDescriptor> vertexLayout;
    vertexLayout.emplace_back(attributePointer(Position, Vertex, position));
    vertexLayout.emplace_back(attributePointer(Normal, Vertex, normal));
    vertexLayout.emplace_back(attributePointer(Tangent, Vertex, tangent));
    vertexLayout.emplace_back(attributePointer(UV, Vertex, uv));

    Entity redMesh = scene.CreateEntity();
    Entity blueMesh = scene.CreateEntity();

    auto skyboxShader =
        shader::ShaderFactory::MakeMaterial(
            shader::ShaderType::Vertex, "$Shaders\\Skybox.vert", shader::ShaderType::Fragment, "$Shaders\\Skybox.frag"
        );


    std::array<glm::mat3, 6> matrices = {
    glm::mat3(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),  // Front
    glm::mat3(glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1)), // Back
    glm::mat3(glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0)),   // Top
    glm::mat3(glm::vec3(-1, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0)), // Bottom
    glm::mat3(glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0)),  // Right
    glm::mat3(glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), glm::vec3(-1, 0, 0))   // Left
    };


    std::vector<AsyncTextureLoad*> loaders;


    Sampler cubemapSampler;
    cubemapSampler.wrapS = WrapMode::CLAMP_TO_EDGE;
    cubemapSampler.wrapT = WrapMode::CLAMP_TO_EDGE;
    cubemapSampler.wrapR = WrapMode::CLAMP_TO_EDGE;
    cubemapSampler.minFilter = MinFilter::LINEAR;
    cubemapSampler.magFilter = MagFilter::LINEAR;
    cubemapSampler.cubeMapSeamless = SeamlessCubemap::ENABLED;
  
    
    struct Scope: Async::IJob<Scope>
    {
        std::shared_ptr<Texture> cubemap;
        std::shared_ptr<Texture> texture;
        std::shared_ptr<shader::ComputeShader> controller;
        std::string path;
        int tileWidth;
        glm::ivec2 grid;
        asset::LoadedTexture asset;
        Sampler sampler;
        Sampler cubemapSampler;

        std::shared_ptr<shader::Material> material;
        Async::Task LoadTexture()
        {
            asset.Load(path);
            asset = asset.tile(tileWidth, grid, true);
            return Async::SyncTask(&Scope::SyncTexture, this);
        }
        Async::Task SyncTexture()
        {
            texture->TextureParams(asset.getDim());
            texture->Allocate(sampler);
            Texture::TextureSubImage3D(*texture,asset.pixels,  asset.pixelFormat, asset.pixelType);
            cubemap->TextureParams(1024, 1024);
            cubemap->Allocate(cubemapSampler);


            std::shared_ptr<shader::ComputeShader> program = shader::ShaderFactory::MakeComputeShader("$Shaders\\CubemapShader.vert");
            
            program->Bind();
            cubemap->BindToImageUnit(0, WriteAccess::Writeonly, 0, 0);
            cubemap->BindToImageUnit(1, WriteAccess::Writeonly, 0, 1);
            cubemap->BindToImageUnit(2, WriteAccess::Writeonly, 0, 2);
            cubemap->BindToImageUnit(3, WriteAccess::Writeonly, 0, 3);
            cubemap->BindToImageUnit(4, WriteAccess::Writeonly, 0, 4);
            cubemap->BindToImageUnit(5, WriteAccess::Writeonly, 0, 5);

            glm::ivec2 size = cubemap->getSize();

            program->Uniform2iv("cubemap_res", &size[0]);

            program->BindSamplerTexture("equirectTexture", *texture, 0);
            glm::vec2 tiling = asset.tiling;
            program->Uniform2fv("equirectTextureGridSize", &tiling[0]);

            program->DispatchCompute(size.x / 16, size.y / 16, 1);
        	return exit();
        }
        ~Scope()
        {
            std::cout << "exit\n";
        }
    };

    auto moon_albedo_cubeMap = std::make_shared<Texture>(TextureTarget::TextureCubeMap, TextureFormat::RGBA8);
    auto albedo_read = std::make_shared<Texture>(TextureTarget::Texture2DArray, TextureFormat::RGBA8);
	Scope* scope = new Scope();
    scope->cubemap = moon_albedo_cubeMap;
    scope->cubemapSampler = cubemapSampler;
    scope->sampler = defaultSampler;
    scope->grid = {10,5};
    scope->tileWidth = 512;
    scope->texture = albedo_read;
    scope->path = "$Textures\\moon_albedo.tif";
    Async::Task task = Async::AsyncTask(&Scope::LoadTexture, scope);



   




  std::vector<std::unique_ptr<QuadTree>> Faces;
  
	for (size_t i = 0; i < 6; i++)
    {
		//Faces.emplace_back(new QuadTree());
        //Faces.back()->Init(&scene.registry, matrices[i], 100, true, material);
    }


    float time = 0;

    auto prog =
        shader::ShaderFactory::NewLoadShader("$ComputeShaders\\TemplateCubemap.vert", 
            shader::name("DefaultShader"),
            shader::replace("imageFormat", "rgba8")
        );

    auto cubeMapShader = shader::ShaderFactory::MakeComputeShader("$Shaders\\CubemapShader.vert");

    Stopwatch watch;
    size_t elapsed = 0;
    size_t frameCount = 0;

    while (!glfwWindowShouldClose(scene.screen.get_GLFW_window()))
    {
       /* for (float y = -1.0; y < 1; y += 0.1)
        {
            for (float x = -1.0; x < 1; x += 0.1)
            {
                glm::dvec3 pos = glm::dvec3(x, y, 1);

                glm::dvec3 norm = cubeSphereProjection(pos);
                glm::dvec3 reverse = cubeSphereProjectioInverseFabiani(norm);

                renderer::Gizmos::Instance().DrawSphere(pos+glm::dvec3(0,0,0.02), glm::dvec3(0.01, 0.01, 0.01), glm::rgba8(0, 0, 255, 255));

				renderer::Gizmos::Instance().DrawSphere(reverse, glm::dvec3(0.01, 0.01, 0.01), glm::rgba8(255, 0, 0, 255));


            }
        }
*/


        
       renderer::Mesh::TriangleCount() = 0;
           material->BindTexture("cubemap", moon_albedo_cubeMap);

           scene.inputSystem.PollEvents();

           Async::WorkOnMainThread();

           scene.componentSystems.ForEach([](component::SystemBase* sys) {sys->Update(); });

           scene.pipeline->Render();

           glm::vec3 camPos = cameraEntity.get<Transform>().position();
           for (auto& face : Faces)
           {
               if (face) face->Update(camPos.x, camPos.y, camPos.z, 1.5);
           }

           material->BindValue("iTime", &time);
        
        scene.screen.SwapBuffers();
        time += 0.01f;
        if(frameCount++%100==0){
            size_t qs = watch.elapsed<std::chrono::microseconds>();
            std::cout << "average frame time: " << qs/100 << "qs" << std::endl;
            watch.start();
        }
        

    }
    Async::CleanUp();
    return 0;
}

// Run programs: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug programs: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
