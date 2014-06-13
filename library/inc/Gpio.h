/*
 * Copyright 2013 OpenMote Technologies, S.L.
 */

/**
 *
 * @file       Gpio.h
 * @author     Pere Tuset-Peiro (peretuset@openmote.com)
 * @version    v0.1
 * @date       May, 2014
 * @brief
 * @ingroup
 *
 */

#ifndef GPIO_H_
#define GPIO_H_

#include "gpio.h"

#include "hw_memmap.h"
#include "hw_types.h"

typedef void (*callback_t)(void);

class Gpio
{

public:
    Gpio(uint32_t port_, uint8_t pin_);
    Gpio(uint32_t port_, uint8_t pin_, uint32_t edge_);
    uint32_t getPort(void);
    uint8_t getPin(void);
    void on(void);
    void off(void);
    void toggle(void);
    uint32_t status(void);
    void setCallback(callback_t callback_);
    void enableInterrupt(void);
    void disableInterrupt(void);
    void interruptHandler(void);
protected:
    uint32_t port;
    uint8_t pin;
    uint32_t dir;
    uint32_t edge;
    callback_t callback;
};

#endif /* GPIO_H_ */
