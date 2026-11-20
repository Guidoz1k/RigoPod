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
    /* everything runs in cartesian
    float mag;    // magnitude      [mm]
    float yaw;    // yaw            [rad]
    float pit;    // pitch          [rad]
    */
} vector_t;

// ============ EXTERNAL FUNCTIONS ============

// new vector from polar coordinates
vector_t vector_new_pol( float magnitude, float yaw, float pitch );

// adds two vectors and return the result
vector_t vector_add( vector_t vector_a, vector_t vector_b );

// subtracts two vectors and return the result
vector_t vector_sub( vector_t vector_a, vector_t vector_b );

// calculates vector's magnitude
float vector_magni( vector_t vector );

// calculates vector's pitch
float vector_pitch( vector_t vector );

// calculates vector's yaw
float vector_yawww( vector_t vector );

// calculates a point's (or vector's) projection onto a vector
vector_t vector_projection( vector_t vector_a, vector_t vector_b );

// calculates the gimbal's origin from cartesian coordinates
vector_t vector_gimbal_origin();

#endif /* __VECTORS */