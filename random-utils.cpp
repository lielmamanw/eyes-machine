#include "random-utils.hpp"

#include <Arduino.h>
#include <math.h>

namespace
{
  float sampleUniform01()
  {
    return static_cast<float>(random(1, 1000001)) / 1000000.0f;
  }
}

float sampleGaussian(float mean, float stddev)
{
  // Box-Muller transform - classic trick for turning uniform randomness into Gaussian randomness.
  const float u1 = sampleUniform01();
  const float u2 = sampleUniform01();
  const float z0 = sqrtf(-2.0f * logf(u1)) * cosf(2.0f * PI * u2);

  return mean + z0 * stddev;
}

float sampleClampedGaussian(float mean, float stddev, float minVal, float maxVal)
{
  const float value = sampleGaussian(mean, stddev);
  if (value < minVal) return minVal;
  if (value > maxVal) return maxVal;
  return value;
}
