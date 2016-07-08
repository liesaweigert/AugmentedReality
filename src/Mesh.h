//
// Created by Liesa  on 06/07/16.
//

#ifndef AR_MESH_H
#define AR_MESH_H
#include <GL/glew.h>
#include <vector>
#include <Eigen/Dense>

struct Index{
    unsigned int vertex_id;
    unsigned int tex_coord_id;
    unsigned int normal_id;
};


class Mesh {
    std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f>> vertices;
    std::vector<Eigen::Vector2f, Eigen::aligned_allocator<Eigen::Vector2f>> tex_coords;
    std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f>> normals;
    std::vector<Index> ids;
    GLuint tex;

public:
    Mesh(char* objfile, char* texturefile, int scale);
    void drawMesh(Eigen::Matrix4f marker_matrix, float falling);
};


#endif //AR_MESH_H
