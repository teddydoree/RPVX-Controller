void init_button (int pin);

void init_encoder (int a, int b);

bool encoder_direction (bool a, bool b, bool newA, bool newB);

void send_kb_report();

void send_mouse_report();