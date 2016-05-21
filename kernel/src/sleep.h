#pragma once

#include "common.h"
#include "svc.h"

void sleep_us(int us);
/// Sleeps for [us] microseconds.

u64 elapsed_us();
/// Returns the number of ellapsed microseconds since powered up.

