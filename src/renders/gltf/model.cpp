#include "renders/gltf/model.h"

#include <glad/gl.h>

#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cstdio>

namespace{
    struct Binder{
        glm::vec3 translation{0.0f};
        glm::vec3 scale{1.0f};
        glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
        int parent = -1;
    };

    std::vector<uint32_t> readIndices(const tinygltf::Model& model, int indx){
        const tinygltf::Accessor& acc = model.accessors[indx];
        const tinygltf::BufferView& buffView = model.bufferViews[acc.bufferView];
        const tinygltf::Buffer& buff = model.buffers[buffView.buffer];
        const uint8_t* base = buff.data.data() + acc.byteOffset + buffView.byteOffset;
        int stride = static_cast<int>(acc.ByteStride(buffView));
        std::vector<uint32_t> result(acc.count);

        for(int i = 0; i < acc.count; i++){
            const uint8_t* temp = i * stride + base;

            if(acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE){
                result[i] = *temp;
            }
            else if(acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT){
                result[i] = *reinterpret_cast<const uint16_t*>(temp);
            }
            else{
                result[i] = *reinterpret_cast<const uint32_t*>(temp);
            }
        }

        return result;
    }

    std::vector<glm::vec3> readVector(const tinygltf::Model& model, int indx){
        const tinygltf::Accessor& acc = model.accessors[indx];
        const tinygltf::BufferView& buffView = model.bufferViews[acc.bufferView];
        const tinygltf::Buffer& buff = model.buffers[buffView.buffer];
        const uint8_t* base = buff.data.data() + acc.byteOffset + buffView.byteOffset;
        int stride = static_cast<int>(acc.ByteStride(buffView));
        std::vector<glm::vec3> result(acc.count);

        for(int i = 0; i < acc.count; i++){
            const float* temp = reinterpret_cast<const float*>(i * stride + base);
            result[i] = glm::vec3(temp[0], temp[1], temp[2]); 
        }

        return result;
    }
}

Gltf::~Gltf(){
    for(const Primitive& primitive : primitives){
        if(primitive.vao){
            glDeleteVertexArrays(1, &primitive.vao);
        }
        if(primitive.vbo){
            glDeleteBuffers(1, &primitive.vbo);
        }
        if(primitive.ebo){
            glDeleteBuffers(1, &primitive.ebo);
        }
    }
}

