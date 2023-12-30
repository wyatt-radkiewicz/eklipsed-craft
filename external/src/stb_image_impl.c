#ifdef __APPLE__
  #include <TargetConditionals.h>
  #if TARGET_OS_MAC == 1 && TARGET_OS_IPHONE == 0
    #define STBI_NEON
  #endif
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
