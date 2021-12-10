#ifndef ADJUSTMENT_H
#define ADJUSTMENT_H

struct recalc_params
{
  int outer_angle;
  float outer_speed_multiplier;
};

void adjust(int angle, recalc_params& params);

#endif //ADJUSTMENT_H
