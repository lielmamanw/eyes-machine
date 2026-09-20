#pragma once

// Interval-until-next-fire is sampled from a normal distribution per timer,
// so different behaviors (blink, saccade, ...) can each have their own rhythm.
struct RandomTimerConfig
{
  float meanIntervalMs;
  float stddevIntervalMs;
  unsigned long minIntervalMs; // clamp so a sampled interval can't fire instantly (or negative)
};

class RandomTimer
{
  public:
    explicit RandomTimer(RandomTimerConfig config);

    void reset(unsigned long now);
    bool isDue(unsigned long now) const;
    void reschedule(unsigned long now);

    // isDue() + reschedule() combined, for behaviors that fire-and-forget in one step.
    bool tryFire(unsigned long now);

  private:
    RandomTimerConfig _config;
    unsigned long _dueAt;
};
