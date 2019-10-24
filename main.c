#include "hashTool.h"
#include "lookup.h"

int main(){
  init();
  flushCache();
  
  benchmark(findNormal, "Normal find");
  benchmark(findGprefetch, "Gprefetch find");
  benchmark(findGopt, "G-opt find");

  return 0;
}




