void init_button (int pin);

void init_encoder (int a, int b);

void init_pwm(int pin);

bool encoder_direction (bool a, bool b, bool newA, bool newB);

bool send_kb_report();

void send_mouse_report();