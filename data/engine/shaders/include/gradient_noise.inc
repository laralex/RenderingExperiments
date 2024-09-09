/* Gradient noise from Jorge Jimenez's presentation: */
/* http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare */
/* See also https://blog.frost.kiwi/GLSL-noise-and-radial-gradient/ */
float GradientNoise(in vec2 uv) {
	return fract(52.9829189 * fract(dot(uv, vec2(0.06711056, 0.00583715))));
}

#include "common/screen_space_dither"
