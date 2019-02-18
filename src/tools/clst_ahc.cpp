#include "alglib/dataanalysis.h"
#include "alglib/stdafx.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

using namespace alglib;

int main(int argc, char **argv) {
  clusterizerstate s;
  // ahcreport rep;
  kmeansreport rep;
  ae_int_t disttype;
  real_2d_array xy =
      "[[1,1,0,0,0,0],[0,0,0,0,1,1],[1,1,1,0,0,0],[0,0,0,1,1,0],[0,0,0,1,1,1]]";
  clusterizercreate(s);
  disttype = 2;
  clusterizersetpoints(s, xy, disttype);
  // clusterizerrunahc(s, rep);
  // printf("%s\n", rep.z.tostring().c_str()); // EXPECTED: [[1,2],[0,3]]
  clusterizerrunkmeans(s, 2, rep);
  printf("%s\n", rep.cidx.tostring().c_str());

  // real_2d_array ab;
  // ab.setlength(3, 6);
  // ab[0] = xy[0].tostring();
  // ab[1] = xy[2].tostring();
  // ab[2] = xy[3].tostring();
  return 0;
}