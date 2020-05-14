/**
 * \author Daniel Keefe (dfk)
 *
 * \file  main.cpp
 *
 */

#include <DrawOnAir.H>

int main(int argc, char **argv)
{
  CavePainting *cavePainting = new CavePainting(argc, argv);
  cavePainting->run();
  return 0;
}
