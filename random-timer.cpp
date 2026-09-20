#include "random-timer.hpp"
#include "random-utils.hpp"

RandomTimer::RandomTimer(RandomTimerConfig config) : _config(config), _dueAt(0)
{}

void RandomTimer::reset(unsigned long now)
{
  reschedule(now);
}

bool RandomTimer::isDue(unsigned long now) const
{
  return now >= _dueAt;
}

void RandomTimer::reschedule(unsigned long now)
{
  const float sampledMs = sampleGaussian(_config.meanIntervalMs, _config.stddevIntervalMs);
  const unsigned long intervalMs = sampledMs > static_cast<float>(_config.minIntervalMs)
    ? static_cast<unsigned long>(sampledMs)
    : _config.minIntervalMs;

  _dueAt = now + intervalMs;
}

bool RandomTimer::tryFire(unsigned long now)
{
  if (!isDue(now)) return false;
  reschedule(now);
  return true;
}
