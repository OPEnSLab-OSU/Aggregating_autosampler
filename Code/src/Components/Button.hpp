#include <KPFoundation.hpp>
#include <Components/StateMachine.hpp>
#include <Application/Constants.hpp>
class Button : public KPComponent {
public:
	const unsigned short pin;
	int buttonState;
	int lastButtonState;
	unsigned long lastDebounceTime;
	unsigned long debounceDelay;
	StateMachine & sm;

	Button(const char * name, KPController * controller, const unsigned short n_pin,
		StateMachine & sm, unsigned long n_delay = DefaultTimesButton::DEBOUNCE_TIME)
		: KPComponent(name, controller), pin(n_pin), debounceDelay(n_delay), sm(sm) {
		pinMode(pin, INPUT);
	}

	void setup() {}
	void update() {}

	void act(StateMachine & sm);
	void listen() {
		int reading = digitalRead(pin);

		if (reading != lastButtonState) {
			lastDebounceTime = millis();
		}
		if ((millis() - lastDebounceTime) > debounceDelay) {
			if (reading != buttonState) {
				buttonState = reading;
				if (buttonState == LOW) {
					act(sm);
				}
			}
		}
		lastButtonState = reading;
	}
};