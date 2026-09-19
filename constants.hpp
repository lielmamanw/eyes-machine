#pragma once

#include <cstdint>

struct RGBColor
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

constexpr RGBColor EyesMachineModeColors[] =
{
  {255, 0, 0}, // Red
  {0, 255, 0}, // Green
  {0, 0, 255} // Blue
};