bool Gltf::loadModel(const std::string& path){
    std::string error, warning;
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;

    bool isLoaded = loader.LoadASCIIFromFile(&model, &error, & warning, path);
    if(!isLoaded){
        std::fprintf(stderr, "Could not load GLTF model %s: %s\n", path.c_str(), error.c_str());
        return false;
    }
    if(!warning.empty()){
        std::fprintf(stderr, "Warning loading GLTF %s: %s\n", path.c_str(), warning.c_str());
    }

    std::vector<Binder> bindings(model.nodes.size());
    for(int i = 0; i < model.nodes.size(); i++){
        const tinygltf::Node& node = model.nodes[i];
        Binder& bind = bindings[i];
        if(node.matrix.size() == 16){
            std::vector<float> mtrx(node.matrix.begin(), node.matrix.end());
            
            glm::mat4 mtrxData = glm::make_mat4(mtrx.data());
            bind.translation = glm::vec3(mtrxData[3]);
            
            glm::vec3 column0(mtrxData[0]);
            glm::vec3 column1(mtrxData[1]);
            glm::vec3 column2(mtrxData[2]);
            bind.scale = glm::vec3(
                glm::length(column0),
                glm::length(column1),
                glm::length(column2)
            );

            glm::mat3 rotation(
                column0 / bind.scale.x,
                column1 / bind.scale.y,
                column2 / bind.scale.z
            );
            bind.rotation = glm::quat_cast(rotation);
        }
        else{
            if(node.translation.size() == 3){
                bind.translation = glm::vec3(
                    static_cast<float>(node.translation[0]),
                    static_cast<float>(node.translation[1]),
                    static_cast<float>(node.translation[2])
                );
            }
            else{
                bind.translation = glm::vec3(0.0f);
            }

            if(node.rotation.size() == 4){
                bind.rotation = glm::quat(
                    static_cast<float>(node.rotation[3]),
                    static_cast<float>(node.rotation[0]),
                    static_cast<float>(node.rotation[1]),
                    static_cast<float>(node.rotation[2])
                );
            }
            else{
                bind.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            }

            if(node.scale.size() == 3){
                bind.scale = glm::vec3(
                    static_cast<float>(node.scale[0]),
                    static_cast<float>(node.scale[1]),
                    static_cast<float>(node.scale[2])
                );
            }
            else{
                bind.scale = glm::vec3(1.0f);
            }
        }
    }

    for(int i = 0; i < model.nodes.size(); i++){
        for(int child : model.nodes[i].children){
            bindings[child].parent = i;
        }
    }

    auto local = [](const Binder& bind){
        return
            glm::translate(glm::mat4(1.0f), bind.translation) *
            glm::mat4_cast(bind.rotation) *
            glm::scale(glm::mat4(1.0f), bind.scale);
    };
    
    auto global = [&](int indx){
        glm::mat4 mtrx(1.0f);
        for(int i = indx; i >= 0; i = bindings[i].parent){
            mtrx = local(bindings[i]) * mtrx;
        }
        return mtrx;
    };

    const glm::vec3 defaultColor(0.8f, 0.8f, 0.8f);
    for(int i = 0; i < model.nodes.size(); i++){
        const tinygltf::Node& node = model.nodes[i];

        if(node.mesh < 0){
            continue;
        }

        tinygltf::Mesh& mesh = model.meshes[node.mesh];
        glm::mat4 globTransform = global(i);
        glm::mat3 transformNormal = glm::mat3(glm::transpose(glm::inverse(globTransform)));

        for(const tinygltf::Primitive& primitive : mesh.primitives){
            auto position = primitive.attributes.find("POSITION");

            if(position == primitive.attributes.end() || primitive.indices < 0){
                continue;
            }

            glm::vec3 mat(1.0f);
            if(primitive.material >= 0){
                const auto& matFactor = 
                    model.materials[primitive.material].pbrMetallicRoughness.baseColorFactor;
                mat = glm::vec3(matFactor[0], matFactor[1], matFactor[2]);
            }
            if(mat == glm::vec3(1.0f)){
                mat = defaultColor;
            }

            std::vector<glm::vec3> positions = readVector(model, position->second);

            auto normal = primitive.attributes.find("NORMAL");
            std::vector<glm::vec3> normals;
            if(normal != primitive.attributes.end()){
                normals = readVector(model, normal->second);
            }
            else{
                normals = std::vector<glm::vec3>(positions.size(), glm::vec3(0.0f, 1.0f, 0.0f));
            }

            for(int i = 0; i < positions.size(); i++){
                positions[i] = glm::vec3(globTransform * glm::vec4(positions[i], 1.0f));
                normals[i] = glm::normalize(transformNormal * normals[i]);
            }

            std::vector<PropModel> vertices(positions.size());
            for(int i = 0; i < positions.size(); i++){
                vertices[i] = {positions[i], mat, normals[i]};
            }
            std::vector<uint32_t> indices = readIndices(model, primitive.indices);

            Primitive primitiveGpu;
            primitiveGpu.indxCount = indices.size();
            glGenVertexArrays(1, &primitiveGpu.vao);
            glGenBuffers(1, &primitiveGpu.vbo);
            glGenBuffers(1, &primitiveGpu.ebo);

            glBindVertexArray(primitiveGpu.vao);
            glBindBuffer(GL_ARRAY_BUFFER, primitiveGpu.vbo);
            glBufferData(
                GL_ARRAY_BUFFER, 
                static_cast<GLsizeiptr>(vertices.size() * sizeof(PropModel)), 
                vertices.data(), 
                GL_STATIC_DRAW
            );
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitiveGpu.ebo);
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)),
                indices.data(),
                GL_STATIC_DRAW
            );

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(
                0, 
                3, 
                GL_FLOAT, 
                GL_FALSE, sizeof(PropModel), 
                (void*)offsetof(PropModel, position)
            );
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(
                1, 
                3, 
                GL_FLOAT, 
                GL_FALSE, 
                sizeof(PropModel), 
                (void*)offsetof(PropModel, normal)
            );
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(
                2, 
                3, 
                GL_FLOAT, 
                GL_FALSE, 
                sizeof(PropModel), 
                (void*)offsetof(PropModel, color)
            );

            glBindVertexArray(0);
            primitives.push_back(primitiveGpu);
        }
    }

    return true;
}

void Gltf::drawModel() const{
    for(const Primitive& primitive : primitives){
        glBindVertexArray(primitive.vao);
        glDrawElements(GL_TRIANGLES, primitive.indxCount, GL_UNSIGNED_INT, nullptr);
    }
    glBindVertexArray(0);
}