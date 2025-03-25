#include <stdio.h>
#include "pico/stdlib.h"
#include "RPVX.h"
#include "hardware/pwm.h"

#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_descriptors.h"

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

//#define RED_RGB 18
//#define GREEN_RGB 19
//#define BLUE_RGB 20
//#define CYCLE_TIME_MS 5000

#define START_KEY HID_KEY_1
#define BT_A_KEY HID_KEY_D
#define BT_B_KEY HID_KEY_F
#define BT_C_KEY HID_KEY_J
#define BT_D_KEY HID_KEY_K
#define FX_L_KEY HID_KEY_C
#define FX_R_KEY HID_KEY_M

#define DEBOUNCE_DELAY_ENC_US 5000
//#define DEBOUNCE_DELAY_BTN_US 5000
#define MOUSE_DELTA 10

//bool buttons_sent = true;

// [VOL-L, VOL-R] [pin A, pin B]
bool vol[2][2];

// [VOL-L, VOL-R] [previous direction, current direction]
bool vol_directions[2][2] = { {0,0}, {0,0} };

// [VOL-L, VOL-R]
int8_t delta[2] = {0, 0}; // mouse delta
int64_t prev_time_enc[2] = {0, 0}; // time of previous encoder change
//int64_t prev_time_btn[7] = {0, 0, 0, 0, 0, 0, 0};

// [START, BT-A, BT-B, BT-C, BT-D, FX_L, FX_R]
bool button_states[7] = {0, 0, 0, 0, 0, 0, 0};
uint8_t button_maps[7] = {
    START_KEY,
    BT_A_KEY,
    BT_B_KEY,
    BT_C_KEY,
    BT_D_KEY,
    FX_L_KEY,
    FX_R_KEY
};


void init_pico ()
{
    init_button(START);
    init_button(BT_A);
    init_button(BT_B);
    init_button(BT_C);
    init_button(BT_D);
    init_button(FX_L);
    init_button(FX_R);
    init_encoder(VOL_L_A, VOL_L_B);
    init_encoder(VOL_R_A, VOL_R_B);
    //init_pwm(RED_RGB);
    //init_pwm(GREEN_RGB);
    //init_pwm(BLUE_RGB);
}

int main()
{
    stdio_init_all();
    init_pico();

    // TinyUSB initialization
    // init device stack on configured roothub port
    tusb_rhport_init_t dev_init = {
       .role = TUSB_ROLE_DEVICE,
       .speed = TUSB_SPEED_AUTO
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);

    while (1)
    {
        tud_task();
        bool button_new_states[7] = {
            gpio_get(START),
            gpio_get(BT_A),
            gpio_get(BT_B),
            gpio_get(BT_C),
            gpio_get(BT_D),
            gpio_get(FX_L),
            gpio_get(FX_R)
        };
        // temporarily store current VOL values (encoder states)
        bool vol_new[2][2] = {
            {gpio_get(VOL_L_A), gpio_get(VOL_L_B)},
            {gpio_get(VOL_R_A), gpio_get(VOL_R_B)}
        };

        // stores direction if encoder states are valid and different
        for (int i = 0; i < 2; ++i) {

            // invalid if 00 -> 11 or 11 -> 00
            if ((vol[i][0]!=vol_new[i][0]) && (vol[i][1]!=vol_new[i][1])) {
                //printf("Invalid state change.\n\n");
            }

            // no change, do nothing
            else if ((vol[i][0]==vol_new[i][0]) && (vol[i][1]==vol_new[i][1])) {}

            // valid change
            else {

                // grabs direction if outside of debounce window
                if (time_us_64() > (prev_time_enc[i] + DEBOUNCE_DELAY_ENC_US)) {
                    vol_directions[i][1] = encoder_direction(vol_new[i][0],vol_new[i][1], vol[i][0],vol[i][1]);

                    //if (vol_directions[i][0] == vol_directions[i][1]) { //----buffer for inputs----------- 
                        if(vol_directions[i][1]) {
                            
                            //printf("R / CW\t\t-->\n\n");
                            delta[i] = MOUSE_DELTA;

                            /*
                            if (time_us_64() > (prev_time_enc[i] + debounce_delay_us)) {
                                delta[i] = MOUSE_DELTA;
                                prev_time_enc[i] = time_us_64();
                            }
                            else {}
                            */
                        }
                        else {
                            //printf("L / CCW\t<--\n\n");
                            delta[i] = -MOUSE_DELTA;

                            /*
                            if (time_us_64() > (prev_time_enc[i] + DEBOUNCE_DELAY_ENC_US)) {
                                delta[i] = -MOUSE_DELTA;
                                prev_time_enc[i] = time_us_64();
                            }
                            else{}
                            */
                        }
                    //}
                    vol_directions[i][0] = vol_directions[i][1];
                    prev_time_enc[i] = time_us_64();
                }
                // ignores all signals in debounce window (in theory)
                else {}
            }
        }
        // send mouse report if any changes
        if (delta[0] != 0 || delta[1] != 0)
            send_mouse_report(); 

        // save new VOL values to stored values
        for (int a = 0; a < 2; ++a) {
            for (int b = 0; b < 2; ++b) {
                if (vol[a][b] != vol_new[a][b]) {
                    vol[a][b] = vol_new[a][b];
                    //changed = true;
                }
            }
        }

        //bool btn_changed = false;
        
        // check button states for changes
        for (int i = 0; i < 7; ++i) {
            if ( (button_states[i] != button_new_states[i]) ) {
                button_states[i] = button_new_states[i];
                //btn_changed = true;
            }
        }
        
        /*
        // check button states for changes with debounce
        for (int i = 0; i < 7; ++i) {
                if (time_us_64() > (prev_time_btn[i] + DEBOUNCE_DELAY_BTN_US)) {
                    if ( (button_states[i] != button_new_states[i]) && buttons_sent) {
                        button_states[i] = button_new_states[i];  
                        prev_time_btn[i] = time_us_64();
                        btn_changed = true;
                    }
                }
            }
            */

        // print button states if any changes
        //if (btn_changed || !buttons_sent) {
            //printf("Button states: %d %d %d %d %d %d %d\n", button_states[0], button_states[1], button_states[2],
            //        button_states[3], button_states[4], button_states[5], button_states[6]);
            //printf("VOL-L: %d %d\nVOL-R: %d %d\n\n",
            //        vol[0][0], vol[0][1], vol[1][0], vol[1][1]);
            send_kb_report(); // ***** always send kb report, let windows handle debounce :D ************
        //}
        
    }
} //-----------------end of main--------------------------------


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

void init_pwm(int pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    int slice = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, 65535);  // 16-bit resolution
    pwm_init(slice, &config, true);
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

// sends kb HID report via TinyUSB
void send_kb_report()
{
    // skips report if HID not ready
    if ( !tud_hid_ready() ) {
        //buttons_sent = false;
        return;
    }
    uint8_t keycode[6] = { 0 };
    int k = 0;
    for (int i = 0; i < 7; ++i)
    {
        if (button_states[i]) {
            // if k=6, start gets overwritten (low priority button)
            keycode[k%6] = button_maps[i];
            k++;
        }
    }

    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
    //buttons_sent = true;
}

// sends mouse HID report via TinyUSB
void send_mouse_report()
{
    // skips report if HID not ready
    if ( !tud_hid_ready() ) return;

    tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta[0], delta[1], 0, 0);
    delta[0] = 0;
    delta[1] = 0;
}


// ---------- TinyUSB unhappy without these :( ---------------

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;
}