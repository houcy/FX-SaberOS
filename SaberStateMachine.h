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
 * SaberStateMachine.h
 *
 *  Created on: Mar 22, 2017
 *      Author: JakeSoft
 */

#ifndef SABERSTATEMACHINE_H_
#define SABERSTATEMACHINE_H_

#include <USaber.h>
#include "StateMachine.h"
#include "Button.h"

/**
 * Enumeration of all possible saber states.
 */
enum ESaberState
{
	eeBoot,
	eeOff,
	eePoweringUp,
	eeOnIdle,
	eeSwing,
	eePostSwing,
	eeClash,
	eePostClash,
	eeLockup,
	eeBlaster,
	eePoweringDown,
	eeSwitchProfile,
	eeMenu
};

/**
 * This class serves as the primary state machine for the saber controlling
 * all higher-level functionality.
 */
class SaberStateMachine : public StateMachine
{
public:
	/**
	 * Constructor.
	 *   Args:
	 *     apSoundPlayer - Plays the sounds
	 *     apMotionManager - Detects motion
	 *     apBlade - Controls the blade
	 *     apActButton - Activation button handler
	 *     apAuxButton - Auxiliary button handler
	 */
	SaberStateMachine(DIYinoSoundPlayer* apSoundPlayer,
	           	   	  Mpu6050LiteMotionManager* apMotionManger,
					  IBladeManager* apBlade,
					  Button* apActButton,
					  Button* apAuxButton);

	/**
	 * Initialize components and get ready to run.
	 */
	void Init();

	/**
	 * Operates the saber state machine.
	 * Call this every cycle from the main loop.
	 */
	void Body();

private:

	DIYinoSoundPlayer* mpSoundPlayer; //Plays sounds
	Mpu6050LiteMotionManager* mpMotion; //Detects motion
	IBladeManager* mpBlade; //Controls the blade
	Button* mpActButton; //Activation button
	Button* mpAuxButton; //Auxiliary button

	unsigned long mLastClashTime; //Time when the last clash event occurred
	unsigned long mLastSwingTime; //Time when the last swing event occurred
};

#endif /* SABERSTATEMACHINE_H_ */
