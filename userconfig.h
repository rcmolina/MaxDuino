#define CONFIGFILE  7
//***********************************************************************************************************************//
//////////////////////          FAST CONFIG FRONT-END FOR CLASSIC ARDUINO NANO OR PRO MINI         ////////////////////////
//***********************************************************************************************************************//
#ifndef CONFIGFILE
#define CONFIGFILE 1
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*  1:OLD BOOTLOADER 30720, OLED1306_128_64, 0,96", SoftI2CMaster, I2CFAST, XY2, XY2force, AYPLAY, NO MenuBLK2A, NO Use_Dragon_sLeader, NO Use_hqUEF  */
#if CONFIGFILE == 1
#include "userconfig1.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*             2:OPTIBOOT BOOTLOADER 32256, OLED1106_1_3, 1,3", SoftI2CMaster, I2CFAST, XY2, XY2force, AYPLAY            */
#elif CONFIGFILE == 2
#include "userconfig2.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*          3:OPTIBOOT BOOTLOADER 32256, LCDSCREEN16x2, LCD_I2C_ADDR 0x27, SoftI2CMaster, I2CFAST, XY, AYPLAY            */
#elif CONFIGFILE == 3
#include "userconfig3.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*      4:PRO MICRO 28672, OLED1306, video64text32, 0,96", SoftI2CMaster, I2CFAST, XY, NO UEF, NO Use_Dragon_sLeader     */
#elif CONFIGFILE == 4
#include "userconfig4.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                        5:OLD BOOTLOADER 30720, video64text32, 0,96", SoftI2CMaster, I2CFAST, XY                       */
#elif CONFIGFILE == 5
#include "userconfig5.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                    6:PRO MICRO 28672, video64text32, 0,96", SoftI2CMaster, I2CFAST, XY, NO Use_CAS                    */
#elif CONFIGFILE == 6
#include "userconfig6.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*  7:OLD BOOTLOADER 30720, OLED1306_128_64, 0,96", SoftI2CMaster, I2CFAST, XY2, XY2force, NO MenuBLK2A, NO Use_UEF_c117 */
#elif CONFIGFILE == 7
#include "userconfig7.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
