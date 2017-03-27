/*
 This file is part of the FX-Saber Operating System (FX-SaberOS).

 FX-SaberOS is free software: you can redistribute it
 and/or modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of the License,
 or (at your option) any later version.

 The FX-SaberOS software is distributed in the hope that it will be
 useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with the FX-SaberOS software.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
 * Button.h
 *
 *  Created on: Mar 22, 2017
 *      Author: JakeSoft
 */

#ifndef BUTTON_H_
#define BUTTON_H_

/**
 * This class used to monitor an input button. The button is assumed to be
 * placed between the input pin and ground. It is assumed the pin is pulled to
 * ground when the button is pressed. This class will automatically set the
 * internal pull-up resistor on initialization.
 */
class Button {
public:
	/**
	 * Constructor.
	 * Args:
	 *   aPin - Pin to use for input.
	 */
	Button(int aPin);

	/**
	 * Destructor.
	 */
	virtual ~Button();

	/**
	 * Call this method to read the latest pin status and update
	 * class member data accordingly. Recommended to call this
	 * method at least once on each main program loop.
	 */
	void Update();

	/**
	 * Is the button being held right now?
	 * Returns:
	 *   TRUE if the button is held, FALSE otherwise.
	 */
	bool IsHeld();

	/**
	 * How long has the button been held?
	 * Returns:
	 *   Time (in milliseconds) that the button has been held.
	 */
	unsigned int GetHeldTime();

	/**
	 * How long was the button held for before being released?
	 * Returns:
	 *  Time (in milliseconds) that the button was last held before being
	 *  released.
	 */
	unsigned int GetPulseWidth();

	/**
	 * If the button state changed from being pressed to not pressed during
	 * the last update, then this method will return TRUE.
	 * Returns:
	 *   TRUE if last update was a pulse edge, FALSE otherwise.
	 */
	//Returns TRUE on trailing edge of a press pulse
	bool IsPulseEdge();

	/**
	 * Initialize the I/O pin.
	 */
	void Init();

protected:
	/**
	 * Digital read to see if button is pressed
	 * Returns:
	 *  TRUE if button press detected, FALSE otherwise.
	 */
	bool IsPressed();
private:

	//Input button pin to monitor
	int mPin;

	//Flag keeps track of if button is currently pressed
	bool mCurrentPressedState;
	//Flag keeps track of the pressed state from previous cycle
	bool mLastPressedState;
	// If this is the trailing edge of a press pulse
	bool mPulseEdge;

	//Time when the button was first pressed
	unsigned long mPressedTimestamp;

	//How long has the button been held
	unsigned int mHeldTime;

	//How long was the button held and then released
	unsigned int mPulseWidth;
};

#endif /* BUTTON_H_ */
