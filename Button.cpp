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
 * Button.cpp
 *
 *  Created on: Mar 22, 2017
 *      Author: JakeSoft
 */

#include <Arduino.h>
#include "Button.h"

Button::Button(int aPin) :
mPin(aPin),
mCurrentPressedState(false),
mLastPressedState(false),
mPulseEdge(false),
mPressedTimestamp(0),
mHeldTime(0),
mPulseWidth(0)
{
	//Handled by initializer list
}

Button::~Button()
{
	// Do nothing
}

void Button::Init()
{
	pinMode(mPin, INPUT); //Initialize the pin mode
	digitalWrite(mPin, HIGH); //Set pull-up resistor
}

bool Button::IsPressed()
{
	bool lnPressed = false;
	if(LOW == digitalRead(mPin))
	{
		lnPressed = true;
	}

	return lnPressed;
}

bool Button::IsHeld()
{
	return(mHeldTime > 0);
}

unsigned int Button::GetHeldTime()
{
	return mHeldTime;
}

unsigned int Button::GetPulseWidth()
{
	return mPulseWidth;
}

bool Button::IsPulseEdge()
{
	return mPulseEdge;
}

void Button::Update()
{

	mCurrentPressedState = IsPressed();

	if(true == mCurrentPressedState)
	{
		//This is the leading edge
		if(false == mLastPressedState)
		{
			//Capture the time when button pressed
			mPressedTimestamp = millis();
			mPulseWidth = 0;
			mPulseEdge = false;
			mHeldTime = 0;
		}
		//Button is being held
		else
		{
			mHeldTime = millis() - mPressedTimestamp;
		}
	}
	else //Button is not being pressed
	{
		// This is the trailing edge
		if(true == mLastPressedState)
		{
			mPulseWidth = millis() - mPressedTimestamp;
			mPulseEdge = true;
		}
		else //Button is idle
		{
			mHeldTime = 0;
			mPulseEdge = false;
		}
	}

	mLastPressedState = mCurrentPressedState;
}

