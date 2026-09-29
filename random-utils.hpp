#pragma once

// Uniform in (0, 1] - e.g. for probability rolls like "10% chance of X".
float sampleUniform01();

float sampleGaussian(float mean, float stddev);
float sampleClampedGaussian(float mean, float stddev, float minVal, float maxVal);
