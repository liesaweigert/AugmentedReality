//
// Created by Liesa  on 06/07/16.
//

#include "Mesh.h"

#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>

using namespace std;
using namespace Eigen;


Mesh::Mesh(char *objfile, char* texturefile, int scale) {

    ifstream in(objfile);

    if(!in){
        cout << "ERROR: could not load objectfile!\n";
        return;
    }

    Vector3f mean = {0.0, 0.0, 0.0};

    string line;
    while(getline(in, line)){
        if(line.substr(0,2) == "v "){ //Vertex
            stringstream ss(line.substr(2));
            Vector3f v;
            ss >> v[0] >> v[1] >> v[2];
            v = v / scale;
            mean += v;
            vertices.push_back(v);
        } else if (line.substr(0, 3) == "vt "){ //Texture coordinate
            stringstream ss(line.substr(3));
            Vector2f t;
            ss >> t[0] >> t[1];
            t[1] = 1 - t[1];
            tex_coords.push_back(t);
        } else if (line.substr(0, 2) == "f "){ //Faces
            stringstream ss(line.substr(2));
            string f[3];
            ss >> f[0] >> f[1] >> f[2];
            for (int j = 0; j < 3; j++){
                string delimiter = "/";
                int pos = f[j].find(delimiter);
                Index i;
                i.vertex_id = stoi(f[j].substr(0, pos)) - 1;
                i.tex_coord_id = stoi(f[j].substr(pos + 1)) - 1;
                ids.push_back(i);
            }
        }
    }

    mean = mean / vertices.size();
    for (int i = 0; i < vertices.size(); i++){
        vertices[i] = vertices[i] - mean;
    }

    for (auto it = ids.begin(); it != ids.end(); it+= 3){
        Vector3f vertex1 = vertices[it->vertex_id];
        Vector3f vertex2 = vertices[next(it, 1)->vertex_id];
        Vector3f vertex3 = vertices[next(it, 2)->vertex_id];

        Vector3f vector1 = vertex2 - vertex1;
        Vector3f vector2 = vertex3 - vertex1;
        Vector3f normal = vector1.cross(vector2);
        normal.normalize();

        normals.push_back(normal);
        int current = normals.size() - 1;
        it->normal_id = next(it, 1)->normal_id = next(it, 2)->normal_id = current;
    }




    cv::Mat image = cv::imread(texturefile);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, image.data);

    if(glGetError()){
        cout << "ERROR: Something went wrong with the texture!\n";
    }
}

void Mesh::drawMesh(Eigen::Matrix4f marker_matrix, float falling) {

    glMatrixMode(GL_MODELVIEW);

    //transform to marker
    glLoadTransposeMatrixf(marker_matrix.data());
    glTranslatef(0.0, 0.0, falling);
    float angle = abs(1500 * falling);
    std::cout << "The rotation angle is: " << angle << "\n";
    glRotatef(angle, 0.0, 1.0, 0.0);
    glRotatef(angle, 1.0, 0.0, 0.0);

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, tex);

    glBegin(GL_TRIANGLES);
    glColor3f(1.0, 1.0, 1.0);
    for (auto it = ids.begin(); it != ids.end(); ++it){
        glVertex3f(vertices[it->vertex_id].x(), vertices[it->vertex_id].y(), vertices[it->vertex_id].z());
        glNormal3f(-normals[it->normal_id].x(), -normals[it->normal_id].y(), -normals[it->normal_id].z());
        glTexCoord2f(tex_coords[it->tex_coord_id].x(), tex_coords[it->tex_coord_id].y());
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);

}
