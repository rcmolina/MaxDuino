#ifndef BUTTONS_H_INCLUDED
#define BUTTONS_H_INCLUDED

extern bool lastbtn;

bool button_any();
bool button_play();
bool button_stop();
bool button_up();
bool button_down();
bool button_menu();
bool button_root();

void debounce(bool (*)());
void debouncemax(bool (*)());

void checkLastButton();
void setup_buttons();

#endif // BUTTONS_H_INCLUDED
