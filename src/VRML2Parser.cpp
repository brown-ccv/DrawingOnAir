#include <TextFileReader.H>
#include "VRML2Parser.H"

using namespace G3D;
bool
VRML2Parser::loadFromFile(const std::string &filename, 
                          Array<Vector3> &vertices, 
                          Array<int> &indices,
                          double uniformScale)
{
  MinVR::TextFileReader tf(filename);
  std::string text;
  tf.getTextUntilToken("point [", text, true);
  tf.getTextUntilToken("\n", text, true);
  
  int lastPerc = 0;
  bool done = false;
  while (!done) {
    std::string line;
    tf.getNextLine(line);
    //cout << line << endl;
    if (line.find("]") == line.npos) {
      float n1,n2,n3;
      sscanf(line.c_str(), "%f %f %f", &n1, &n2, &n3);
      Vector3 v(n1,n2,n3);
      vertices.append(uniformScale*v);
    }
    else {
      done = true;
    }
    int p = iRound(tf.getFilePosAsPercentage());
    if (p > lastPerc) {
      //cout << p << "%" << endl;
      lastPerc = p;
    }
  }
  
  tf.getTextUntilToken("coordIndex [", text);
  tf.getTextUntilToken("\n", text, true);

  int i=0;
  lastPerc = 0;
  done = false;
  while (!done) {
    std::string line;
    tf.getNextLine(line);
    if (line.find("]") == line.npos) {
      int n1,n2,n3;
      sscanf(line.c_str(), "%d, %d, %d", &n1, &n2, &n3);
      //cout << line << endl;
      //cout << "nums: " << n1 << " " << n2 << " " << n3 << endl;
      Array<int> face;
      face.append(n1);
      face.append(n2);
      face.append(n3);
      if ((face[0] < vertices.size()) && 
          (face[1] < vertices.size()) && 
          (face[2] < vertices.size()) &&
          (face[0] >= 0) && 
          (face[1] >= 0) && 
          (face[2] >= 0)) {
        indices.append(face);
      }
    }
    else {
      done = true;
    }
    int p = iRound(tf.getFilePosAsPercentage());
    if (p > lastPerc) {
      //cout << p << "%" << endl;
      lastPerc = p;
    }
    i++;
  }

  return true;
}




ArticulatedModel::Ref
VRML2Parser::createArticulatedModelFromFile(const std::string &filename, double uniformScale)
{
  MeshAlg::Geometry geom;
  Array<int> indices;
  if (!loadFromFile(filename, geom.vertexArray, indices, uniformScale)) {
    return NULL;
  }

  // Remove redundant vertices via G3D's weld routine
  Array<Vector3> newVerts;
  Array<int> toNew, toOld;
  MeshAlg::computeWeld(geom.vertexArray, newVerts, toNew, toOld);
  //cout << geom.vertexArray.size() << " " << newVerts.size() << endl;
  Array<int> newIndices;
  for (int i=0;i<indices.size();i++) {
    newIndices.append(toNew[indices[i]]);
  }
  geom.vertexArray = newVerts;
  indices = newIndices;

  MeshAlg::computeNormals(geom, indices);

  ArticulatedModel::Ref model = ArticulatedModel::createEmpty();
  model->name = filename;
  ArticulatedModel::Part& part = model->partArray.next();
  part.name = "root";
  part.geometry = geom;
  part.indexArray = indices;
  ArticulatedModel::Part::TriList::Ref triList = part.triList.next();
  triList->indexArray = part.indexArray;
  triList->twoSided = false;
  triList->material = Material::createDiffuse(Color3::white());
  //->diffuse.constant = Color3::white();

  part.updateVAR();
  //part.updateShaders();
  
  return model;
}

                                            
SMeshRef
VRML2Parser::createSMeshFromFile(const std::string &filename, double uniformScale)
{
  MeshAlg::Geometry geom;
  Array<int> indices;
  if (!loadFromFile(filename, geom.vertexArray, indices, uniformScale)) {
    return NULL;
  }

  // Remove redundant vertices via G3D's weld routine
  Array<Vector3> newVerts;
  Array<int> toNew, toOld;
  MeshAlg::computeWeld(geom.vertexArray, newVerts, toNew, toOld);
  Array<int> newIndices;
  for (int i=0;i<indices.size();i++) {
    newIndices.append(toNew[indices[i]]);
  }
  geom.vertexArray = newVerts;
  indices = newIndices;

  MeshAlg::computeNormals(geom, indices);

  //cout << "# Vertices " << newVerts.size() << endl;
  //cout << "# Indices " << newIndices.size() << endl;

  SMeshRef mesh = new SMesh(geom.vertexArray, geom.normalArray, indices);
  return mesh;
}

