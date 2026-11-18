#include "vectors.h"

// ========== GLOBAL VARIABLES ==========

vector_t tof_origin;

// ============ INTERNAL FUNCTIONS ============

void update_pol( vector_t *vector ){
    vector->mag = sqrt( ( vector->x * vector->x ) + ( vector->y * vector->y ) + ( vector->z * vector->z ) );
    vector->yaw = atan2( vector->x, vector->z );
    vector->pit = atan2( vector->y, sqrt( ( vector->x * vector->x ) + ( vector->z * vector->z ) ) );
}

void update_car( vector_t *vector ){
    vector->x = vector->mag * cos( vector->pit ) * sin( vector->yaw );
    vector->y = vector->mag * sin( vector->pit );
    vector->z = vector->mag * cos( vector->pit ) * cos( vector->yaw );
}

float vector_dot_product( vector_t vector_a, vector_t vector_b ){
    return ( vector_a.x * vector_b.x ) + ( vector_a.y * vector_b.y ) + ( vector_a.z * vector_b.z );
}

vector_t vector_cross_product( vector_t vector_a, vector_t vector_b ){
    vector_t vector = {
        .x = ( vector_a.y * vector_b.z) - ( vector_a.z * vector_b.y ),
        .y = ( vector_a.z * vector_b.x) - ( vector_a.x * vector_b.z ),
        .z = ( vector_a.x * vector_b.y) - ( vector_a.y * vector_b.x ),
    };

    update_pol( &vector );

    return vector;
}

float vector_magn( vector_t vector ){
    return sqrt( ( vector.x * vector.x ) + ( vector.y * vector.y ) + ( vector.z * vector.z ) );
}

vector_t vector_mult_esc( float escalar, vector_t vector ){
    vector.x *= escalar;
    vector.y *= escalar;
    vector.z *= escalar;

    update_pol( &vector );

    return vector;
}

vector_t vector_projection( vector_t vector_a, vector_t vector_b ){
    return vector_mult_esc( vector_dot_product( vector_a, vector_b ) / ( vector_magn(vector_b) * vector_magn(vector_b) ), vector_b );
}

// ============ EXTERNAL FUNCTIONS ============

// new vector from cartesian coordinates
vector_t vector_new_car( float x, float y, float z ){
    vector_t vector = {
        .x = x,
        .y = y,
        .z = z,
    };

    update_pol( &vector );

    return vector;
}

// new vector from polar coordinates
vector_t vector_new_pol( float magnitude, float yaw, float pitch ){
    vector_t vector = {
        .mag = magnitude,
        .yaw = yaw,
        .pit = pitch,
    };

    update_pol( &vector );

    return vector;
}

// adds two vectors and return the result
vector_t vector_add( vector_t vector_a, vector_t vector_b ){
    vector_t vector = {
        .x = vector_a.x + vector_b.x,
        .y = vector_a.y + vector_b.y,
        .z = vector_a.z + vector_b.z,
    };

    update_pol( &vector );

    return vector;
}

// subtracts two vectors and return the result
vector_t vector_sub( vector_t vector_a, vector_t vector_b ){
    vector_t vector = {
        .x = vector_a.x + vector_b.x,
        .y = vector_a.y + vector_b.y,
        .z = vector_a.z + vector_b.z,
    };

    update_pol( &vector );

    return vector;
}

// returns the angle between two vectors
float vector_ang( vector_t vector_a, vector_t vector_b ){
    // angle = arctan ( cross_product_magnitude( a x b ), dot_product( a * b ) )
    return atan2( vector_magn( vector_cross_product( vector_a, vector_b ) ), vector_dot_product( vector_a, vector_b ) );
}

// calculate the direction vector between a point in 3D space and a vector, using the point's projection onto the vector
vector_t vector_point_to_vector( vector_t vector_a, vector_t vector_b ){
    return vector_sub(vector_a, vector_projection( vector_a, vector_b ) );
}

// converts int16_t centesimal fixed point degrees to float radians
float vector_deg_to_rad( int16_t degrees ){
    // optimized ( ( (float)( degrees / 100 ) / 180 ) * M_PI ) for resolution
    return (float)( (float)( degrees * M_PI ) / ( 180 * 100 ) );
}

// converts from float radians back to int16_t centesimal fixed point degrees
int16_t vector_rad_to_deg( float radians ){
    return (int16_t)( ( radians * 180 * 100 ) / M_PI );
}

// calculates the gimbal's origin from cartesian coordinates
void vector_init(){
    tof_origin = vector_new_car( TOF_ZERO_X, TOF_ZERO_Y, TOF_ZERO_Z);
}
