#ifndef SIM_THREAD_H
#define SIM_THREAD_H

#include "sim/sim_pixel.h"
#include <stdint.h>

void simulate_checkerboard(pixel* read, pixel* write, uint16_t sim_w, uint16_t sim_h, int checker_phase);

#endif