#!/usr/bin/env python3

import math
from string import Template

OUTPUT_CODE = 'wheels/adjustment.cpp'

ALPHA_MIN = 5
ALPHA_MAX = 40
ALPHA_STEP = 1
L = 437.5
W = 470
b = 100
Ws = W - 2 * b


def calc_beta(alpha):
    return math.atan(L / (L / math.tan(alpha) + Ws))

def rpm2(rpm1, alpha):
    Rturn = L / math.tan(alpha) - b
    return rpm1 * (1 + W / Rturn)

def precalc(alpha_min=ALPHA_MIN, alpha_max=ALPHA_MAX, alpha_step=ALPHA_STEP):
    rpm1 = 1.0

    return [(int(math.degrees(calc_beta(math.radians(alpha)))), rpm2(rpm1, math.radians(alpha)))
            for alpha in range(alpha_min, alpha_max, alpha_step)]
        #print('{},{:.2f},{},{:.2f}'.format(alpha, )))

def main():
    code_template = Template("""
    #include "adjustment.h"

    void adjust(int angle, recalc_params& params)
    {
        const int min_angle = $alpha_min;
        const int max_angle = $alpha_max;
        const int angle_step = $alpha_step;

        static const recalc_params recalc_table[$rows] = {
            $values
        };

        if (angle < min_angle) {
          params.outer_angle = angle;
          params.outer_speed_multiplier = 1.0;
          return;
        }

        if (angle >= max_angle) {
          params.outer_angle = recalc_table[$rows - 1].outer_angle;
          params.outer_speed_multiplier = recalc_table[$rows - 1].outer_speed_multiplier;
          return;
        }

        int line = (angle - min_angle) / angle_step;
        const recalc_params& row = recalc_table[line];
        params.outer_angle = row.outer_angle;
        params.outer_speed_multiplier = row.outer_speed_multiplier;
    }
    """)

    values = precalc()
    values_str = ', \n            '.join(["{{ {}, {} }}".format(val[0], val[1]) for val in values])
    code = code_template.substitute(values=values_str,
                                    alpha_min=ALPHA_MIN,
                                    alpha_max=ALPHA_MAX - ALPHA_STEP,
                                    alpha_step=ALPHA_STEP,
                                    rows=len(values))
    with open(OUTPUT_CODE, 'w') as code_file:
        code_file.write(code)

if __name__ == '__main__':
    main()
