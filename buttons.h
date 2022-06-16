#ifndef BUTTONS_H_INCLUDED
#define BUTTONS_H_INCLUDED

bool button_any();
bool button_play();
bool button_stop();
bool button_up();
bool button_down();
bool button_menu();

void debounce(bool (*)());
void debouncemax(bool (*)());

#endif // BUTTONS_H_INCLUDED
