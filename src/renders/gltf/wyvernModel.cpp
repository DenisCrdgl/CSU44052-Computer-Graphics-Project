#include "renders/gltf/wyvernModel.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/gl.h>
#include <tiny_gltf.h>
#include <cmath>
#include <functional>
#include <algorithm>
#include <cstdio>

namespace{
    bool skip(
        tinygltf::Image*, 
        const int, 
        std::string*, std::string*,
        int, int,
        const unsigned char*,
        int, 
        void*
    ){

        return true;
    }

    glm::vec3 sampleVecList(const VecFrameList& list, float time){
        if(list.frames.empty()){
            return glm::vec3(0.0f);
        }
        if(time <= list.frames.front().time || list.frames.size() == 1){
            return list.frames.front().val;
        }
        if(time >= list.frames.back().time){
            return list.frames.back().val;
        }

        auto nextFrame = std::upper_bound(
            list.frames.begin(),
            list.frames.end(),
            time,
            [](float frameTime, const VecFrame& frame){
                return frameTime < frame.time;
            }
        );
        auto prevFrame = nextFrame - 1;

        if(list.step){
            return prevFrame->val;
        }

        float period = nextFrame->time - prevFrame->time;
        float periodScale;
        if(period > 0.0f){
            periodScale = (time - prevFrame->time) / period;
        }
        else{
            periodScale = 0.0f;
        }

        return glm::mix(prevFrame->val, nextFrame->val, periodScale);
    }

    glm::quat sampleQuaterList(const QuaterFrameList& list, float time){
        if(list.frames.empty()){
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }
        if(time <= list.frames.front().time || list.frames.size() == 1){
            return list.frames.front().val;
        }
        if(time >= list.frames.back().time){
            return list.frames.back().val;
        }

        auto nextFrame = std::upper_bound(
            list.frames.begin(),
            list.frames.end(),
            time,
            [](float frameTime, const QuaterFrame& frame){
                return frameTime < frame.time;
            }
        );
        auto prevFrame = nextFrame - 1;

        if(list.step){
            return prevFrame->val;
        }

        float period = nextFrame->time - prevFrame->time;
        float periodScale;
        if(period > 0.0f){
            periodScale = (time - prevFrame->time) / period;
        }
        else{
            periodScale = 0.0f;
        }

        return glm::slerp(prevFrame->val, nextFrame->val, periodScale);
    }

    std::vector<glm::vec4> readVec4(const tinygltf::Model& model, int indx){
        const tinygltf::Accessor& acc = model.accessors[indx];
        const tinygltf::BufferView& buffView = model.bufferViews[acc.bufferView];
        const tinygltf::Buffer& buff = model.buffers[buffView.buffer];
        const uint8_t* base = buff.data.data() + acc.byteOffset + buffView.byteOffset;
        int stride = static_cast<int>(acc.ByteStride(buffView));
        std::vector<glm::vec4> result(acc.count);

        for(int i = 0; i < acc.count; i++){
            const float* temp = reinterpret_cast<const float*>(i * stride + base);
            result[i] = glm::vec4(temp[0], temp[1], temp[2], temp[3]);
        }

        return result;
    }

