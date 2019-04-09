/**
 * @file	io_pins.cpp
 * @brief	his file is about general input/output utility methods, not much EFI-specifics
 *
 *
 * @date Jan 24, 2013
 * @author Andrey Belomutskiy, (c) 2012-2018
 */

#include "global.h"

#if EFI_PROD_CODE
#include "io_pins.h"
#include "efi_gpio.h"

#include "pin_repository.h"
#include "status_loop.h"
#include "engine_configuration.h"
#include "console_io.h"

EXTERN_ENGINE;

#if EFI_ENGINE_CONTROL || defined(__DOXYGEN__)
#include "main_trigger_callback.h"
#endif /* EFI_ENGINE_CONTROL */

static LoggingWithStorage logger("io_pins");

extern EnginePins enginePins;
extern ioportid_t PORTS[];

ioportid_t getHwPort(const char *msg, brain_pin_e brainPin) {
	if (brainPin == GPIO_UNASSIGNED || brainPin == GPIO_INVALID)
		return GPIO_NULL;
	if (brainPin < GPIOA_0 || brainPin > GPIOH_15) {
		firmwareError(CUSTOM_ERR_INVALID_PIN, "%s: Invalid brain_pin_e: %d", msg, brainPin);
		return GPIO_NULL;
	}
	return PORTS[(brainPin - GPIOA_0)/ PORT_SIZE];
}

bool efiReadPin(brain_pin_e pin) {
	return palReadPad(getHwPort("readPin", pin), getHwPin("readPin", pin));
}

/**
 * This method would set an error condition if pin is already used
 */
void efiSetPadMode(const char *msg, brain_pin_e brainPin, iomode_t mode) {
	ioportid_t port = getHwPort(msg, brainPin);
	ioportmask_t pin = getHwPin(msg, brainPin);

	if (port == GPIO_NULL) {
		return;
	}
	efiAssertVoid(OBD_PCM_Processor_Fault, pin != EFI_ERROR_CODE, "pin_error");

	scheduleMsg(&logger, "%s on %s%d", msg, portname(port), pin);

	bool wasUsed = gpio_pin_markUsed(port, pin, msg);
	if (wasUsed) {
		return;
	}

	palSetPadMode(port, pin, mode);
}

iomode_t getInputMode(pin_input_mode_e mode) {
	switch (mode) {
	case PI_PULLUP:
		return PAL_MODE_INPUT_PULLUP;
	case PI_PULLDOWN:
		return PAL_MODE_INPUT_PULLDOWN;
	case PI_DEFAULT:
	default:
		return PAL_MODE_INPUT;
	}
}

#if HAL_USE_ICU || defined(__DOXYGEN__)
void efiIcuStart(const char *msg, ICUDriver *icup, const ICUConfig *config) {
	if (icup->state != ICU_STOP && icup->state != ICU_READY) {
		static char icuError[30];
		sprintf(icuError, "ICU already used %s", msg);
		firmwareError(CUSTOM_ERR_6679, icuError);
		return;
	}
	icuStart(icup, config);
}
#endif /* HAL_USE_ICU */
#endif