bool
VRML2Parser::loadVINFromFile(const std::string &filename, 
                           Array<Vector3> &vertices, 
                           Array<int> &indices,
						   Array<Vector3> &normals,
                           double uniformScale)
{
  MinVR::TextFileReader tf(filename);
  std::string text;
  tf.getTextUntilToken("point [", text, true);
  tf.getTextUntilToken("\n", text, true);
  
  int lastPerc = 0;
  bool done = false;
  while (!done) {
    std::string line;
    tf.getNextLine(line);
    //cout << line << endl;
    if (line.find("]") == line.npos) {
      float n1,n2,n3;
      sscanf(line.c_str(), "%f %f %f", &n1, &n2, &n3);
      Vector3 v(n1,n2,n3);
      vertices.append(uniformScale*v);
    }
    else {
      done = true;
    }
    int p = iRound(tf.getFilePosAsPercentage());
    if (p > lastPerc) {
      //cout << p << "%" << endl;
      lastPerc = p;
    }
  }
  
  tf.getTextUntilToken("coordIndex [", text);
  tf.getTextUntilToken("\n", text, true);

  int i=0;
  lastPerc = 0;
  done = false;
  while (!done) {
    std::string line;
    tf.getNextLine(line);
    if (line.find("]") == line.npos) {
      int n1,n2,n3;
      sscanf(line.c_str(), "%d, %d, %d", &n1, &n2, &n3);
      //cout << line << endl;
      //cout << "nums: " << n1 << " " << n2 << " " << n3 << endl;
      Array<int> face;
      face.append(n1);
      face.append(n2);
      face.append(n3);
      if ((face[0] < vertices.size()) && 
          (face[1] < vertices.size()) && 
          (face[2] < vertices.size()) &&
          (face[0] >= 0) && 
          (face[1] >= 0) && 
          (face[2] >= 0)) {
        indices.append(face);
      }
    }
    else {
      done = true;
    }
    int p = iRound(tf.getFilePosAsPercentage());
    if (p > lastPerc) {
      //cout << p << "%" << endl;
      lastPerc = p;
    }
    i++;
  }

  tf.getTextUntilToken("Normal", text, true);
  tf.getTextUntilToken("vector [", text, true);
  tf.getTextUntilToken("\n", text, true);
  
  lastPerc = 0;
  done = false;
  while (!done) {
    std::string line;
    tf.getNextLine(line);
    //cout << line << endl;
    if (line.find("]") == line.npos) {
      float n1,n2,n3;
      sscanf(line.c_str(), "%f %f %f", &n1, &n2, &n3);
      Vector3 v(n1,n2,n3);
      normals.append(v);
    }
    else {
      done = true;
    }
    int p = iRound(tf.getFilePosAsPercentage());
    if (p > lastPerc) {
      //cout << p << "%" << endl;
      lastPerc = p;
    }
  }
 
  return true;
}


SMeshRef
VRML2Parser::createSMeshWithNormsFromFile(const std::string &filename,
                                      double uniformScale)
{

  Array<Vector3> vertices;
  Array<Vector3> normals;
  Array<int> indices;
  if (!loadVINFromFile(filename, vertices, indices, normals, uniformScale)) {
    return NULL;
  }

  SMeshRef mesh = new SMesh(vertices, normals, indices);
  return mesh;
}


