#ifndef __VECTORS
#define __VECTORS

// ============ ESP IDF LIBRARIES ============

#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

// ============ DEFINITIONS ============

#define TOF_ZERO_X  0   // 0mm overall
#define TOF_ZERO_Y  113 // +64mm pre 45deg rotation
#define TOF_ZERO_Z  -23 // -96mm pre 45deg rotation

// ============ EXTERNAL TYPES ============

// vector type, with cartesian and polar coordinates
typedef struct _vector_t{
    float x;      // cartesian x    [mm]
    float y;      // cartesian y    [mm]
    float z;      // cartesian z    [mm]
    float mag;    // magnitude      [mm]
    float yaw;    // yaw            [rad]
    float pit;    // pitch          [rad]
} vector_t;

// ============ EXTERNAL FUNCTIONS ============



#endif /* __VECTORS */