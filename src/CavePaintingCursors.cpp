
#include "CavePaintingCursors.H"
#include "Shadows.H"
#include <ConfigVal.H>

using namespace G3D;
namespace DrawOnAir {

CavePaintingCursors::CavePaintingCursors(GfxMgrRef gfxMgr, 
                     EventMgrRef eventMgr, BrushRef brush,
                                         Array<std::string> handTrackerTriggers)
{
  _brush = brush;
  _gfxMgr = gfxMgr;
  _brushCursor = DEFAULT_CURSOR;
  _handCursor = DEFAULT_CURSOR;
  _sphereTipRad = 0.015;
  _lastGoodUpVec = Vector3(0,0,1);
  _lastGoodRightVec = Vector3(1,0,0);
  _lastRight = Vector3(1,0,0);

  gfxMgr->addDrawCallback(this, &CavePaintingCursors::draw);

  FsaRef fsa = new Fsa("CavePaintingCursors");
  fsa->addState("Start");
  fsa->addArc("HandMove", "Start", "Start", handTrackerTriggers);
  fsa->addArcCallback("HandMove", this, &CavePaintingCursors::moveHandTracker);
  eventMgr->addFsaRef(fsa);
}

CavePaintingCursors::~CavePaintingCursors()
{
}

void
CavePaintingCursors::moveHandTracker(MinVR::EventRef e)
{
  _handFrame = e->getCoordinateFrameData();
}


void
CavePaintingCursors::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  double rad = MinVR::ConfigVal("CavePaintingCursors_HandCursorSize",0.05,false);
  double wrad = MinVR::ConfigVal("CavePaintingCursors_BlendToolSize",0.05,false);
  double len = MinVR::ConfigVal("Charcoal_Length", 0.15, false);
  Color3 brushColor = Color3(0.82, 0.82, 0.61);
  Color3 flatBrushColor = Color3(1.0, 1.0, 0.8);
  Color3 handColor = Color3(0.61, 0.92, 0.72);
  double guideRad = MinVR::ConfigVal("Guideline_Rad",0.001, false);
  int    guideNFaces = MinVR::ConfigVal("Guideline_NFaces", 10, false);
  double glr = MinVR::ConfigVal("GuideLineTexRepeat", 0.015, false);

  // ---  Brush Cursor  ---
  if (_brushCursor == DEFAULT_CURSOR) {

    drawPressureMeter();

    if (_brush->state->brushInterface.find("TapeDraw") != _brush->state->brushInterface.npos) {
      drawSphere(rd, _brush->state->physicalFrameInRoomSpace, rad/3.0, Color3(0.61,0.72,0.92));

      Vector3 start,end;
      if (_brush->state->brushInterface.find("Reverse") == _brush->state->brushInterface.npos) {
        start = _brush->state->frameInRoomSpace.translation;
        end = _handFrame.translation;
      }
      else { // Reverse Versions
        start = _brush->state->frameInRoomSpace.translation;
        Vector3 dir = (_brush->state->frameInRoomSpace.translation - _handFrame.translation).unit();
        end = _brush->state->frameInRoomSpace.translation + 0.5*dir;
      }
      drawLineAsCylinder(start, end, guideRad, guideRad, rd, Color3::white(), guideNFaces, 
                         _gfxMgr->getTexture("GuideLineTex"), (start-end).length()/glr);
    }
    else if (_brush->state->brushInterface == "ForceOneHandDraw") {
      // TODO: draw guides for this version as well
      Vector3 start = _brush->state->frameInRoomSpace.translation;
      Vector3 dir = _brush->state->frameInRoomSpace.vectorToWorldSpace(Vector3(0,1,0)).unit();
      Vector3 end = _brush->state->frameInRoomSpace.translation + 0.5*dir;
      drawLineAsCylinder(start, end, guideRad, guideRad, rd, Color3::white(), guideNFaces, 
                         _gfxMgr->getTexture("GuideLineTex"), (start-end).length()/glr);
    }

    if (_brush->state->brushModel == BrushState::BRUSHMODEL_DEFAULT) {
      drawFlatBrush(rd, _brush->state->frameInRoomSpace, 0.03, flatBrushColor, _gfxMgr->getTexture("BrushTexture"));
    }
    else if (_brush->state->brushModel == BrushState::BRUSHMODEL_CAVEPAINTING) {
      drawFlatBrush(rd, _brush->state->frameInRoomSpace, _brush->state->size, flatBrushColor, _gfxMgr->getTexture("BrushTexture"));
    }
    else if (_brush->state->brushModel == BrushState::BRUSHMODEL_ROUND) {
      drawRoundBrush(rd, _brush->state->frameInRoomSpace);
    }
    else if (_brush->state->brushModel == BrushState::BRUSHMODEL_CHARCOAL_W_HEURISTIC) {
      drawRoundBrush(rd, _brush->state->frameInRoomSpace);
    }
  }
  else if (_brushCursor == POINTER_CURSOR) {
    drawArrow(rd, _brush->state->frameInRoomSpace, MinVR::ConfigVal("Arrow_Length",0.15,false), brushColor);
  }
  else if (_brushCursor == DELETE_CURSOR) {
    drawArrow(rd, _brush->state->frameInRoomSpace, MinVR::ConfigVal("Arrow_Length",0.15,false), Color3::red());
  }
  else if (_brushCursor == SPHERETIP_CURSOR) {
    drawSphereTip(rd, _brush->state->frameInRoomSpace);
  }


