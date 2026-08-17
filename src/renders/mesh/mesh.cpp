#include "renders/mesh/mesh.h"

#include<glad/gl.h>

/*
    Summary: This class handles meshes.
    The init() function uploads a mesh from MeshData and generates
    the proper VAO, VBO and EBO, which are instead updated if they exist
    and init() is called again. The drawMesh() function then binds VAO and
    draws the triangle polygons
*/
Mesh::~Mesh(){
    if(vao){
        glDeleteVertexArrays(1, &vao);
    }

    if(vbo){
        glDeleteBuffers(1, &vbo);
    }

    if(ebo){
        glDeleteBuffers(1, &ebo);
    }
}

void Mesh::init(const MeshData& mesh){
    indxNum = static_cast<int>(mesh.indices.size());

    if(!vao){
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
    }

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER, 
        static_cast<GLsizeiptr>(mesh.vertices.size() * sizeof(Vert)), 
        mesh.vertices.data(), GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, 
        static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(uint32_t)), 
        mesh.indices.data(), GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)offsetof(Vert, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)offsetof(Vert, normal));

    glBindVertexArray(0);
}

void Mesh::drawMesh() const{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indxNum, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}