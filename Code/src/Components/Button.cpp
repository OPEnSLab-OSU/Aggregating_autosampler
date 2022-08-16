#include <Components/Button.hpp>

void Button::act(StateMachine & sm) {
	if (sm.isBusy()) {
		sm.halt();
	} else {
		sm.begin();
	}
}