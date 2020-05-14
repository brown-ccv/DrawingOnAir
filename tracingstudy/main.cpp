/**
 * \author Daniel Keefe (dfk)
 *
 * \file  main.cpp
 *
 */

#include <DrawOnAir.H>

#include "SteeringStudy.H"

int main(int argc, char **argv)
{
  SteeringStudy *app = new SteeringStudy(argc, argv);
  app->run();
  return 0;
}
