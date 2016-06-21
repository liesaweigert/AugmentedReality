////
//// Created by Liesa  on 19/06/16.
////
//
//#include "AssimpModel.h"
//
//bool AssimpModel::LoadModelFromFile(const char* path)
//{
//    // Create an instance of the Importer class
//    Importer importer;
//
//    // And have it read the given file with some example postprocessing
//    // Usually - if speed is not the most important aspect for you - you'll
//    // probably request more postprocessing than we do in this example.
//    const aiScene* scene = importer.ReadFile( path,
//                                              aiProcess_CalcTangentSpace       |
//                                              aiProcess_Triangulate            |
//                                              aiProcess_JoinIdenticalVertices  |
//                                              aiProcess_SortByPType);
//
//    // If the import failed, report it
//    if( !scene)
//    {
//        std::cout << "ERROR: could not import model.\n";
//        return false;
//    }
//
//    return true;
//}