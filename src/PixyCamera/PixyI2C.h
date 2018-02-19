//
// begin license header
//
// This file is part of Pixy CMUcam5 or "Pixy" for short
//
// All Pixy source code is provided under the terms of the
// GNU General Public License v2 (http://www.gnu.org/licenses/gpl-2.0.html).
// Those wishing to use Pixy source code, software and/or
// technologies under different licensing terms should contact us at
// cmucam@cs.cmu.edu. Such licensing terms are available for
// all portions of the Pixy codebase presented here.
//
// end license header
//
// Note, the PixyI2C class takes two optional arguments, the first being the I2C address 
// of the Pixy you want to talk to and the second being the port on the RoboRIO you want to use (Onboard or MXP).
// The default address and port are 0x54 and kOnboard respectively.
// So, for example, if you wished to talk to Pixy at I2C address 0x55 and using the MXP port, declare like this:
//
// PixyI2C *Pixy = new PixyI2C(0x55, I2C::Port::kMXP);
//

#ifndef _PIXYI2C_H
#define _PIXYI2C_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "I2C.h"
#include "SensorBase.h"
//#include <support/mutex.h>
//#include <support/condition_variable.h>

// Communication/misc parameters
#define PIXY_INITIAL_ARRAYSIZE      30
#define PIXY_MAXIMUM_ARRAYSIZE      130
#define PIXY_START_WORD             0xaa55
#define PIXY_START_WORD_CC          0xaa56
#define PIXY_START_WORDX            0x55aa
#define PIXY_MAX_SIGNATURE          7
#define PIXY_DEFAULT_ARGVAL         0xffff

// Pixy x-y position values
#define PIXY_MIN_X                  0L
#define PIXY_MAX_X                  319L
#define PIXY_MIN_Y                  0L
#define PIXY_MAX_Y                  199L

// RC-servo values
#define PIXY_RCS_MIN_POS            0L
#define PIXY_RCS_MAX_POS            1000L
#define PIXY_RCS_CENTER_POS         ((PIXY_RCS_MAX_POS-PIXY_RCS_MIN_POS)/2)
#define PIXY_FOV					75

#define PIXY_I2C_DEFAULT_ADDR  0x54
#define PIXY_I2C_DEFAULT_PORT  I2C::Port::kOnboard

class PixyI2C{
public:
	PixyI2C(uint8_t address=PIXY_I2C_DEFAULT_ADDR, I2C::Port port=PIXY_I2C_DEFAULT_PORT);
	~PixyI2C();

	uint16_t GetBlockCount();
	uint16_t GetBlockSignature(int = 0);
	uint16_t GetBlockX(int = 0);
	uint16_t GetBlockY(int = 0);
	uint16_t GetBlockWidth(int = 0);
	uint16_t GetBlockHeight(int = 0);
	uint16_t GetBlockAngle(int = 0);

	int8_t SetServos(uint16_t s0, uint16_t s1);
	int8_t SetBrightness(uint8_t brightness);
	int8_t SetLED(uint8_t r, uint8_t g, uint8_t b);

private:
	enum BlockType{
	  NORMAL_BLOCK,
	  CC_BLOCK
	};

	struct Block {
		// print block structure!
		void print(){
			int i, j;
			char buf[128], sig[6], d;
			bool flag;

			if (signature>PIXY_MAX_SIGNATURE){ // color code! (CC)
				// convert signature number to an octal string
				for (i=12, j=0, flag=false; i>=0; i-=3){
					d = (signature>>i)&0x07;

					if (d>0 && !flag){
						flag = true;
					}

					if (flag){
						sig[j++] = d + '0';
					}
				}

				sig[j] = '\0';
				sprintf(buf, "CC block! sig: %s (%d decimal) x: %d y: %d width: %d height: %d angle %d\n", sig, signature, x, y, width, height, angle);
			}else{ // regular block.  Note, angle is always zero, so no need to print
				sprintf(buf, "sig: %d x: %d y: %d width: %d height: %d\n", signature, x, y, width, height);
			}
			printf(buf);
		}

		uint16_t signature;
		uint16_t x;
		uint16_t y;
		uint16_t width;
		uint16_t height;
		uint16_t angle;
	};

	I2C Wire;
	bool  m_skipStart;
	BlockType m_blockType;
	uint16_t m_blockCount;
	uint16_t m_blockArraySize;
	//Block *m_blocks;
	std::vector<Block> m_blocks;

	uint16_t getWord();
	uint8_t getByte();
	int8_t send(uint8_t *data, uint8_t len);

	void GetBlocks(uint16_t maxBlocks=1000);
	bool GetStart();
	//void Resize();

	std::atomic_bool m_freed;
	mutable std::mutex m_mutex;
	std::thread m_GetBlocks;
};

#endif
