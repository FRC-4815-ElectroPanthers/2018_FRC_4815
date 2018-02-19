/*
 * PixyI2C.cpp
 *
 *  Created on: Feb 14, 2018
 *      Author: 4815
 */
#include "PixyI2C.h"
#include "Timer.h"

PixyI2C::PixyI2C(uint8_t address, I2C::Port port): Wire(address, port){
	m_skipStart = false;
	m_blockType = NORMAL_BLOCK;
	m_blockCount = 0;
	m_blockArraySize = PIXY_INITIAL_ARRAYSIZE;
	//blocks = (Block *)malloc(sizeof(Block)*blockArraySize);

	m_freed = false;
	m_GetBlocks = std::thread(&PixyI2C::GetBlocks(), this);
}

PixyI2C::~PixyI2C(){
	m_freed = true;

	if(m_GetBlocks.joinable()){
		m_GetBlocks.join();
		//free(blocks);
	}
}

uint16_t PixyI2C::getWord(){
	uint8_t c[2];
	Wire.ReadOnly(2, c);
	uint16_t w = (c[1] << 8) + c[0];

	return w;
}

uint8_t PixyI2C::getByte(){
	uint8_t *c;
	Wire.ReadOnly(1, c);

	return *c;
}

int8_t PixyI2C::send(uint8_t *data, uint8_t len){
	Wire.WriteBulk(data, len);

	return len;
}

bool PixyI2C::GetStart(){
	uint16_t w, lastw;

	lastw = 0xffff;

	while(true){
		w = getWord();

		if (w==0 && lastw==0){
			Timer* delay = new Timer();
			delay->Start();

			while (!delay->HasPeriodPassed(0.00005)) {
				// Wait 50 microseconds
			}

			return false;

		}else if (w==PIXY_START_WORD && lastw==PIXY_START_WORD){
			std::lock_guard lock(m_mutex);

			m_blockType = NORMAL_BLOCK;
			return true;

		}else if (w==PIXY_START_WORD_CC && lastw==PIXY_START_WORD){
			std::lock_guard lock(m_mutex);

			m_blockType = CC_BLOCK;
			return true;

		}else if (w==PIXY_START_WORDX){
			printf("reorder");
			getByte(); // resync
		}

		lastw = w;
	}

}

/*
void PixyI2C::Resize(){
	std::lock_guard lock(m_mutex);

	blockArraySize += PIXY_INITIAL_ARRAYSIZE;
	blocks = (Block *)realloc(blocks, sizeof(Block)*blockArraySize);
}
*/

void PixyI2C::GetBlocks(uint16_t maxBlocks){
	uint8_t i;
	uint16_t w, checksum, sum, buffer[6];
	Block temp;
	//Block *block;

	BlockType blockType;
	uint16_t blockCount;
	std::vector<Block> block;

	{
		std::lock_guard lock(m_mutex);
		blockType = m_blockType;
	}

	while(!m_freed){
		blockCount = 0;
		if(!block.empty()){
			block.clear();
		}

		if (!m_skipStart){
			if (GetStart()==false){
				std::lock_guard lock(m_mutex);

				m_blockCount = blockCount;
				continue; //return;
			}
		}else{
			m_skipStart = false;
		}

		for(blockCount=0; blockCount<maxBlocks && blockCount<PIXY_MAXIMUM_ARRAYSIZE;){

			checksum = getWord();
			if (checksum==PIXY_START_WORD){
				// we've reached the beginning of the next frame
				m_skipStart = true;
				blockType = NORMAL_BLOCK;
				break;
			}else if (checksum==PIXY_START_WORD_CC){
				m_skipStart = true;
				blockType = CC_BLOCK;
				break;
			}else if (checksum==0){
				break;
			}

			/*
			if (m_blockCount>m_blockArraySize){
				m_Resize();
			}


			block = blocks + blockCount;
			*/

			for (i=0, sum=0; i < 6; i++){
				if (blockType==NORMAL_BLOCK && i==5){ //skip
					buffer[i] = 0;
					break;
				}

				buffer[i] = getWord();
				sum += buffer[i];
				//*((uint16_t *)block + i) = w;
			}
			temp.signature = buffer[0];
			temp.x = buffer[1];
			temp.y = buffer[2];
			temp.width= buffer[3];
			temp.height = buffer[4];
			temp.angle = buffer[5];

			block.push_back(temp);

			if (checksum==sum){
				blockCount++;
			}else{
				printf("CheckSum Error");
			}

			w = getWord();
			if (w==PIXY_START_WORD){
				blockType = NORMAL_BLOCK;
			}else if (w==PIXY_START_WORD_CC){
				blockType = CC_BLOCK;
			}else{
				break;
			}
		}

		{
			std::lock_guard lock(m_mutex);

			m_blockType = blockType;
			m_blockCount = blockCount;
			m_blocks = block;
		}
	}


}

uint16_t PixyI2C::GetBlockCount(){
	std::lock_guard lock(m_mutex);
	return m_blockCount;
}

uint16_t PixyI2C::GetBlockSignature(int block){
	std::lock_guard lock(m_mutex);
	return m_blocks[block].signature;
}

uint16_t PixyI2C::GetBlockX(int block){
	std::lock_guard lock(m_mutex);
	return m_blocks[block].x;
}

uint16_t PixyI2C::GetBlockY(int block){
	std::lock_guard lock(m_mutex);
	return m_blocks[block].y;
}

uint16_t PixyI2C::GetBlockWidth(int block){
	std::lock_guard lock(m_mutex);
	return m_blocks[block].width;
}

uint16_t PixyI2C::GetBlockHeight(int block){
	std::lock_guard lock(m_mutex);
	return m_blocks[block].height;
}

uint16_t PixyI2C::GetBlockAngle(int block){
	std::lock_guard lock(m_mutex);
	return m_blocks[block].angle;
}

int8_t PixyI2C::SetServos(uint16_t s0, uint16_t s1){
	uint8_t outBuf[6];

	outBuf[0] = 0x00;
	outBuf[1] = 0xff;
	*(uint16_t *)(outBuf + 2) = s0;
	*(uint16_t *)(outBuf + 4) = s1;

	return send(outBuf, 6);
}

int8_t PixyI2C::SetBrightness(uint8_t brightness){
	uint8_t outBuf[3];

	outBuf[0] = 0x00;
	outBuf[1] = 0xfe;
	outBuf[2] = brightness;

	return send(outBuf, 3);
}

int8_t PixyI2C::SetLED(uint8_t r, uint8_t g, uint8_t b){
	uint8_t outBuf[5];

	outBuf[0] = 0x00;
	outBuf[1] = 0xfd;
	outBuf[2] = r;
	outBuf[3] = g;
	outBuf[4] = b;

	return send(outBuf, 5);
}




