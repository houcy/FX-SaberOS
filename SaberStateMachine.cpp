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
 * SaberStateMachine.cpp
 *
 *  Created on: Mar 22, 2017
 *      Author: JakeSoft
 */


#include "SaberStateMachine.h"

#define SWITCH_DEBOUCE_TIME 50
#define POWER_DOWN_SWITCH_TIME 1500
#define BLADE_FLICKER 1
#define MIN_SWING_INTERVAL 200
#define MAX_SWING_INTERVAL 1000
#define CLASH_PULSE_TIME 100
#define POST_CLASH_SWING_SUPPRESS_TIME 1000
#define POWER_UP_TIME 1000
#define POWER_DOWN_TIME 1000

SaberStateMachine::SaberStateMachine(DIYinoSoundPlayer* apSoundPlayer,
	           	   	  Mpu6050LiteMotionManager* apMotionManger,
					  IBladeManager* apBlade,
					  Button* apActButton,
					  Button* apAuxButton) :
mpSoundPlayer(apSoundPlayer),
mpMotion(apMotionManger),
mpBlade(apBlade),
mpActButton(apActButton),
mpAuxButton(apAuxButton),
mLastClashTime(0),
mLastSwingTime(0)
{
	//Do nothing here, handled by initializer list
}

void SaberStateMachine::Init()
{
	//Set initial state to the boot-up state
	mState = eeBoot;

	//Initialize components
	mpBlade->Init();
	mpSoundPlayer->Init();
	mpMotion->Init();
	mpActButton->Init();
	mpAuxButton->Init();

	delay(100); //Give time for the MPU6050 and Sound chip to wake up
	//TODO: Load the volume level from EEPROM settings
	mpSoundPlayer->SetVolume(15); //Don't wake the family during late night mad scientist sessions!
	delay(100);
}

