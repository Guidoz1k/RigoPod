#include "demos.h"

void led_demo(void){
    const uint8_t maximum = 100;
    uint8_t red = 0, green = 2 * maximum / 3, blue = 2 * maximum / 3;
    bool red_status = true, green_status = true, blue_status = false;

    while(1){ // this code tests the buil-in led
        delay_milli(10);

        if(red_status == true)
            if(red < maximum)
                red++;
            else
                red_status = false;
        else
            if(red > 0)
                red--;
            else
                red_status = true;

        if(green_status == true)
            if(green < maximum)
                green++;
            else
                green_status = false;
        else
            if(green > 0)
                green--;
            else
                green_status = true;

        if(blue_status == true)
            if(blue < maximum)
                blue++;
            else
                blue_status = false;
        else
            if(blue > 0)
                blue--;
            else
                blue_status = true;

        printf(" red: %d green %d blue: %d \n", red, green, blue);
        led_color(red, green, blue);
    }
}

/* Processes the manual control of the servo for tests and calibration
    All numerical entries will be comprised of '+' or '-' followed by a 4 digits number, ranging from 0 to 9000
    first char: command letter
        S = set servo position, followed by numerical entry
        X = calibrate the maximum PWM value of chosen servo, followed by numerical entry
        N = calibrate the minimum PWM value of chosen servo, followed by numerical entry
        R = read any calibration parameter of chosen servo, followed by:
            N = minimum PWM value
            X = maximum PWM value
            A = angle set
    second char: servo
        P = pitch servo
        R = roll servo
    following chars: command dependent
*/
void servo_demo(void){
    char buffer[7] = {0};
    servo_t servo = SERVO_PITCH;
    int16_t value = 0;

    while(1){ // this codes reads serial and controls servos
        /* scanf("%s", buffer) is harder to use
            it continuously reads the buffer, returning -1, only returning 1 when it reads anything
            if it reads a string smaller than desired size, it still returns 1, letting waste to remain in buffer
        */
        while(scanf("%c%c%c%c%c%c%c", &buffer[0], &buffer[1], &buffer[2], &buffer[3], &buffer[4], &buffer[5], &buffer[6]) != 7)
            delay_milli(100);

        printf("\n COMMAND: ");
        if( (buffer[1] == 'P') || (buffer[1] == 'R') ){
            if(buffer[1] == 'P')
                servo = SERVO_PITCH;
            else
                servo = SERVO_ROLL;
            switch(buffer[0]){
            case 'S':
                value = servo_string_to_number(&buffer[2]);
                if(value != 0x7FFF){
                    printf("SET %c to %d", buffer[1], value);
                    servo_pos(value, servo);
                }
                else
                    printf("INVALID POSITION");
                break;
            case 'X':
                value = servo_string_to_number(&buffer[2]);
                if(value != 0x7FFF){
                    printf("CALIBRATED MAX of %c to %d", buffer[1], value);
                    servo_cal(0, value, servo);
                }
                else
                    printf(" INVALID CALIBRATION");                   
                break;
            case 'N':
                value = servo_string_to_number(&buffer[2]);
                if(value != 0x7FFF){
                    printf("CALIBRATED MIN of %c to %d", buffer[1], value);
                    servo_cal(value, 0, servo);
                }
                else
                    printf(" INVALID CALIBRATION");
                break;
            case 'R':
                switch(buffer[2]){
                case 'X':
                    printf("RANGE MAX of %c of ", buffer[1]);
                    if(buffer[1] == 'P')
                        printf("%d", servo_return_cal(PITCH_MAX));
                    else
                        printf("%d", servo_return_cal(ROLL_MAX));
                    break;
                case 'N':
                    printf("RANGE MIN of %c of ", buffer[1]);
                    if(buffer[1] == 'P')
                        printf("%d", servo_return_cal(PITCH_MIN));
                    else
                        printf("%d", servo_return_cal(ROLL_MIN));
                    break;
                case 'A':
                    printf("RANGE ANGLE of %c of ", buffer[1]);
                    if(buffer[1] == 'P')
                        printf("%d", servo_return_cal(PITCH_ANG));
                    else
                        printf("%d", servo_return_cal(ROLL_ANG));
                    break;
                default:
                    printf("INVALID REGISTER");
                    break;
                }
                break;
            default:
                printf("INVALID COMMAND");
                break;
            }
        }
        else
            printf("INVALID SERVO");
    }
}

void lidar_demo(void){
    #define DATASET 50
    
    // uint8_t registers[21] = {0, 1, 2, 3, 4, 5, 6, 7, 64, 65, 66 ,86, 87, 22, 23, 24, 25, 26, 59, 60, 61};
    char *distance_diag[13] = {
        "ERROR -13: overexposure",
        "ERROR -12: no object detected",
        "ERROR -11: abnormal TOF image",
        "ERROR -10: abnormal temperature image",
        "ERROR -9: abnormal grey scale image",
        "ERROR -8: reserve",
        "ERROR -7: signal too weak",
        "ERROR -6: signal too strong",
        "ERROR -5: reserve",
        "ERROR -4: sample data below min value",
        "ERROR -3: sample data beyond max value",
        "ERROR -2: pixel saturation",
        "ERROR -1: SPI communication error",
    };
    uint8_t i;
    int16_t measurements[DATASET] = {0}, minimum, maximum;
    float variance, standard_deviation, mean;

    printf("\n");
    while(1){
        minimum = 30000;
        maximum = 0;
        mean = 0;
        variance = 0;
        standard_deviation = 0;
        printf("starting measurements.\n");
        for(i = 0; i < DATASET; i++){
            // acquire data and check for error messages
            measurements[i] = 0;
            while(measurements[i] < 1){
                measurements[i] = xts1_measure_distance();
                if(measurements[i] < 0)
                    printf("  --> %s\n", distance_diag[measurements[i] + 13]);
            }
            // check for minimum
            if(measurements[i] < minimum)
                minimum = measurements[i];
            // check for maximum
            if(measurements[i] > maximum)
                maximum = measurements[i];
            // calculating mean dynamically
            /*
            if(i == 0){
                mean = measurements[i];
            }
            else{
                mean *= i;
                mean += measurements[i];
                mean /= i + 1;
            }
            */
            delay_milli(20);
        }
        // calculating mean
        for(i = 0; i < DATASET; i++)
            mean += measurements[i];
        mean /= DATASET;
        // calculating variance
        for(i = 0; i < DATASET; i++)
            variance +=  pow(measurements[i] - mean, 2);
        variance /= DATASET;
        // calculating standard deviation
        standard_deviation = sqrt(variance);

        printf(" minimum = %5d \n", minimum);
        printf(" maximum = %5d \n", maximum);
        printf(" mean = %f \n", mean);
        printf(" variance = %f \n", variance);
        printf(" standard deviation = %f \n\n", standard_deviation);
    }
}
