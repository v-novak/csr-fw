#!/usr/bin/env python3

import math

L = 437.5
W = 470
b = 100
Ws = W - 2 * b

def calc_beta(alpha):
    return math.atan(L / (L / math.tan(alpha) + Ws))

def rpm2(rpm1, alpha):
    Rturn = L / math.tan(alpha) - b
    return rpm1 * (1 + W / Rturn)

def main():
    alpha_min = 5
    alpha_max = 45
    alpha_step = 2

    print('alpha,beta,rpm1,rpm2')
    rpm1 = 1.0
    for alpha in range(alpha_min, alpha_max, alpha_step):
        print('{},{:.2f},{},{:.2f}'.format(alpha, math.degrees(calc_beta(math.radians(alpha))),
              rpm1, rpm2(rpm1, math.radians(alpha))))

if __name__ == '__main__':
    main()