  // ---  Hand Cursor  ---
  if (_handCursor == DEFAULT_CURSOR) {
    drawSphere(rd, _handFrame, rad, handColor);
  }
  else if (_handCursor == POINTER_CURSOR) {
    drawArrow(rd, _handFrame, MinVR::ConfigVal("Arrow_Length",0.15,false), handColor);
  }
  else if (_handCursor == DELETE_CURSOR) {
    drawArrow(rd, _handFrame, MinVR::ConfigVal("Arrow_Length",0.15,false), Color3::red());
  }
}





void
CavePaintingCursors::drawActiveLayerStatus(RenderDevice *rd)
{
  // Draw a little status display for the active drawing layer
  Vector2 size = Vector2(200, 56);
  double textSize = 12;
  Vector2 pos  = Vector2(rd->width() - size[1] - size[0]/2.0, size[1]);
  GFontRef font = _gfxMgr->getDefaultFont();

  Color4 outlineCol  = Color4(1.0,1.0,1.0,1.0);
  Color4 bgCol       = Color4(0.1,0.1,0.44,1.0);
  Color4 barCol      = Color4(0.61,0.72,0.92,1.0);
  Color4 titleCol    = Color4(1.0,1.0,1.0,1.0);
  Color4 percCol     = Color4(0.8,0.22,0.0,1.0);
  
  Vector2 topl = Vector2(pos[0]-size[0]/2.0, pos[1]-size[1]/2.0);
  Vector2 topr = Vector2(pos[0]+size[0]/2.0, pos[1]-size[1]/2.0);
  Vector2 botl = Vector2(pos[0]-size[0]/2.0, pos[1]+size[1]/2.0);
  Vector2 botr = Vector2(pos[0]+size[0]/2.0, pos[1]+size[1]/2.0);

  rd->push2D();
  rd->setTexture(0,NULL);
  rd->setBlendFunc(RenderDevice::BLEND_ONE, RenderDevice::BLEND_ZERO);

  // outline
  rd->setColor(outlineCol);
  rd->beginPrimitive(PrimitiveType::QUADS);
  rd->sendVertex(topl);
  rd->sendVertex(botl);
  rd->sendVertex(botr);
  rd->sendVertex(topr);
  rd->endPrimitive();

  // background
  rd->setColor(bgCol);
  rd->beginPrimitive(PrimitiveType::QUADS);
  rd->sendVertex(Vector2(topl[0] + 2, topl[1] + 2));
  rd->sendVertex(Vector2(botl[0] + 2, botl[1] - 2));
  rd->sendVertex(Vector2(botr[0] - 2, botr[1] - 2));
  rd->sendVertex(Vector2(topr[0] - 2, topr[1] + 2));
  rd->endPrimitive();
  
  // title
  font->draw2D(rd, "Frame #" + MinVR::intToString(_brush->state->frameIndex),
               pos - Vector2(0, 0.5*textSize), textSize, titleCol, Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
  font->draw2D(rd, "Active Drawing Layer #" + MinVR::intToString(_brush->state->layerIndex),
               pos + Vector2(0, 0.5*textSize), textSize, titleCol, Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);

  rd->pop2D();
}



void
CavePaintingCursors::drawSphere(RenderDevice *rd, const CoordinateFrame &frame, double rad, Color3 color)
{
  rd->setTexture(0, NULL);
  Sphere s = Sphere(frame.translation, rad);
  Draw::sphere(s, rd, color, Color4::clear());

  // shadow
  if (getShadowsOn()) {
    pushShadowState(rd);
    Draw::sphere(s, rd, rd->color(), Color4::clear());
    popShadowState(rd);
  }
}

void
CavePaintingCursors::drawWireSphere(RenderDevice *rd, const CoordinateFrame &frame, double rad, Color3 color)
{
  rd->setTexture(0, NULL);
  Sphere s = Sphere(frame.translation, rad);
  Draw::sphere(s, rd, Color4::clear(), color);

  // shadow
  if (getShadowsOn()) {
    pushShadowState(rd);
    Draw::sphere(s, rd, Color4::clear(), rd->color());
    popShadowState(rd);
  }
}

void
CavePaintingCursors::drawArrow(RenderDevice *rd, const CoordinateFrame &frame, double size, Color3 color)
{
  rd->pushState();
  rd->setObjectToWorldMatrix(frame);
  rd->setTexture(0, NULL);
  Draw::arrow(Vector3(0,0,size), Vector3(0,0,-size), rd, color, size);

  if (getShadowsOn()) {
    pushShadowState(rd);
    // BUG: Draw::arrow calls G3D's line draw routine which inverts rd->getCameraToWorld which crashes
    // when the shadow projection matrix is applied.
    //Draw::arrow(Vector3(0,0,size), Vector3(0,0,-size), rd, rd->color(), size);
    popShadowState(rd);
  }

  rd->popState();
}

void
CavePaintingCursors::drawCylinder(RenderDevice *rd, const CoordinateFrame &frame, double size, Color3 color)
{
  rd->pushState();

  rd->setObjectToWorldMatrix(frame);
  rd->setTexture(0, NULL);
  rd->setColor(color);

  double radius = _brush->state->width/2.0;
  Vector3 top(0, 0, size);

  // Cylinder faces
  rd->beginPrimitive(PrimitiveType::QUAD_STRIP);
  for (int y = 0; y <= 16; y++) {
    const double yaw0 = -y * twoPi() / 16.0;
    Vector3 v0 = Vector3(cos(yaw0), sin(yaw0), 0);
    rd->setNormal(v0);
    rd->sendVertex(v0 * radius);
    rd->sendVertex(v0 * radius + top);
  }
  rd->endPrimitive();

  // Cylinder top
  rd->beginPrimitive(PrimitiveType::TRIANGLE_FAN);
  rd->setNormal(top.unit());
  rd->sendVertex(top);
  for (int y = 0; y <= 16; y++) {
    const double yaw0 = y * twoPi() / 16.0;
    Vector3 v0 = Vector3(cos(yaw0), sin(yaw0), 0);
    rd->sendVertex(v0 * radius + top);
  }
  rd->endPrimitive();

  // Cylinder bottom
  rd->beginPrimitive(PrimitiveType::TRIANGLE_FAN);
  rd->setNormal(-top.unit());
  rd->sendVertex(Vector3::zero());
  for (int y = 0; y <= 16; y++) {
    const double yaw0 = -y * twoPi() / 16.0;
    Vector3 v0 = Vector3(cos(yaw0), sin(yaw0), 0);
    rd->sendVertex(v0 * radius);
  }
  rd->endPrimitive();

  rd->popState();
}

void
CavePaintingCursors::drawRoundBrush(RenderDevice *rd, const CoordinateFrame &frame)
{
  rd->pushState();

  double size = MinVR::ConfigVal("RoundBrush_Length", 0.1, false);
  double radius = MinVR::ConfigVal("RoundBrush_Radius", size/20.0, false);
  Color3 color = MinVR::ConfigVal("RoundBrush_Color", Color3(0.82, 0.82, 0.61), false);
  
  Vector3 top(0, 0, size);
  Vector3 coneTop(0,0,size/5.0);

  int smax = 1;
  if (getShadowsOn()) {
    smax = 2;
  }
    
  for (int s=0;s<smax;s++) {
    rd->setObjectToWorldMatrix(frame);
    if (s==0) {
      rd->setColor(color);
      rd->setTexture(0, _gfxMgr->getTexture("RoundBrushTexture"));
    }
    else {
      pushShadowState(rd);
      rd->setTexture(0, NULL);
    }

    // Cylinder faces
    rd->beginPrimitive(PrimitiveType::QUAD_STRIP);
    for (int y = 0; y <= 16; y++) {
      const double yaw0 = -y * twoPi() / 16.0;
      Vector3 v0 = Vector3(cos(yaw0), sin(yaw0), 0);
      rd->setNormal(v0);
      rd->setTexCoord(0, Vector2(0, (float)y/16.0));
      rd->sendVertex(v0 * 0.1*radius);
      rd->setTexCoord(0, Vector2(1, (float)y/16.0));
      rd->sendVertex(v0 * radius + coneTop);
    }
    rd->endPrimitive();

    // Cylinder faces
    rd->beginPrimitive(PrimitiveType::QUAD_STRIP);
    for (int y = 0; y <= 16; y++) {
      const double yaw0 = -y * twoPi() / 16.0;
      Vector3 v0 = Vector3(cos(yaw0), sin(yaw0), 0);
      rd->setNormal(v0);
      rd->setTexCoord(0, Vector2(1, (float)y/16.0));
      rd->sendVertex(v0 * radius + coneTop);
      rd->setTexCoord(0, Vector2(0, (float)y/16.0));
      rd->sendVertex(v0 * radius + top);
    }
    rd->endPrimitive();
    
    // top
    rd->beginPrimitive(PrimitiveType::TRIANGLE_FAN);
    rd->setNormal(top.unit());
    rd->sendVertex(top);
    for (int y = 0; y <= 16; y++) {
      const double yaw0 = y * twoPi() / 16.0;
      Vector3 v0 = Vector3(cos(yaw0), sin(yaw0), 0);
      rd->sendVertex(v0 * radius + top);
    }
    rd->endPrimitive();     

    if (s==1) {
      popShadowState(rd);
    }
  }

  rd->popState();


  // pressure bars
  double w = _brush->state->width;
  //if ((_brush->state->maxPressure != 0.0) && (_brush->state->pressure = 0.0)) {
  //  w = 0.0;
  //}
  double s = _brush->state->size;
  double r = MinVR::ConfigVal("PressureMeterSize", 0.02, false) / 10.0;
  Vector3 tip = frame.translation;

  rd->enableLighting();
  drawLineAsCylinder(tip + frame.vectorToWorldSpace(Vector3(0.5*w, 0.0, 0.0)),
                     tip + frame.vectorToWorldSpace(Vector3(-0.5*w, 0.0, 0.0)),
                     r, r, rd, Color3::yellow(), 6, NULL, 1.0);
  /**
  drawLineAsCylinder(tip + frame.vectorToWorldSpace(Vector3(0.5*s, 1.5*r, 0.0)),
                     tip + frame.vectorToWorldSpace(Vector3(0.5*s, -1.5*r, 0.0)),
                     0.5*r, 0.5*r, rd, Color3::yellow(), 6);

  drawLineAsCylinder(tip + frame.vectorToWorldSpace(Vector3(-0.5*s, 1.5*r, 0.0)),
                     tip + frame.vectorToWorldSpace(Vector3(-0.5*s, -1.5*r, 0.0)),
                     0.5*r, 0.5*r, rd, Color3::yellow(), 6);
  **/
}


void
CavePaintingCursors::drawSphereTip(RenderDevice *rd, const CoordinateFrame &frame)
{
  Color3 brushColor = Color3(0.82, 0.82, 0.61);
  drawRoundBrush(rd, frame);
  drawWireSphere(rd, frame, _sphereTipRad, brushColor);
}


void 
CavePaintingCursors::drawPlane(RenderDevice *rd, const CoordinateFrame &frame, double w, double h, Color3 color)
{
  rd->pushState();
  rd->setObjectToWorldMatrix(frame);
  rd->setTexture(0, NULL);
  rd->setColor(color);
  rd->setObjectToWorldMatrix(frame);
  rd->setCullFace(RenderDevice::CULL_NONE);
  rd->beginPrimitive(PrimitiveType::QUADS);
  rd->setNormal(Vector3(0,0,1));
  rd->sendVertex(Vector3(-w/2.0, h/2.0, 0));
  rd->sendVertex(Vector3(-w/2.0, -h/2.0, 0));
  rd->sendVertex(Vector3(w/2.0, -h/2.0, 0));
  rd->sendVertex(Vector3(w/2.0, h/2.0, 0));
  rd->endPrimitive();
  rd->popState();
}

void
CavePaintingCursors::drawBlendTool(RenderDevice *rd, const CoordinateFrame &frame, Color3 color)
{
  rd->pushState();
  rd->setObjectToWorldMatrix(frame);
  rd->setTexture(0, NULL);

  double w = MinVR::ConfigVal("BlendToolWidth", 0.025, false)/2.0;
  double h = MinVR::ConfigVal("BlendToolHeight", 0.025, false)/2.0;
  double d = MinVR::ConfigVal("BlendToolDepth", 0.005, false)/2.0;
  Box b = Box(Vector3(-w, -h, -d), Vector3(w, h, d));
  Draw::box(b, rd, Color4::clear(), color);

  rd->popState();
}

void
CavePaintingCursors::drawFlatBrush(RenderDevice *rd, const CoordinateFrame &frame, double width, Color3 color, Texture::Ref texture)
{
  if (MinVR::ConfigVal("AlwaysDrawRoundBrush", 0, false)) {
    drawRoundBrush(rd, frame);
    return;
  }

  // The first time through, store geometry in G3D VARs..
  if (_brushIndices.size() == 0) {
    Array<Vector3> vertices;
    Array<Vector2> texCoords;
  
    vertices.append(Vector3(0.5,0,0));    
    texCoords.append(Vector2(1.0,0.0));
    
    vertices.append(Vector3(-0.5,0,0));                         
    texCoords.append(Vector2(0.0,0.0));
      
    vertices.append(Vector3(0.5,0.1,0.25));  
    texCoords.append(Vector2(1.0,0.25));
    
    vertices.append(Vector3(-0.5,0.1,0.25));
    texCoords.append(Vector2(0.0,0.25));
    
    vertices.append(Vector3(0.5,0.1,0.75));
    texCoords.append(Vector2(1.0,0.75));
    
    vertices.append(Vector3(-0.5,0.1,0.75));
    texCoords.append(Vector2(0.0,0.75));
        
    vertices.append(Vector3(0.1,0.06,1.0));
    texCoords.append(Vector2(0.6,1.0));
        
    vertices.append(Vector3(-0.1,0.06,1.0));
    texCoords.append(Vector2(0.4,1.0));
    
    vertices.append(Vector3(0.15,0.1,1.75));
    texCoords.append(Vector2(0.65,1.75));
    
    vertices.append(Vector3(-0.15,0.1,1.75));
    texCoords.append(Vector2(0.35,1.75));
    
    vertices.append(Vector3(0,0,1.85));
    texCoords.append(Vector2(0.5,1.85));
    
    
    vertices.append(Vector3(0.5,-0.1,0.25));
    texCoords.append(Vector2(1.0,0.25));
    
    vertices.append(Vector3(-0.5,-0.1,0.25));
    texCoords.append(Vector2(0.0,0.25));
  
    vertices.append(Vector3(0.5,-0.1,0.75));
    texCoords.append(Vector2(1.0,0.75));
    
    vertices.append(Vector3(-0.5,-0.1,0.75));
    texCoords.append(Vector2(0.0,0.75));
    
    vertices.append(Vector3(0.1,-0.06,1.0));
    texCoords.append(Vector2(0.6,1.0));
    
    vertices.append(Vector3(-0.1,-0.06,1.0));
    texCoords.append(Vector2(0.4,1.0));
    
    vertices.append(Vector3(0.15,-0.1,1.75));
    texCoords.append(Vector2(0.65,1.75));
    
    vertices.append(Vector3(-0.15,-0.1,1.75));
    texCoords.append(Vector2(0.35,1.75));
    

    // top
    _brushIndices.append(0,1,2);
    _brushIndices.append(1,3,2);
  
    _brushIndices.append(2,3,4);
    _brushIndices.append(3,5,4);
  
    _brushIndices.append(4,5,6);
    _brushIndices.append(5,7,6);
  
    _brushIndices.append(6,7,8);
    _brushIndices.append(7,9,8);
  
    _brushIndices.append(8,9,10);
  
    // bottom
    _brushIndices.append(0,12,1);
    _brushIndices.append(11,12,0);
  
    _brushIndices.append(11,14,12);
    _brushIndices.append(13,14,11);
  
    _brushIndices.append(13,16,14);
    _brushIndices.append(15,16,13);
  
    _brushIndices.append(15,18,16);
    _brushIndices.append(17,18,15);
  
    _brushIndices.append(18,17,10);
  
    // one side
    _brushIndices.append(11,0,2);
  
    _brushIndices.append(11,2,4);
    _brushIndices.append(4,13,11);
  
    _brushIndices.append(13,4,6);
    _brushIndices.append(6,15,13);
  
    _brushIndices.append(15,6,8);
    _brushIndices.append(8,17,15);
  
    _brushIndices.append(17,8,10);
  
    // the other side
    _brushIndices.append(3,1,12);
  
    _brushIndices.append(3,12,14);
    _brushIndices.append(14,5,3);
  
    _brushIndices.append(5,14,16);
    _brushIndices.append(16,7,5);
  
    _brushIndices.append(7,16,18);
    _brushIndices.append(18,9,7);
  
    _brushIndices.append(9,18,10);


    Array<MeshAlg::Face> faces;
    for (int i=0;i<_brushIndices.size();i+=3) {
      MeshAlg::Face face;
      face.vertexIndex[0] = _brushIndices[i];
      face.vertexIndex[1] = _brushIndices[i+1];
      face.vertexIndex[2] = _brushIndices[i+2];
      faces.append(face);
    }

    Array<Vector3> faceNormals;
    MeshAlg::computeFaceNormals(vertices, faces, faceNormals);


    Array<G3D::uint32>  indices;
    Array<Vector3> newverts;
    Array<Vector3> normals;
    Array<Vector2> newTexCoords;
    for (int i=0;i<faces.size();i++) {
      newverts.append(vertices[faces[i].vertexIndex[0]]);
      newTexCoords.append(texCoords[faces[i].vertexIndex[0]]);
      normals.append(faceNormals[i]);
      indices.append(newverts.size()-1);
      newverts.append(vertices[faces[i].vertexIndex[1]]);
      newTexCoords.append(texCoords[faces[i].vertexIndex[1]]);
      normals.append(faceNormals[i]);
      indices.append(newverts.size()-1);
      newverts.append(vertices[faces[i].vertexIndex[2]]);
      newTexCoords.append(texCoords[faces[i].vertexIndex[2]]);
      normals.append(faceNormals[i]);
      indices.append(newverts.size()-1);
    }
    
    _brushIndices      = indices;

    // add 8 * number of individual VARs to be stored in the VAR array to the 
    // calculated size because the VAR arrays are byte aligned in memory.
    const int numVARs = 3;
    size_t sizeNeeded = (2*sizeof(Vector3) + sizeof(Vector2))*newverts.size() + 8*numVARs;
    _varArea           = VARArea::create(sizeNeeded, VARArea::WRITE_ONCE);
    _varBrushVerts     = VAR(newverts, _varArea);
    _varBrushTexCoords = VAR(newTexCoords, _varArea);
    _varBrushNormals   = VAR(normals, _varArea);
    debugAssert(_varBrushVerts.valid());
    debugAssert(_varBrushTexCoords.valid());
    debugAssert(_varBrushNormals.valid());
  }

  // Now, render using VARs..
  rd->pushState();
  rd->setObjectToWorldMatrix(
      frame * CoordinateFrame(Matrix3(width,0,0, 0,width,0, 0,0,width), Vector3::zero()));
  rd->setColor(color);
  rd->setTexture(0, texture);
  rd->setShadeMode(RenderDevice::SHADE_FLAT);
  rd->setSpecularCoefficient(0.0);
  rd->setShininess(0);
  
  rd->beginIndexedPrimitives();
  rd->setVertexArray(_varBrushVerts);
  rd->setNormalArray(_varBrushNormals);
  rd->setTexCoordArray(0, _varBrushTexCoords);
  rd->sendIndices(PrimitiveType::TRIANGLES, _brushIndices);
  rd->endIndexedPrimitives();
 
  rd->popState();
}

void
CavePaintingCursors::drawDrawingGuides()
{
  RenderDevice *rd = _gfxMgr->getRenderDevice();
  rd->pushState();

  Vector3 right;
  double width;
  _brush->state->getDrawParametersBasedOnBrushModel(right, width, _lastGoodUpVec, _lastGoodRightVec, _lastRight);
  _lastRight = right;

  // stick with the initial right vector for charcoal once we start drawing
  if ((_brush->state->brushModel == BrushState::BRUSHMODEL_CHARCOAL_W_HEURISTIC) &&
      (_brush->currentMark.notNull()) && (_brush->currentMark->getNumSamples() >= 1)) {  
    double dummy;
    _brush->currentMark->getBrushState(0)->getDrawParametersBasedOnBrushModel(right, dummy, _lastGoodUpVec, _lastGoodRightVec, _lastRight);
  }

  Vector3 perpUp = right.cross(_brush->state->drawingDir).unit();

  if (_brush->state->brushModel == BrushState::BRUSHMODEL_CHARCOAL_W_HEURISTIC) {  
    Vector3 tip = _brush->state->frameInRoomSpace.translation + width*right;
    Vector3 back = tip - 2.0*width*right;
    rd->pushState();
    rd->disableLighting();
    rd->setColor(Color3::green());
    rd->setLineWidth(6.0);
    rd->beginPrimitive(PrimitiveType::LINES);
    rd->sendVertex(tip);
    rd->sendVertex(back);
    rd->endPrimitive();
    rd->popState();
  }

  Vector3 perpSide = (right - right.dot(_brush->state->drawingDir)*_brush->state->drawingDir).unit();

  // Draw an indication of the surface that the brush draws on..
  // These are the 4 points that specify the extents of a quad that we
  // draw to indicate the surface.
  double dgw = MinVR::ConfigVal("DrawingGuide_Width", 1.0*_brush->state->size, false);
  double offsetAmt = 0.0; 
  Vector3 hl = _handFrame.translation - dgw*0.5*right + offsetAmt*_brush->state->drawingDir;
  Vector3 hr = hl + dgw*right;

  Vector3 bl = _brush->state->frameInRoomSpace.translation - dgw*0.5*right - 
    offsetAmt*_brush->state->drawingDir;
  Vector3 br = bl + dgw*right;

  double s = (hl - bl).magnitude() / (hl - hr).magnitude();

  rd->setCullFace(RenderDevice::CULL_NONE);
  rd->enableTwoSidedLighting();
  rd->setColor(Color3::white());
  rd->setAlphaTest(RenderDevice::ALPHA_GEQUAL, 0.05);

  // render the quad that indicates the drawing surface
  //rd->setTexture(0, _gfxMgr->getTexture("DrawPreviewGrid"));
  rd->setColor(Color3(0.1, 0.1, 0.1));
  rd->beginPrimitive(PrimitiveType::LINE_STRIP);
  rd->setNormal((hr-br).cross(bl-br).unit());
  rd->setTexCoord(0, Vector2(0,0));
  rd->sendVertex(br);
  rd->setTexCoord(0, Vector2(s,0));
  rd->sendVertex(hr);
  rd->setTexCoord(0, Vector2(s,1));
  rd->sendVertex(hl);
  rd->setTexCoord(0, Vector2(0,1));
  rd->sendVertex(bl);
  rd->setTexCoord(0, Vector2(0,0));
  rd->sendVertex(br);
  rd->endPrimitive();


  /***
  rd->setColor(Color3::white());

  // Render the quad that is a preview for the line to be drawn.  It
  // adjusts its width based on pressure, etc..
  hl = _handFrame.translation - 0.5*width*right + 0.001*perpUp;
  hr = hl + width*right;
  bl = _brush->state->frameInRoomSpace.translation - 0.5*width*right + 0.001*perpUp;
  br = bl + width*right;

  // back up on the side of the hand to indicate the hands too close
  // threshold where drawing stops
  hl = hl + 0.05*(bl-hl).unit();
  hr = hr + 0.05*(br-hr).unit();
  

  rd->setTexture(0, NULL);
  rd->pushState();
  rd->setAlphaTest(RenderDevice::ALPHA_GREATER, 0.05);
  rd->setColor(Color3::white());
  rd->setShininess(125);           // 0 -> 255
  rd->setSpecularCoefficient(0.1); // 0 -> 1
  
  Texture::Ref brushTipTex = _gfxMgr->getTexture("BrushTipsTile"); 
  int numBrushTips = ConfigVal("NumBrushTips", brushTipTex->width()/brushTipTex->height(), false);  
  double s1 = (double)_brush->state->brushTip/(double)numBrushTips;
  double s2 = (double)(_brush->state->brushTip+1)/(double)numBrushTips;

  Texture::Ref patternTex = _gfxMgr->getTexture("PatternsTile"); 
  int numPatterns = ConfigVal("NumBrushTips", patternTex->width()/patternTex->height(), false);  
  double ps1 = (double)_brush->state->pattern/(double)numPatterns;
  double ps2 = (double)(_brush->state->pattern+1)/(double)numPatterns;

  double t1 = 0.0;
  double t2 = (hr - br).magnitude() / (hr - hl).magnitude();


  rd->setTexture(0, brushTipTex);  
  rd->setTextureCombineMode(0, RenderDevice::TEX_MODULATE);
  rd->setTexture(1, patternTex);  
  rd->setTextureCombineMode(0, RenderDevice::TEX_MODULATE);
  if (GLCaps::numTextureUnits() > 2) {
    rd->setTexture(2, _gfxMgr->getTexture("ColorSwatchTable"));
    rd->setTextureCombineMode(2, RenderDevice::TEX_MODULATE);   
  }

  rd->beginPrimitive(PrimitiveType::QUADS);
  rd->setNormal((hr-br).cross(bl-br).unit());
  if (GLCaps::numTextureUnits() > 2) {
    rd->setTexCoord(2, Vector2(_brush->state->colorSwatchCoord, _brush->state->colorValue));
  }
  rd->setTexCoord(0, Vector2(s1, t1));
  rd->setTexCoord(1, Vector2(ps1, t1));
  rd->sendVertex(br);
  rd->setTexCoord(0, Vector2(s1, t2));
  rd->setTexCoord(1, Vector2(ps1, t2));
  rd->sendVertex(hr);
  rd->setTexCoord(0, Vector2(s2, t2));
  rd->setTexCoord(1, Vector2(ps2, t2));
  rd->sendVertex(hl);
  rd->setTexCoord(0, Vector2(s2, t1));
  rd->setTexCoord(1, Vector2(ps2, t1));
  rd->sendVertex(bl);
  rd->endPrimitive();

  rd->popState();
  ***/

  rd->popState();
}

void
CavePaintingCursors::drawPressureMeter()
{
  if (MinVR::ConfigVal("HidePressureMeter", 0, false)) {
    return;
  }

  RenderDevice *rd = _gfxMgr->getRenderDevice();
  rd->pushState();
  rd->setCullFace(RenderDevice::CULL_NONE);

  double alpha = 1.0;
  if (_brush->state->maxPressure != 0.0) {
    alpha = clamp(_brush->state->pressure / _brush->state->maxPressure, 0.0, 1.0);
  }
  double w = _brush->state->width;
  if (alpha == 0.0) {
    w = 0.0;
  }
  
  //Draw::sphere(Sphere(_brush->state->frameInRoomSpace.translation, w), rd, col, Color4::clear());
  
  // Draw a visual indicator for pressure  
  Vector3 offset = MinVR::ConfigVal("PressureMeterOffset", Vector3(0.075, 0, 0), false);
  Vector3 side = Vector3(MinVR::ConfigVal("PressureMeterSize", 0.02, false), 0, 0);
  Vector3 up = Vector3(0, 1.25*MinVR::ConfigVal("PressureMeterSize", 0.025, false), 0);
  Vector3 a = _brush->state->frameInRoomSpace.translation + offset - side + up;
  Vector3 b = _brush->state->frameInRoomSpace.translation + offset + side + up;
  Vector3 c = _brush->state->frameInRoomSpace.translation + offset + side - up;
  Vector3 d = _brush->state->frameInRoomSpace.translation + offset - side - up;

  offset += Vector3(0,0,0.002);
  Vector3 a2 = _brush->state->frameInRoomSpace.translation + offset - side - up + 2.0*alpha*up;
  Vector3 b2 = _brush->state->frameInRoomSpace.translation + offset + side - up + 2.0*alpha*up;
  Vector3 c2 = _brush->state->frameInRoomSpace.translation + offset + side - up;
  Vector3 d2 = _brush->state->frameInRoomSpace.translation + offset - side - up;

  rd->disableLighting();
  rd->setColor(Color3(0.25, 0.25, 0.25));
  rd->beginPrimitive(PrimitiveType::LINE_STRIP);
  rd->sendVertex(a);
  rd->sendVertex(b);
  rd->sendVertex(c);
  rd->sendVertex(d);
  rd->sendVertex(a);
  rd->endPrimitive();
  
  rd->setAlphaTest(RenderDevice::ALPHA_GEQUAL, 0.5);
  rd->disableLighting();
  rd->setColor(Color3::white());
  rd->setTexture(0, _gfxMgr->getTexture("PressureMeterFG"));
  rd->beginPrimitive(PrimitiveType::QUADS);
  rd->setTexCoord(0, Vector2(0,0));
  rd->sendVertex(a2);
  rd->setTexCoord(0, Vector2(1,0));
  rd->sendVertex(b2);
  rd->setTexCoord(0, Vector2(1,1));
  rd->sendVertex(c2);
  rd->setTexCoord(0, Vector2(0,1));
  rd->sendVertex(d2);
  rd->endPrimitive();

  rd->popState();
}


void
CavePaintingCursors::drawLineAsCylinder(const Vector3 &startPoint,
                                        const Vector3 &endPoint,
                                        const double &startRadius,
                                        const double &endRadius,
                                        RenderDevice *renderDevice,
                                        const Color4 &color,
                                        const int &numSections,
                                        Texture::Ref tex,
                                        float texRepeat)  
{
  CoordinateFrame rot;
  Vector3 Y = (endPoint - startPoint).direction();
  Vector3 X = (fabs(Y.dot(Vector3::unitX())) > 0.9) ?
    Vector3::unitY() : Vector3::unitX();
  Vector3 Z = X.cross(Y).direction();
  X = Y.cross(Z);
  rot.rotation.setColumn(0, X);
  rot.rotation.setColumn(1, Y);
  rot.rotation.setColumn(2, Z);

  Array<Vector3> vfan1;
  Array<Vector3> vfan2;
  Array<Vector2> fanTexCoords;

  int smax = 1;
  if (getShadowsOn()) {
    smax = 2;
  }

  for (int s=0;s<smax;s++) {

    if (s==0) {
      renderDevice->setColor(color);
      renderDevice->setTexture(0, tex);
    }
    else {
      pushShadowState(renderDevice);
      renderDevice->setTexture(0, NULL);
    }
    
    renderDevice->beginPrimitive(PrimitiveType::QUAD_STRIP);
    for (int i=0;i<numSections;i++) {
      const double a = (double)i * twoPi() / (double)(numSections-1);
      Vector3 v = Vector3(cos(a), 0, sin(a));
      Vector3 vrot = (rot.pointToWorldSpace(v)).unit();
      renderDevice->setNormal(rot.normalToWorldSpace(v).unit());
      Vector3 v1 = vrot*startRadius + startPoint;
      renderDevice->setTexCoord(0, Vector2(0, sin(0.5*a)));
      renderDevice->sendVertex(v1);
      vfan1.append(v1);
      Vector3 v2 = vrot*endRadius + endPoint;
      renderDevice->setTexCoord(0, Vector2(texRepeat, sin(0.5*a)));
      renderDevice->sendVertex(v2);
      vfan2.append(v2);
      fanTexCoords.append(Vector2(0.5 + 0.5*v[0], 0.5 + 0.5*v[2]));
    }
    renderDevice->endPrimitive();
    
    renderDevice->beginPrimitive(PrimitiveType::TRIANGLE_FAN);
    renderDevice->setNormal((startPoint - endPoint).unit());
    for (int i=0;i<vfan1.size();i++) {
      renderDevice->setTexCoord(0, fanTexCoords[i]);
      renderDevice->sendVertex(vfan1[i]);
    }
    renderDevice->endPrimitive();
    
    renderDevice->beginPrimitive(PrimitiveType::TRIANGLE_FAN);
    renderDevice->setNormal((endPoint - startPoint).unit());
    for (int i=vfan2.size()-1;i>=0;i--) {
      renderDevice->setTexCoord(0, fanTexCoords[i]);
      renderDevice->sendVertex(vfan2[i]);
    }
    renderDevice->endPrimitive();

    if (s==1) {
      popShadowState(renderDevice);
    }
  }
}


} // end namespace
