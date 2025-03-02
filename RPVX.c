#include <stdio.h>
#include "pico/stdlib.h"
#include "RPVX.h"

#define VOL_R_A 0
#define VOL_R_B 1
#define FX_R 15

bool vol_R_states[2][2];
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
            0, // Start
            0, // BT-A
            0, // BT-B
            0, // BT-C
            0, // BT-D
            0, // FX-L
            gpio_get(FX_R)
        };
        bool changed = false;
        for (int i = 0; i < 7; ++i) {
            if (button_states[i] != button_new_states[i]) {
                button_states[i] = button_new_states[i];
                changed = true;
            }
        }


        if (changed)
            printf("Button states: %d %d %d %d %d %d %d\n", button_states[0], button_states[1], button_states[2],
                    button_states[3], button_states[4], button_states[5], button_states[6]);

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