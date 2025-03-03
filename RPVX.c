#include <stdio.h>
#include "pico/stdlib.h"
#include "RPVX.h"

#define START 22
#define BT_A 13
#define BT_B 12
#define BT_C 11
#define BT_D 10
#define FX_L 14
#define FX_R 15
#define VOL_L_A 16
#define VOL_L_B 17
#define VOL_R_A 0
#define VOL_R_B 1

// [VOL-L, VOL-R] [pin A, pin B]
bool vol[2][2];

// [VOL-L, VOL-R] [previous direction, current direction]
bool vol_directions[2][2] = { {0,0}, {0,0} };

// [START, BT-A, BT-B, BT-C, BT-D, FX_L, FX_R]
bool button_states[7];

void init_pico ()
{
    init_button(FX_R);
    init_encoder(VOL_R_A, VOL_R_B);
}

int main()
{
    stdio_init_all();
    init_pico();

    while (1)
    {
        bool button_new_states[7] = {
            gpio_get(START),
            gpio_get(BT_A),
            gpio_get(BT_B),
            gpio_get(BT_C),
            gpio_get(BT_D),
            gpio_get(FX_L),
            gpio_get(FX_R)
        };
        bool vol_new[2][2] = {
            {gpio_get(VOL_L_A), gpio_get(VOL_L_B)},
            {gpio_get(VOL_R_A), gpio_get(VOL_R_B)}
        };

        // prints direction if encoder states are valid and new
        for (int i = 0; i < 2; ++i) {
            // invalid if 00 -> 11 or 11 -> 00
            if ((vol[i][0]!=vol_new[i][0]) && (vol[i][1]!=vol_new[i][1])) {
                printf("Invalid state change.\n\n");
            }
            // do nothing if no change
            else if ((vol[i][0]==vol_new[i][0]) && (vol[i][1]==vol_new[i][1])) {}
            // valid change, grab direction
            else {
                vol_directions[i][1] = encoder_direction(vol_new[i][0],vol_new[i][1], vol[i][0],vol[i][1]);
                if (vol_directions[i][0] == vol_directions[i][1]){
                    if(vol_directions[i][1])
                        printf("R / CW\t\t-->\n\n");
                    else
                        printf("L / CCW\t<--\n\n");
                }
                else {}
                vol_directions[i][0] = vol_directions[i][1];
            }
        }
        // save new VOL values to stored values
        for (int a = 0; a < 2; ++a) {
            for (int b = 0; b < 2; ++b) {
                if (vol[a][b] != vol_new[a][b]) {
                    vol[a][b] = vol_new[a][b];
                    //changed = true;
                }
            }
        }

        bool changed = false;
        // check button states for changes
        for (int i = 0; i < 7; ++i) {
            if (button_states[i] != button_new_states[i]) {
                button_states[i] = button_new_states[i];
                changed = true;
            }
        }
        
        // print button states if any changes
        if (changed) {
            printf("Button states: %d %d %d %d %d %d %d\n", button_states[0], button_states[1], button_states[2],
                    button_states[3], button_states[4], button_states[5], button_states[6]);
            //printf("VOL-L: %d %d\nVOL-R: %d %d\n\n",
            //        vol[0][0], vol[0][1], vol[1][0], vol[1][1]);
        }
        
    }
}
void init_button (int pin)
{
    gpio_set_function (pin, GPIO_FUNC_SIO);
    gpio_pull_down (pin);
    gpio_set_dir (pin, GPIO_IN);
}

void init_encoder (int a, int b)
{
    gpio_set_function (a, GPIO_FUNC_SIO);
    gpio_set_function (b, GPIO_FUNC_SIO);
    gpio_disable_pulls (a);
    gpio_disable_pulls (b);
    gpio_set_dir (a, GPIO_IN);
    gpio_set_dir (b, GPIO_IN);
}

// returns 0 for CCW, 1 for CW turns
bool encoder_direction (bool a, bool b, bool oldA, bool oldB)
{
    if ( (a==1 && b==0 && oldA==1 && oldB==1) || (a==0 && b==0 && oldA==1 && oldB==0) ||
        (a==0 && b==1 && oldA==0 && oldB==0) || (a==1 && b==1 && oldA==0 && oldB==1))
        return 0;
    else
        return 1;
}