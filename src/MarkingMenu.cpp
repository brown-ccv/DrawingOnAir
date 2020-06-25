
#include <ConfigVal.H>
#include "MarkingMenu.H"

#define NOTHING_HIGHLIGHTED -1

using namespace G3D;
namespace DrawOnAir {


MarkingMenu::MarkingMenu(GfxMgrRef              gfxMgr,
                         EventMgrRef            eventMgr,
                         HCIMgrRef              hciMgr,
                         CavePaintingCursorsRef cursors,
                         BrushRef          brush,
                         //ForceNetInterface*      forceNetInterface,
                         Array<std::string>     btnDownTriggers,
                         Array<std::string>     trackerTriggers,
                         Array<std::string>     btnUpTriggers,
                         Array<std::string>     images,
                         Array<std::string>     helpNames,
                         Array<std::string>     eventNames,
                         CoordinateFrame        initialFrame) : WidgetHCI(hciMgr)
{
  _gfxMgr = gfxMgr;
  _eventMgr = eventMgr;
  _cursors = cursors;
  _brush = brush;
  //_forceNetInterface = forceNetInterface;
  _images = images;
  _helpNames = helpNames;
  _eventNames = eventNames;
  _frame = initialFrame;
  _hidden = false;
  debugAssert(_images.size());
  alwaysAssertM(_images.size() == _helpNames.size(), "Marking Menu: Number of images and number of help names differ.");
  alwaysAssertM(_images.size() == _eventNames.size(), "Marking Menu: Number of images and number of event names differ.");

  _highlighted = NOTHING_HIGHLIGHTED;
  SPHERE_RAD = MinVR::ConfigVal("MarkingMenu_SphereRad",0.25,false);
  SPHERE_HIGH_RAD = MinVR::ConfigVal("MarkingMenu_SphereHighRad",0.29,false);
  SELECTION_RAD = MinVR::ConfigVal("MarkingMenu_SelectionRad",0.29,false);
  ITEM_SPACING = MinVR::ConfigVal("MarkingMenu_ItemSpacing",0.55,false);
  OFFSET_DIST = MinVR::ConfigVal("MarkingMenu_OffsetDist",0.05,false);
  
  _fsa = new Fsa("MarkingMenu");
  _fsa->addState("Start");
  _fsa->addState("Drag");
  _fsa->addArc("TrackerMove", "Start", "Start", trackerTriggers);
  _fsa->addArcCallback("TrackerMove", this, &MarkingMenu::trackerMove);
  _fsa->addArc("ClickOn", "Start", "Drag", btnDownTriggers);
  _fsa->addArcCallback("ClickOn", this, &MarkingMenu::clickOn);
  _fsa->addArc("TrackerDrag", "Drag", "Drag", trackerTriggers);
  _fsa->addArcCallback("TrackerDrag", this, &MarkingMenu::trackerDrag);
  _fsa->addArc("ClickOff", "Drag", "Start", btnUpTriggers);  
  _fsa->addArcCallback("ClickOff", this, &MarkingMenu::clickOff);

  // set locations of menu items - currently hardcoded for up to 4
  // menu items, but you could have more if you did something smarter
  // involving distributing them evenly around a circle here..  note,
  // the first location is that of the main icon for the menu -
  // different from the others which are specific menu items.
  _norm  = _frame.vectorToObjectSpace(Vector3(0,0,1)).unit();
  _right = _frame.vectorToObjectSpace(Vector3(1,0,0)).unit();
  Vector3 up   = _norm.cross(_right).unit();
  Vector3 down = -up;


  _itemPts.append(Vector3(0,0,0));
 
  // Order of images and placement around a 8 point compass
  // East, West, North, South, NE, NW, SE, SW
  if (_images.size() > 1) {
    double a = toRadians(0.0);
    _itemPts.append(_itemPts[0] + ITEM_SPACING*cos(a)*_right + ITEM_SPACING*sin(a)*up + OFFSET_DIST*_norm);
  }
  if (_images.size() > 2) {
    double a = toRadians(180.0);
    _itemPts.append(_itemPts[0] + ITEM_SPACING*cos(a)*_right + ITEM_SPACING*sin(a)*up + OFFSET_DIST*_norm);
  }
  if (_images.size() > 3) {
    double a = toRadians(90.0);
    _itemPts.append(_itemPts[0] + ITEM_SPACING*cos(a)*_right + ITEM_SPACING*sin(a)*up + OFFSET_DIST*_norm);
  }
  if (_images.size() > 4) {
    double a = toRadians(-90.0);
    _itemPts.append(_itemPts[0] + ITEM_SPACING*cos(a)*_right + ITEM_SPACING*sin(a)*up + OFFSET_DIST*_norm);
  }
  if (_images.size() > 5) {
    double a = toRadians(45.0);
    _itemPts.append(_itemPts[0] + ITEM_SPACING*cos(a)*_right + ITEM_SPACING*sin(a)*up + OFFSET_DIST*_norm);
  }
  if (_images.size() > 6) {
    double a = toRadians(135.0);
    _itemPts.append(_itemPts[0] + ITEM_SPACING*cos(a)*_right + ITEM_SPACING*sin(a)*up + OFFSET_DIST*_norm);
  }
  if (_images.size() > 7) {
    double a = toRadians(-45.0);
    _itemPts.append(_itemPts[0] + ITEM_SPACING*cos(a)*_right + ITEM_SPACING*sin(a)*up + OFFSET_DIST*_norm);
  }
  if (_images.size() > 8) {
    double a = toRadians(-135.0);
    _itemPts.append(_itemPts[0] + ITEM_SPACING*cos(a)*_right + ITEM_SPACING*sin(a)*up + OFFSET_DIST*_norm);
  }
  
  debugAssert(_images.size());
  debugAssert(_images.size() <= 9);
  debugAssert(_itemPts.size() == _images.size());

  _font = _gfxMgr->getDefaultFont();
  show();
}

MarkingMenu::~MarkingMenu()
{
  _gfxMgr->removeDrawCallback(_cbid);
}

void
MarkingMenu::hide()
{
  _hidden = true;
  _gfxMgr->removeDrawCallback(_cbid);
}

void
MarkingMenu::show()
{
  _hidden = false;
  _cbid = _gfxMgr->addDrawCallback(this, &MarkingMenu::draw); 
}

void
MarkingMenu::activate()
{
  _eventMgr->addFsaRef(_fsa);
  _cursors->setBrushCursor(CavePaintingCursors::POINTER_CURSOR);
  /*if (_forceNetInterface != NULL) {
    _forceNetInterface->startPlaneEffect();
  }*/
  _highlighted = 0;
}

void
MarkingMenu::deactivate()
{
  _highlighted = NOTHING_HIGHLIGHTED;
  releaseFocusWithHCIMgr();
  _eventMgr->removeFsaRef(_fsa);
  /*if (_forceNetInterface != NULL) {
    _forceNetInterface->stopPlaneEffect();
  }*/
}

bool
MarkingMenu::pointerOverWidget(Vector3 pointerPosRoomSpace)
{
  if (_hidden) {
    return false;
  }

  if (G3D::fuzzyEq(_frame.rotation.determinant(), 1.0f)) {
    Vector3 pos = _frame.pointToObjectSpace(pointerPosRoomSpace);
    if (Sphere(_itemPts[0], SELECTION_RAD).contains(pos)) {
      return true;
    }
    else {
      return false;
    }
  }
}

void
MarkingMenu::trackerMove(MinVR::EventRef e)
{
  // assume the tracker is already inside the menu.  if we move outside of the menu, then deactivate the widget.
  if (pointerOverWidget(e->getCoordinateFrameData().translation)) {
    _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  }
  else {
    deactivate();    
  }
}

void
MarkingMenu::clickOn(MinVR::EventRef e)
{
}

void
MarkingMenu::trackerDrag(MinVR::EventRef e)
{
  _brush->state->frameInRoomSpace = e->getCoordinateFrameData();
  /** Alogrithm for determining highlighting..
      if in menu icon, then it is highlighted,
      if outside menu icon, then closest point is highlighted
  **/
  Vector3 pos = _frame.pointToObjectSpace(e->getCoordinateFrameData().translation);
  _trackerPos = pos;

  _highlighted = NOTHING_HIGHLIGHTED;  
  if (Sphere(_itemPts[0], SPHERE_HIGH_RAD).contains(pos)) {
    _highlighted = 0;
  }
  else if (_itemPts.size() > 1) {
    _highlighted = 1;
    double dist = (pos - _itemPts[1]).magnitude();
    for (int i=2;i<_itemPts.size();i++) {
      double newdist = (pos - _itemPts[i]).magnitude();
      if (newdist < dist) {
        _highlighted = i;
        dist = newdist;
      }
    }
  }
}

void
MarkingMenu::clickOff(MinVR::EventRef e)
{
  int selection = _highlighted;
  deactivate();
  if (selection > 0) {
    _eventMgr->processEvent(new MinVR::VRG3DEvent(_eventNames[selection]));
  }
}


void
MarkingMenu::draw(RenderDevice *rd, const CoordinateFrame &virtualToRoomSpace)
{
  rd->pushState();
  rd->setObjectToWorldMatrix(_frame);
  rd->disableLighting();

  Vector3 drawOffset(0,0,0);
  Color3 highCol = Color3(0.93,0.6,0.5);
  Color3 bgCol = Color3(0.1, 0.1, 0.44);

  Vector3 offset = 0.001*_norm;

  // Draw main menu icon
  if (_highlighted == 0) {
    drawText(rd, _helpNames[0], _itemPts[0], _norm, _right, 1.0);
    //Sphere s(_itemPts[0] + drawOffset, SPHERE_HIGH_RAD);
    //drawTexturedSphereSection(s, rd, highCol, true, false, _images[0]);
    drawColoredCircle(rd, bgCol, SPHERE_HIGH_RAD, _itemPts[0]-2.0*offset, _norm, _right);
    //rd->setPolygonOffset(1.0);
    drawTexturedCircle(rd, _images[0], 0.9*SPHERE_HIGH_RAD, _itemPts[0]-offset, _norm, _right, 1.0);
    //rd->setPolygonOffset(0.0);
  }
  else {
    //Sphere s(_itemPts[0] + drawOffset, SPHERE_RAD);
    //drawTexturedSphereSection(s, rd, Color3::white(), true, false, _images[0]);
    drawColoredCircle(rd, bgCol, SPHERE_RAD, _itemPts[0]-2.0*offset, _norm, _right);
    //rd->setPolygonOffset(1.0);
    drawTexturedCircle(rd, _images[0], 0.9*SPHERE_RAD, _itemPts[0]-offset, _norm, _right, 1.0);
    //rd->setPolygonOffset(0.0);
  }
  
  if (_fsa->getCurrentState() == "Drag") {
    // Draw menu items
    for (int i=1;i<_images.size();i++) {
      if (_highlighted == i) {
        drawText(rd, _helpNames[i], _itemPts[i], _norm, _right, 1.0);
        //Sphere s(_frame.vectorToWorldSpace(_itemPts[i]+drawOffset), SPHERE_HIGH_RAD);
        //drawTexturedSphereSection(s, rd, highCol, true, false, _images[i]);
        drawColoredCircle(rd, bgCol, SPHERE_HIGH_RAD, _itemPts[i], _norm, _right);
        //rd->setPolygonOffset(1.0);
        drawTexturedCircle(rd, _images[i], 0.9*SPHERE_HIGH_RAD, _itemPts[i]+offset, _norm, _right, 1.0);
        //rd->setPolygonOffset(0.0);
      }
      else {
        //Sphere s(_frame.vectorToWorldSpace(_itemPts[i]+drawOffset), SPHERE_RAD);
        //drawTexturedSphereSection(s, rd, Color3::white(), true, false, _images[i]);
        drawColoredCircle(rd, bgCol, SPHERE_RAD, _itemPts[i], _norm, _right);
        //rd->setPolygonOffset(1.0);
        drawTexturedCircle(rd, _images[i], 0.9*SPHERE_RAD, _itemPts[i]+offset, _norm, _right, 1.0);
        //rd->setPolygonOffset(0.0);
      }
    }
    
    // Draw feedback line
    rd->setColor(bgCol);
    rd->setTexture(0, NULL);
    Draw::lineSegment(LineSegment::fromTwoPoints(_norm*OFFSET_DIST, _trackerPos+_norm*OFFSET_DIST), rd);  
  }

  rd->popState();
}


void
MarkingMenu::drawColoredCircle(RenderDevice *rd, const Color4 &color, double radius, 
                                Vector3 centerPt, Vector3 normal, Vector3 right)
{
  rd->setCullFace(RenderDevice::CULL_NONE);
  rd->setColor(color);
  rd->disableLighting();
  rd->beginPrimitive(PrimitiveType::TRIANGLE_FAN);
  rd->sendVertex(centerPt);
  const int nsections = 40;
  double a = 0.0;
  double ainc = -twoPi() / (double)(nsections-1);
  for (int i=0;i<nsections;i++) {
    rd->sendVertex(centerPt + Vector3(radius*cos(a), radius*sin(a), 0.0));
    a -= ainc;
  }
  rd->endPrimitive();
}

void
MarkingMenu::drawTexturedCircle(RenderDevice *rd, const std::string &texname, double radius, 
                                Vector3 centerPt, Vector3 normal, Vector3 right, float alpha)
{
  rd->setCullFace(RenderDevice::CULL_NONE);
  rd->setColor(Color3::white());
  rd->disableLighting();
  rd->setTexture(0, _gfxMgr->getTexture(texname));
  rd->beginPrimitive(PrimitiveType::TRIANGLE_FAN);
  rd->setTexCoord(0, Vector2(0.5, 0.5));
  rd->sendVertex(centerPt);
  const int nsections = 40;
  double a = 0.0;
  double ainc = -twoPi() / (double)(nsections-1);
  for (int i=0;i<nsections;i++) {
    rd->setTexCoord(0, Vector2(0.5 + 0.5*cos(a),1.0-(0.5 + 0.5*sin(a))));
    rd->sendVertex(centerPt + Vector3(radius*cos(a), radius*sin(a), 0.0));
    a -= ainc;
  }
  rd->endPrimitive();
}


void
MarkingMenu::drawTexturedSphereSection(
    const Sphere&       sphere,
    RenderDevice*       renderDevice,
    const Color4&       color,
    bool                top,
    bool                bottom,
    const std::string&  texname)
{
  
  // The first time this is invoked we create a series of quad
  // strips in a vertex array.  Future calls then render from the
  // array.
  
  CoordinateFrame cframe = renderDevice->objectToWorldMatrix();
  
  // Auto-normalization will take care of normal length
  cframe.rotation = cframe.rotation * sphere.radius;
  cframe.translation += sphere.center;
  
  glPushAttrib(GL_ENABLE_BIT);
  //glDisable(GL_NORMALIZE);
  //glEnable(GL_RESCALE_NORMAL);
  
  renderDevice->pushState();
  renderDevice->setObjectToWorldMatrix(cframe);
  
  renderDevice->setColor(color);
  renderDevice->setShadeMode(RenderDevice::SHADE_SMOOTH);
  renderDevice->setTextureCombineMode(0,RenderDevice::TEX_MODULATE);
  renderDevice->disableLighting();
  
  static bool initialized = false;
  static int numIndices = 0;
  static VAR vbuffer;
  static VAR tbuffer;
  static Array< Array<G3D::uint16> > stripIndexArray;

  const int SPHERE_PITCH_SECTIONS = 20;
  const int SPHERE_YAW_SECTIONS = 20;
  
  if (! initialized) {
    // The normals are the same as the vertices for a sphere
    Array<Vector3> vertex;
    Array<Vector2> texcoord;
    
    int s = 0;
    int i = 0;
  
    for (int p = 0; p < SPHERE_PITCH_SECTIONS; ++p) {
      const double pitch0 = (p * 0.5*twoPi() / (double)SPHERE_PITCH_SECTIONS) / 2.0;
      const double pitch1 = ((p + 1) * 0.5*twoPi() / (double)SPHERE_PITCH_SECTIONS) / 2.0;
      
      stripIndexArray.next();
      
      for (int y = 0; y <= SPHERE_YAW_SECTIONS; ++y) {
        const double yaw = -y * twoPi() / SPHERE_YAW_SECTIONS;
        
        float cy = cos(yaw);
        float sy = sin(yaw);
        float sp0 = sin(pitch0);
        float sp1 = sin(pitch1);
        
        Vector3 v0 = Vector3(cy * sp0, cos(pitch0), sy * sp0);
        Vector3 v1 = Vector3(cy * sp1, cos(pitch1), sy * sp1);

        vertex.append(v0, v1);
        texcoord.append(Vector2((v0[0]+1.0)/2.0, (v0[2]+1.0)/2.0));
        texcoord.append(Vector2((v1[0]+1.0)/2.0, (v1[2]+1.0)/2.0));
        stripIndexArray.last().append(i, i + 1);
        i += 2;
      }
    }
    
    VARArea::Ref area = VARArea::create(vertex.size() * sizeof(Vector3) +
                                      texcoord.size() * sizeof(Vector2),
                                      VARArea::WRITE_ONCE);
    vbuffer = VAR(vertex, area);
    tbuffer = VAR(texcoord, area);
    numIndices = vertex.size();
    initialized = true;
  }
  
  
  int start = top ? 0 : (SPHERE_PITCH_SECTIONS / 2);
  int stop = bottom ? SPHERE_PITCH_SECTIONS : (SPHERE_PITCH_SECTIONS / 2);
  
  renderDevice->beginIndexedPrimitives();
  renderDevice->setNormalArray(vbuffer);
  renderDevice->setVertexArray(vbuffer);
  renderDevice->setTexCoordArray(0, tbuffer);
  renderDevice->setTexture(0, _gfxMgr->getTexture(texname));
  for (int s = start; s < stop; ++s) {
    renderDevice->sendIndices(PrimitiveType::QUAD_STRIP, stripIndexArray[s]);
  }
  renderDevice->endIndexedPrimitives();
  
  renderDevice->popState();
  
  glPopAttrib();
}



void
MarkingMenu::drawText(RenderDevice *rd, std::string text, Vector3 centerPt,
                      Vector3 normal, Vector3 right, float alpha)
{
  Vector3 up = normal.cross(right);
  Vector3 offset = SPHERE_RAD*0.5*right + SPHERE_RAD*up + 1.1*OFFSET_DIST*normal;
  Matrix3 rot(right[0], up[0], normal[0],
              right[1], up[1], normal[1],
              right[2], up[2], normal[2]);
  CoordinateFrame frame = _frame * CoordinateFrame(rot, centerPt + offset);
  rd->disableLighting();

  float textheight = SPHERE_RAD/2.5;
  Vector2 bounds = _font->bounds(text,textheight); 
  double border = 0.15*bounds[1];

  rd->pushState();
  rd->setObjectToWorldMatrix(frame);//rd->getObjectToWorldMatrix()*frame);
  rd->setColor(Color4(0.93,0.9,0.8,1.0));//alpha));
  rd->beginPrimitive(PrimitiveType::QUADS);
  rd->sendVertex(Vector3(-border,-border,0));
  rd->sendVertex(Vector3(bounds[0]+border,-border,0));
  rd->sendVertex(Vector3(bounds[0]+border,bounds[1]+border,0));
  rd->sendVertex(Vector3(-border,bounds[1]+border,0));
  rd->endPrimitive();

  rd->setColor(Color4(1,1,1,1));//alpha));
  rd->setLineWidth(1.0);
  rd->beginPrimitive(PrimitiveType::LINE_STRIP);
  rd->sendVertex(Vector3(-border,-border,0));
  rd->sendVertex(Vector3(bounds[0]+border,-border,0));
  rd->sendVertex(Vector3(bounds[0]+border,bounds[1]+border,0));
  rd->sendVertex(Vector3(-border,bounds[1]+border,0));
  rd->sendVertex(Vector3(-border,-border,0));
  rd->endPrimitive();

  rd->popState();

  // move text off the background
  frame.translation += 0.001*normal;
  _font->draw3D(rd, text, frame, textheight, Color4(0.8,0.22,0,alpha),
                Color4::clear(), GFont::XALIGN_LEFT,
                GFont::YALIGN_BOTTOM);
  rd->enableLighting();
}


} // end namespace
