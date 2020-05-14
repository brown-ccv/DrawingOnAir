/**
   \author Daniel Keefe (dfk)
   \file   VRML2Parser.H
   
   \brief A home brew VRML 2.0 parser that handles a very very small
   subset of the VRML 2.0 spec.  This assumes each file has one model
   specified in it and extracts the model's geometry only.
*/

#ifndef VRML2PARSER_H
#define VRML2PARSER_H

//#include <VRBase.H>
#include <SMesh.H>
#include <TexPerFrameSMesh.H>

#include <fstream>


typedef G3D::ReferenceCountedPointer<class VRML2Parser> VRML2ParserRef;

/**
*/
class VRML2Parser : public G3D::ReferenceCountedObject
{
public:
  static bool loadFromFile(const std::string &filename, 
                           G3D::Array<G3D::Vector3> &vertices, 
                           G3D::Array<int> &indices,
                           double uniformScale=1.0);
  
  static G3D::ArticulatedModel::Ref createArticulatedModelFromFile(const std::string &filename,
                                                            double uniformScale=1.0);
  static SMeshRef createSMeshFromFile(const std::string &filename,
                                      double uniformScale=1.0);
                                             
  static bool loadVINFromFile(const std::string &filename, 
                           G3D::Array<G3D::Vector3> &vertices, 
                           G3D::Array<int> &indices,
						   G3D::Array<G3D::Vector3> &normals,
                           double uniformScale=1.0);

  static SMeshRef createSMeshWithNormsFromFile(const std::string &filename,
                                      double uniformScale=1.0);


};


#endif

