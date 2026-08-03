#pragma once

#include <string>
#include <cstdint>
#include <glm/glm.hpp>

class Shader{
    public:
        uint32_t program_ = 0;
        int uniLocation(const std::string& name) const;

        Shader(const std::string& fragPath, const std::string& vertPath);
        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        void use() const;

        void uniSet(const std::string& loc, const glm::vec2& val) const;
        void uniSet(const std::string& loc, const glm::vec3& val) const;
};