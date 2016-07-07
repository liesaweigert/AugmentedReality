//
//  ObjectParser.cpp
//  AR_Project
//
//  Created by Verena Dogeanu on 07/07/16.
//  Copyright © 2016 Verena Dogeanu. All rights reserved.
//

#include <stdio.h>
#include "ObjectParser.h"

void drawObject(string objName){
    
    char *c = strdup(objName.c_str());
    
    loadObject(c);
}


void loadObject(char *fname){
    
    string str;
    
    ifstream in(fname);
    
    if (!in) {
        printf("can't open file %s\n", fname);
        exit(1);
    }
    
    vector<Vector3f> vertices;
    vector<Vector3f> normals;
    vector<Vector2f> texturecoordinates;
    vector<Index> indices;
    
    while(getline(in,str))
    {
        if(str.substr(0,2) == "v "){
            stringstream ss(str.substr(2));
            Vector3f v;
            
            ss >> v[0] >> v[1] >> v[2];
            
            vertices.push_back(v);
            
        } else if (str.substr(0,3) == "vt "){
            stringstream ss(str.substr(3));
            Eigen::Vector2f t;
            
            ss >> t[0] >> t[1];
            t[1] = 1-t[1];
            
            texturecoordinates.push_back(t);
            
        } else if (str.substr(0,3) == "vn "){
            stringstream ss(str.substr(3));
            Vector3f n;
            
            ss >> n[0] >> n[1] >> n[2];
            
            normals.push_back(n);
            
        } else if (str.substr(0,2) == "f "){
            stringstream ss(str.substr(2));
            Vector2f f;
            Index i;
            char discard;
            
            ss>>f[0]>>discard>>f[1];
            i.vertexIdx_1 = f[0] - 1;
            i.texIdx_1 = f[1] - 1;
            
            ss>>f[0]>>discard>>f[1];
            i.vertexIdx_2 = f[0] - 1;
            i.texIdx_2 = f[1] - 1;
            
            ss>>f[0]>>discard>>f[1];
            i.vertexIdx_3 = f[0] - 1;
            i.texIdx_3 = f[1] - 1;
            
            indices.push_back(i);
        }
    }
    
    renderObject(indices, vertices, normals, texturecoordinates);
}

void renderObject(vector<Index> idx, vector<Vector3f> v, vector<Vector3f> n, vector<Vector2f> uv){
    
    glBegin(GL_TRIANGLES);
    
    for (auto it = idx.begin(); it != idx.end(); ++it) {
        glVertex3f(v[it->vertexIdx_1].x(), v[it->vertexIdx_1].y(), v[it->vertexIdx_1].z());
        glVertex3f(v[it->vertexIdx_2].x(), v[it->vertexIdx_2].y(), v[it->vertexIdx_2].z());
        glVertex3f(v[it->vertexIdx_3].x(), v[it->vertexIdx_3].y(), v[it->vertexIdx_3].z());
        
        glTexCoord2f(uv[it->texIdx_1].x(), uv[it->texIdx_1].y());
        glTexCoord2f(uv[it->texIdx_2].x(), uv[it->texIdx_2].y());
        glTexCoord2f(uv[it->texIdx_3].x(), uv[it->texIdx_3].y());
        

    }
    glEnd();
    
}