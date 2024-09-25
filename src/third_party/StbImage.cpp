#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_PIC
#define STBI_NO_PNM
#if defined(__GNUC__)
#   define STBIDEF __attribute__ ((visibility ("default"))) extern
#else
#   define STBIDEF extern
#endif
#include <stb_image.h>