void SaberStateMachine::Body()
{
	//Update motion sensing
	mpMotion->Update();

	//Detect button presses
	mpActButton->Update();
	mpAuxButton->Update();

	//Perform state-specific actions
	switch(mState)
	{
	case eeBoot:
		//Set the current font
		//TODO: Load this from EEPROM
		mpSoundPlayer->SetFont(0);

		//Play the boot sound
		mpSoundPlayer->PlaySound(ESoundTypes::eeBootSnd, 0);
		delay(100);

		ChangeState(eeOff);
		break;
	case eeOff:
		if(mpActButton->IsPulseEdge() && mpActButton->GetPulseWidth() > SWITCH_DEBOUCE_TIME)
		{
			ChangeState(eePoweringUp);
		}

		break;
	case eePoweringUp:
		//Do these actions only once upon entering this state
		if(mIsNewState)
		{
			Serial.println("Powering Up"); //Debug
		}

		//Play the power up sound
		mpSoundPlayer->PlaySound(ESoundTypes::eePowerUpSnd, 0);
		//Turn on the blade
		//TODO: Load user settings from EEPROM to set color
		mpBlade->SetChannel(255, 0);
		mpBlade->SetChannel(0, 1);
		mpBlade->SetChannel(0, 2);

		//TODO: Use blade ramp time based on power-up sound time, hard-coded for now
		while(!mpBlade->PowerUp(POWER_UP_TIME - 5))
		{
			//Do nothing, just wait for power up complete
		}

		//Re-synchronize the motion manager
		for(unsigned int lUpd = 0; lUpd <= 5; lUpd++)
		{
			mpMotion->Update();
			delay(1);
		}

		ChangeState(eeOnIdle);
		break;
	case eeOnIdle:
		//Do these actions only once upon entering this state
		if(mIsNewState)
		{
			Serial.println("On."); //Debug

			//TODO: Load user settings from EEPROM to set color
			mpBlade->SetChannel(255, 0);
			mpBlade->SetChannel(0, 1);
			mpBlade->SetChannel(0, 2);
			mpBlade->PerformIO();
		}

		//User pressed the button, so turn off the saber
		if(mpActButton->IsHeld() && mpActButton->GetHeldTime() >= POWER_DOWN_SWITCH_TIME)
		{
			//Go to power down state
			ChangeState(eePoweringDown);
		}
		//Clash event detected
		else if(this->mpMotion->IsClash())
		{
			ChangeState(eeClash);
		}
		//Swing event detected
		else if(mpMotion->IsSwing() && mpMotion->GetSwingMagnitude() > eeSmall)
		{
			ChangeState(eeSwing);
		}
		//Re-launch hum after 30 seconds
		//TODO: Re-launch based on sound timings
		else if(millis() - mStateChangeTime >= 30000)
		{
			Serial.println("Hum re-launch.");
			mpSoundPlayer->PlaySound(ESoundTypes::eeHumSnd, 0);
			mStateChangeTime = millis(); //Reset the state change time so we don't keep repeating
		}
		else
		{
			//TODO: Use settings from EEPROM
			mpBlade->ApplyFlicker(BLADE_FLICKER);
		}
		break;
	case eeSwing:
		mpSoundPlayer->PlayRandomSound(ESoundTypes::eeSwingSnd);
		ChangeState(eePostSwing);
		break;
	case eePostSwing:
		//User wants to power down the saber
		if(mpActButton->IsHeld() && mpActButton->GetHeldTime() >= POWER_DOWN_SWITCH_TIME)
		{
			ChangeState(eePoweringDown);
		}
		//A clash happened
		else if(mpMotion->IsClash())
		{
			//Don't jam the sound card with too many requests
			while(millis() - mStateChangeTime <= 100)
			{
				delay(1);
				mpMotion->Update();
			}
			ChangeState(eeClash);
		}
		//Swing is over
		else if(!mpMotion->IsSwing() && millis() - mStateChangeTime >= MIN_SWING_INTERVAL)
		{
			ChangeState(eeOnIdle);
		}
		//Swing has gone on for a long time, exit this state so a new swing sound can play
		//TODO: Repeat swing based on sound timing
		else if(millis() - mStateChangeTime > MAX_SWING_INTERVAL)
		{
			ChangeState(eeOnIdle);
		}
		break;
	case eeClash:
		//Do these actions only once upon entering this state
		if(mIsNewState)
		{
			//Capture the time of the clash event
			mLastClashTime = millis();

			//Play a clash sound
			mpSoundPlayer->PlayRandomSound(ESoundTypes::eeClashSnd);

			//Set blade to the flash color
			//TODO: Load flash color settings from EEPROM
			mpBlade->SetChannel(255, 0);
			mpBlade->SetChannel(255, 1);
			mpBlade->SetChannel(0, 2);
			mpBlade->PerformIO();
		}

		if(millis() - mStateChangeTime > CLASH_PULSE_TIME)
		{
			//Set blade back to the normal color
			//TODO: Load color from EEPROM
			mpBlade->SetChannel(255, 0);
			mpBlade->SetChannel(0, 1);
			mpBlade->SetChannel(0, 2);
			mpBlade->PerformIO();

			ChangeState(eePostClash);
		}
		break;
	case eePostClash:
		if(mpMotion->IsClash() && millis() - mLastClashTime > 200)
		{
			//Respond to new clash events, but not at a rate faster than once per 200ms
			//This allows for clash to settle and avoids jamming the sound card
			Serial.print("Clash repeat. State time delta ="); //Debug
			Serial.println(millis() - mStateChangeTime);      //Debug
			ChangeState(eeClash);
		}
		//TODO: Use sound timings to decide when the clash sound is done playing
		else if(millis() - mStateChangeTime >= POST_CLASH_SWING_SUPPRESS_TIME)
		{
			ChangeState(eeOnIdle);
		}
		break;
	case eeLockup:
		//TBD
		break;
	case eeBlaster:
		//TBD
		break;
	case eePoweringDown:
		Serial.println("Powering down"); //Debug

		//Play power down sound
		mpSoundPlayer->PlaySound(ESoundTypes::eePowerDownSnd, 0);

		//Turn off the blade
		//TODO: Use sound timings to decide how long power-down should take
		while(!mpBlade->PowerDown(POWER_DOWN_TIME))
		{
			//Do nothing, just wait for power down
		}

		//Wait for user to let off the button
		while(mpActButton->IsHeld() || mpActButton->IsPulseEdge())
		{
			mpActButton->Update();
		}
		ChangeState(eeOff);
		break;
	case eeSwitchProfile:
		//TBD
		break;
	case eeMenu:
		//TBD
		break;
	default:
		//Do nothing
		break;
	}
	//TODO: Fill this in
}
