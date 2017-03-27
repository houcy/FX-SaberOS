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
 * FX_SaberOS.ino
 *
 *  Created on: Mar 24, 2017
 *      Author: JakeSoft
 */

#include <Arduino.h>
#include <USaber.h>
#include "SaberStateMachine.h"
#include "Pins_DIYinoStardust.h"
#include "Button.h"

//Saber components
DIYinoSoundPlayer* gpSoundPlayer;
IBladeManager* gpBlade;
Mpu6050LiteMotionManager* gpMotion;

//Buttons
Button* gpActButton;
Button* gpAuxButton;

//Global configuration variables
DIYinoSoundMap gSoundMap; //Sound configuration data for the sound player
MPU6050LiteTolData gToleranceData; //Tolerance threshold data for MPU6050 motion manager

//The primary state machine for saber control
SaberStateMachine* gpStateMachine;

//The setup function is called once at startup of the sketch
void setup()
{
	delay(100);
	Serial.begin(9600); //For debugging

	//Default all values to zero in the sound map
    //Not strictly necessary, but a good idea
	memset(&gSoundMap, 0, sizeof(WT588DSoundMap));

	/***
	 * Set up sound map so sketch knows where sounds are on the SD card
	 * or SPI Flash and what features are supported. You only need to set the
	 * fields you intend to use. These values should be adjusted to match how
	 * your SD card or SPI Flash sounds are configured.
	 */
	gSoundMap.Features.FontIdsPerFont = 1;
	gSoundMap.Features.HumSoundsPerFont = 1;
	gSoundMap.Features.PowerUpSoundsPerFont = 1;
	gSoundMap.Features.PowerDownSoundsPerFont = 1;
	gSoundMap.Features.ClashSoundsPerFont = 8;
	gSoundMap.Features.SwingSoundsPerFont = 8;
	gSoundMap.Features.LockupSoundsPerFont = 1;
	gSoundMap.Features.BlasterSoundsPerFont = 1;
	gSoundMap.Features.ForceSoundsPerFont = 0;
	gSoundMap.Features.CustomSoundsPerFont = 0;
	gSoundMap.Features.MenuSounds = 19;

	//Sound locations
	gSoundMap.Locations.BaseAddr = 0;
	gSoundMap.Locations.BlasterBase = gSoundMap.Features.MenuSounds + 21;
	gSoundMap.Locations.BootBase = 11;
	gSoundMap.Locations.ClashBase = gSoundMap.Features.MenuSounds + 12;
	gSoundMap.Locations.SwingBase = gSoundMap.Features.MenuSounds + 4;
	gSoundMap.Locations.LockupBase = gSoundMap.Features.MenuSounds + 20;
	gSoundMap.Locations.PowerupBase = gSoundMap.Features.MenuSounds + 2;
	gSoundMap.Locations.PowerdownBase = gSoundMap.Features.MenuSounds + 22;
	gSoundMap.Locations.HumBase = gSoundMap.Features.MenuSounds + 3;
	gSoundMap.Locations.FontIdBase = gSoundMap.Features.MenuSounds + 1;
	gSoundMap.Locations.CustomBase = 0;
	gSoundMap.Locations.MenuBase = 1;

	//Create a sound player (using the map that was defined above)
	gpSoundPlayer = new DIYinoSoundPlayer(SOUND_TX_PIN, SOUND_RX_PIN, &gSoundMap);

	//Create a blade
	gpBlade = new RGBBlade(LED_LS1_PIN, LED_LS2_PIN, LED_LS3_PIN);

	//Motion manager swing tolerances
	//TODO: Read these from EEPROM, hard-coded for now
	gToleranceData.mSwingLarge  = 100;
	gToleranceData.mSwingMedium = 50;
	gToleranceData.mSwingSmall  = 25;
	gToleranceData.mClash = 10;
	gToleranceData.mTwist = gToleranceData.mSwingMedium;

	//Create motion manager
	gpMotion = new Mpu6050LiteMotionManager(&gToleranceData);

	//Create buttons
	//Use Stardust pinout
	gpActButton = new Button(BUTTON1_PIN);
	gpAuxButton = new Button(BUTTON2_PIN);
	//Use DIYino Prime V1 buttons for testing
//	gpActButton = new Button(4);
//	gpAuxButton = new Button(7);

	//Create the state machine
	gpStateMachine = new SaberStateMachine(gpSoundPlayer,
	           	   	   	   	   	   	   	   gpMotion,
										   gpBlade,
										   gpActButton,
										   gpAuxButton);

	gpStateMachine->Init();
}

// The loop function is called in an endless loop
void loop()
{
	//Run the Saber's primary state machine
	gpStateMachine->Operate();
}
