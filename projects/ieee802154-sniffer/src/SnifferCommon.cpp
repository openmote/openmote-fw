/**
 * @file       SnifferCommon.cpp
 * @author     Pere Tuset-Peiro (peretuset@openmote.com)
 * @version    v0.1
 * @date       May, 2015
 * @brief
 *
 * @copyright  Copyright 2015, OpenMote Technologies, S.L.
 *             This file is licensed under the GNU General Public License v2.
 */

/*================================ include ==================================*/

#include <string.h>

#include "SnifferCommon.hpp"
#include "Gpio.hpp"

/*================================ define ===================================*/

/*================================ typedef ==================================*/

/*=============================== prototypes ================================*/

/*=============================== variables =================================*/

extern GpioOut led_red;
extern GpioOut led_orange;
extern GpioOut led_yellow;

const uint8_t SnifferCommon::broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const uint8_t SnifferCommon::ethernetType[2]     = {0x80, 0x9A};

/*================================= public ==================================*/

SnifferCommon::SnifferCommon(Board& board, Radio& radio, Aes& aes):
    board_(board), radio_(radio), aes_(aes), semaphore(false), \
    snifferRadioRxInitCallback_(this, &SnifferCommon::radioRxInitCallback), \
    snifferRadioRxDoneCallback_(this, &SnifferCommon::radioRxDoneCallback), \
    radioBuffer_ptr(radioBuffer), radioBuffer_len(sizeof(radioBuffer)), \
    aesBuffer_ptr(radioBuffer), aesBuffer_len(sizeof(radioBuffer)), \
    outputBuffer_ptr(outputBuffer), outputBuffer_len(sizeof(outputBuffer)),
    decrypt(false)
{
}

void SnifferCommon::init(void)
{
    // Get the EUI48
    board_.getEUI48(macAddress);

    // Set Radio receive callbacks
    radio_.setRxCallbacks(&snifferRadioRxInitCallback_, \
                          &snifferRadioRxDoneCallback_);

    // Enable Radio module
    radio_.enable();
    radio_.enableInterrupts();
}

void SnifferCommon::start(void)
{
    // Start receiving
    radio_.on();
    radio_.receive();
    led_orange.on();
}

void SnifferCommon::stop(void)
{
    // Stop receiving
    radio_.off();
    led_orange.off();
}

void SnifferCommon::setChannel(uint8_t channel)
{
    // Set the radio channel
    radio_.setChannel(channel);
}

void SnifferCommon::setKey(bool enable, uint8_t* key)
{
	// Mark if incoming frames should be decrypted
    decrypt = enable;

	// Set the key to decrypt incoming frames
	if (decrypt)
	{	
		led_yellow.on();
    	aes_.loadKey(key);
	}
	else
	{
		led_yellow.off();
	}
}

void SnifferCommon::initFrame(uint8_t* buffer, uint8_t length, int8_t rssi, uint8_t lqi, uint8_t crc)
{
    // Pre-calculate the frame length
    uint32_t frameLength = 6 + 2 + 6 + length + 2;

    // Check that we do not overflow the Ethernet buffer
    if (frameLength > outputBuffer_len)
    {
        return;
    }

    // Reset the Ethernet buffer length
    outputBuffer_len = 0;

    // Set MAC destination address
    memcpy(&outputBuffer[0], SnifferCommon::broadcastAddress, 6);
    outputBuffer_len += 6;

    // Set MAC source address
    memcpy(&outputBuffer[6], &macAddress, 6);
    outputBuffer_len += 6;

    // Set MAC type
    memcpy(&outputBuffer[12], SnifferCommon::ethernetType, 2);
    outputBuffer_len += 2;

    // Need to set the PHR field?
    // memset(&ethernetBuffer[14], length, 1);
    // ethernetBuffer_len += 1;

    // Copy the IEEE 802.15.4 payload
    memcpy(&outputBuffer[14], buffer, length);
    outputBuffer_len += length;

    // Ensure that we meet the minimum Ethernet frame size
    if (frameLength < 60)
    {
        // Calculate the remaining space
        uint32_t bytes = 60 - frameLength;

        // Fill the remaining space with zeros
        memset(&outputBuffer[outputBuffer_len], 0x00, bytes);
        outputBuffer_len += bytes;
    }

    // Copy the IEEE 802.15.4 RSSI
    outputBuffer[outputBuffer_len] = rssi;
    outputBuffer_len += 1;

    // Copy the IEEE 802.15.4 CRC and LQI
    outputBuffer[outputBuffer_len] = crc | lqi;
    outputBuffer_len += 1;
}

/*================================ private ==================================*/

void SnifferCommon::radioRxInitCallback(void)
{
    led_red.on();
}

void SnifferCommon::radioRxDoneCallback(void)
{
    led_red.off();
    semaphore.giveFromInterrupt();
}