    std::vector<glm::vec3> readVec3(const tinygltf::Model& model, int indx){
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

    std::vector<glm::mat4> readMat(const tinygltf::Model& model, int indx){
        const tinygltf::Accessor& acc = model.accessors[indx];
        const tinygltf::BufferView& buffView = model.bufferViews[acc.bufferView];
        const tinygltf::Buffer& buff = model.buffers[buffView.buffer];
        const uint8_t* base = buff.data.data() + acc.byteOffset + buffView.byteOffset;
        int stride = static_cast<int>(acc.ByteStride(buffView));
        std::vector<glm::mat4> result(acc.count);

        for(int i = 0; i < acc.count; i++){
            const float* temp = reinterpret_cast<const float*>(i * stride + base);
            result[i] = glm::make_mat4(temp);
        }

        return result;
    }

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

    std::vector<float> readScalar(const tinygltf::Model& model, int indx){
        const tinygltf::Accessor& acc = model.accessors[indx];
        const tinygltf::BufferView& buffView = model.bufferViews[acc.bufferView];
        const tinygltf::Buffer& buff = model.buffers[buffView.buffer];
        const uint8_t* base = buff.data.data() + acc.byteOffset + buffView.byteOffset;
        int stride = static_cast<int>(acc.ByteStride(buffView));
        std::vector<float> result(acc.count);

        for(int i = 0; i < acc.count; i++){
            result[i] = *reinterpret_cast<const float*>(i * stride + base);
        }

        return result;
    }

    std::vector<glm::vec4> readJoints(const tinygltf::Model& model, int indx){
        const tinygltf::Accessor& acc = model.accessors[indx];
        const tinygltf::BufferView& buffView = model.bufferViews[acc.bufferView];
        const tinygltf::Buffer& buff = model.buffers[buffView.buffer];
        const uint8_t* base = buff.data.data() + acc.byteOffset + buffView.byteOffset;
        int stride = static_cast<int>(acc.ByteStride(buffView));
        std::vector<glm::vec4> result(acc.count);

        for(int i = 0; i < acc.count; i++){
            const uint8_t* temp = i * stride + base;

            if(acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT){
                const uint16_t* temp16 = reinterpret_cast<const uint16_t*>(temp);
                result[i] = glm::vec4(temp16[0], temp16[1], temp16[2], temp16[3]);
            }
            else{
                result[i] = glm::vec4(temp[0], temp[1], temp[2], temp[3]);
            }
        }

        return result;
    }
}

WyvernModel::~WyvernModel(){
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

bool WyvernModel::loadWyvern(const std::string& path){
    std::string error, warning;
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    loader.SetImageLoader(skip, nullptr);

    bool isLoaded = loader.LoadBinaryFromFile(&model, &error, &warning, path);
    if(!isLoaded){
        std::fprintf(stderr, "Could not load wyvern GLB: %s\n", error.c_str());
        return false;
    }
    if(!warning.empty()){
        std::fprintf(stderr, "Warning loading wyvern GLB: %s\n", warning.c_str());
    }

    bindings.resize(model.nodes.size());
    for(int i = 0; i < model.nodes.size(); i++){
        const tinygltf::Node& node = model.nodes[i];
        Binder& bind = bindings[i];
        bind.children = node.children;

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

    std::vector<bool> parents(model.nodes.size());
    for(int i = 0; i < model.nodes.size(); i++){
        for(int child : model.nodes[i].children){
            parents[child] = true;
        }
    }
    for(int i = 0; i < model.nodes.size(); i++){
        if(!parents[i]){
            roots.push_back(i);
        }
    }

    meshIndx = -1;
    int skinIndx = -1;
    for(int i = 0; i < model.nodes.size(); i++){
        if(model.nodes[i].skin >= 0 && model.nodes[i].mesh >= 0){
            meshIndx = i;
            skinIndx = model.nodes[i].skin;
            break;
        }
    }

    if(meshIndx < 0){
        std::fprintf(stderr, "Could not find wyvern skin mesh\n");
        return false;
    }

    const tinygltf::Skin& skin = model.skins[skinIndx];
    joints = skin.joints;
    if(joints.size() > 40){
        std::fprintf(stderr, "Wyvern has too many joints: %zu\n", joints.size());
        return false;
    }

    if(skin.inverseBindMatrices >= 0){
        inverseBindings = readMat(model, skin.inverseBindMatrices);
    }
    else{
        inverseBindings = std::vector<glm::mat4>(joints.size(), glm::mat4(1.0f));
    }

    for(const tinygltf::Animation& animationGltf : model.animations){
        Animation animation;
        for(const tinygltf::AnimationChannel& channel : animationGltf.channels){
            const tinygltf::AnimationSampler& sample = animationGltf.samplers[channel.sampler];
            std::vector<float> timings = readScalar(model, sample.input);
            bool step = sample.interpolation == "STEP";

            if(!timings.empty()){
                animation.length = std::max(animation.length, timings.back());
            }

            if(channel.target_path == "translation"){
                std::vector<glm::vec3> vals = readVec3(model, sample.output);
                VecFrameList list;
                list.step = step;
                for(int i = 0; i < timings.size(); i++){
                    list.frames.push_back({vals[i], timings[i]});
                }
                animation.translations[channel.target_node] = std::move(list);
            }
            else if(channel.target_path == "rotation"){
                std::vector<glm::vec4> vals = readVec4(model, sample.output);
                QuaterFrameList list;
                list.step = step;
                for(int i = 0; i < timings.size(); i++){
                    list.frames.push_back({
                        glm::quat(vals[i].w, vals[i].x, vals[i].y, vals[i].z),
                        timings[i]
                    });
                }
                animation.rotations[channel.target_node] = std::move(list);
            }
            else if(channel.target_path == "scale"){
                std::vector<glm::vec3> vals = readVec3(model, sample.output);
                VecFrameList list;
                list.step = step;
                for(int i = 0; i < timings.size(); i++){
                    list.frames.push_back({vals[i], timings[i]});
                }
                animation.scales[channel.target_node] = std::move(list);
            }
        }
        animations[animationGltf.name] = std::move(animation);
    }

    const tinygltf::Mesh& mesh = model.meshes[model.nodes[meshIndx].mesh];
    for(const tinygltf::Primitive& primitive : mesh.primitives){
        auto position = primitive.attributes.find("POSITION");
        auto normal = primitive.attributes.find("NORMAL");
        auto primJoints = primitive.attributes.find("JOINTS_0");
        auto primWeights = primitive.attributes.find("WEIGHTS_0");

        if(primitive.indices < 0 || position == primitive.attributes.end()){
            continue;
        }

        std::vector<glm::vec3> positions = readVec3(model, position->second);
        
        std::vector<glm::vec3> normals;
        if(normal != primitive.attributes.end()){
            normals = readVec3(model, normal->second);
        }
        else{
            normals = std::vector<glm::vec3> (
                positions.size(), 
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
        }

        std::vector<glm::vec4> vertJoints;
        if(primJoints != primitive.attributes.end()){
            vertJoints = readJoints(model, primJoints->second);
        }
        else{
            vertJoints = std::vector<glm::vec4>(
                positions.size(), 
                glm::vec4(0.0f)
            );
        }

        std::vector<glm::vec4> vertWeights;
        if(primWeights != primitive.attributes.end()){
            vertWeights = readVec4(model, primWeights->second);
        }
        else{
            vertWeights = std::vector<glm::vec4>(
                positions.size(),
                glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)
            );
        }

        std::vector<Vertex> vertices(positions.size());
        for(int i = 0; i < positions.size(); i++){
            vertices[i] = {
                positions[i],
                normals[i],
                glm::vec3(1.0f),
                vertWeights[i],
                vertJoints[i]
            };
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
            static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
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
            GL_FALSE,
            sizeof(Vertex),
            (void*)offsetof(Vertex, position)
        );

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            (void*)offsetof(Vertex, normal)
        );

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            (void*)offsetof(Vertex, color)
        );

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(
            3,
            4,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            (void*)offsetof(Vertex, weights)
        );

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(
            4,
            4,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            (void*)offsetof(Vertex, joints)
        );

        glBindVertexArray(0);
        primitives.push_back(primitiveGpu);
    }

    return true;
}

std::vector<glm::mat4> WyvernModel::jointMatrices(
    float length, 
    const std::string& animation) const{

    const Animation* animated = nullptr;
    auto animationEntry = animations.find(animation);

    if(animationEntry != animations.end()){
        animated = &animationEntry->second;
    }

    float animationTime = 0.0f;
    if(animated && animated->length > 0.0f){
        animationTime = std::fmod(length, animated->length);
        if(animationTime < 0.0f){
            animationTime += animated->length;
        }
    }

    std::vector<glm::mat4> globTransforms(bindings.size(), glm::mat4(1.0f));
    std::function<void(int, const glm::mat4&)> visitNode = 
        [&](int nodeIndx, const glm::mat4& parentGlobal){
            const Binder& bind = bindings[nodeIndx];
            glm::vec3 translation = bind.translation;
            glm::quat rotation = bind.rotation;
            glm::vec3 scale = bind.scale;

            if(animated){
                auto translated = animated->translations.find(nodeIndx);
                if(translated != animated->translations.end()){
                    translation = sampleVecList(translated->second, animationTime);
                }

                auto rotated = animated->rotations.find(nodeIndx);
                if(rotated != animated->rotations.end()){
                    rotation = sampleQuaterList(rotated->second, animationTime);
                }

                auto scaled = animated->scales.find(nodeIndx);
                if(scaled != animated->scales.end()){
                    scale = sampleVecList(scaled->second, animationTime);
                }
            }

            glm::mat4 local =
                glm::translate(glm::mat4(1.0f), translation) *
                glm::mat4_cast(rotation) *
                glm::scale(glm::mat4(1.0f), scale
            );
            glm::mat4 global = parentGlobal * local;
            globTransforms[nodeIndx] = global;

            for(int child : bind.children){
                visitNode(child, global);
            }
        };
    
    for(int root : roots){
        visitNode(root, glm::mat4(1.0f));
    }

    glm::mat4 inverseGlobMesh = glm::inverse(globTransforms[meshIndx]);
    std::vector<glm::mat4> result(joints.size());
    for(int i = 0; i < joints.size(); i++){
        result[i] = 
            inverseGlobMesh *
            globTransforms[joints[i]] *
            inverseBindings[i];
    }

    return result;
}

void WyvernModel::drawWyvern() const{
    for(const Primitive& primitive : primitives){
        glBindVertexArray(primitive.vao);
        glDrawElements(
            GL_TRIANGLES, 
            primitive.indxCount,
            GL_UNSIGNED_INT,
            nullptr
        );
    }
    glBindVertexArray(0);
}
