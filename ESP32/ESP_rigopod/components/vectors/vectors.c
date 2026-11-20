#include "vectors.h"

// ============ INTERNAL FUNCTIONS ============

// new vector from cartesian coordinates
vector_t vector_new_car( float x, float y, float z ){
    vector_t vector = {
        .x = x,
        .y = y,
        .z = z,
    };

    //update_pol( &vector );

    return vector;
}

/* everything runs in cartesian
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
*/

// calculates scalar product of two vectors
float vector_dot_product( vector_t vector_a, vector_t vector_b ){
    return ( vector_a.x * vector_b.x ) + ( vector_a.y * vector_b.y ) + ( vector_a.z * vector_b.z );
}

// calculates vector product of two vectors
vector_t vector_cross_product( vector_t vector_a, vector_t vector_b ){
    vector_t vector = {
        .x = ( vector_a.y * vector_b.z) - ( vector_a.z * vector_b.y ),
        .y = ( vector_a.z * vector_b.x) - ( vector_a.x * vector_b.z ),
        .z = ( vector_a.x * vector_b.y) - ( vector_a.y * vector_b.x ),
    };

    //update_pol( &vector );

    return vector;
}

// multiplies vector by scalar
vector_t vector_mult_esc( float escalar, vector_t vector ){
    vector.x *= escalar;
    vector.y *= escalar;
    vector.z *= escalar;

    //update_pol( &vector );

    return vector;
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

// ============ EXTERNAL FUNCTIONS ============

// new vector from polar coordinates
vector_t vector_new_pol( float magnitude, float yaw, float pitch ){
    float new_pitch = vector_deg_to_rad ( pitch );
    float new_yaw = vector_deg_to_rad ( yaw );
    vector_t vector = {
        .x = magnitude * cos( new_pitch ) * sin( new_yaw ),
        .y = magnitude * sin( new_pitch ),
        .z = magnitude * cos( new_pitch ) * cos( new_yaw ),
    };

    //update_car();

    return vector;
}

// adds two vectors and return the result
vector_t vector_add( vector_t vector_a, vector_t vector_b ){
    vector_t vector = {
        .x = vector_a.x + vector_b.x,
        .y = vector_a.y + vector_b.y,
        .z = vector_a.z + vector_b.z,
    };

    //update_pol( &vector );

    return vector;
}

// subtracts vector a from vector b and returns the result
vector_t vector_sub( vector_t vector_a, vector_t vector_b ){
    vector_t vector = {
        .x = vector_a.x - vector_b.x,
        .y = vector_a.y - vector_b.y,
        .z = vector_a.z - vector_b.z,
    };

    //update_pol( &vector );

    return vector;
}

// calculates vector's magnitude
float vector_magni( vector_t vector ){
    return sqrt( ( vector.x * vector.x ) + ( vector.y * vector.y ) + ( vector.z * vector.z ) );
}

// calculates vector's pitch
float vector_pitch( vector_t vector ){
    return atan2( vector.y, sqrt( ( vector.x * vector.x ) + ( vector.z * vector.z ) ) );
}

// calculates vector's yaw
float vector_yawww( vector_t vector ){
    return atan2( vector.x, vector.z );
}

/* turns out this was not necessary
// returns the angle between two vectors
float vector_ang( vector_t vector_a, vector_t vector_b ){
    // angle = arctan ( cross_product_magnitude( a x b ), dot_product( a * b ) )
    return atan2( vector_magni( vector_cross_product( vector_a, vector_b ) ), vector_dot_product( vector_a, vector_b ) );
}
*/

// calculates a projection of vector a onto vector b
vector_t vector_projection( vector_t vector_a, vector_t vector_b ){
    return vector_mult_esc( vector_dot_product( vector_a, vector_b ) / ( vector_magni(vector_b) * vector_magni(vector_b) ), vector_b );
}

// calculates the gimbal's origin from cartesian coordinates
vector_t vector_gimbal_origin(){
    return ( vector_t )vector_new_car( TOF_ZERO_X, TOF_ZERO_Y, TOF_ZERO_Z);
}
