/*!
	\file ezchronos.h
	\brief Main openchronos include file
*/

#ifndef __EZCHRONOS_H__
#define __EZCHRONOS_H__

#include <msp430.h>

#include <stdlib.h>

#include "config.h"

/*!
	\brief Adds an entry to the main menu.
	\details This function is to be used by modules, so that they can be visible in the main menu. A good place to call this function is from the corresponding module's _init function.
	\note This function is NULL safe. You can set its parameters to NULL if you don't need their functionality.
*/
void menu_add_entry(
	void (*up_btn_fn)(void),    /*!< callback for up button presses. */
	void (*down_btn_fn)(void),  /*!< callback for down button presses. */
	void (*num_btn_fn)(void),   /*!< callback for num button presses. */
	void (*lstar_btn_fn)(void), /*!< callback for long star button presses. */
	void (*lnum_btn_fn)(void),  /*!< callback for long num button presses. */
	void (*activate_fn)(void),  /*!< callback for when the user switches into this entry in the menu. */
	void (*deactivate_fn)(void) /*!< callback for when the user switches out from this entry in the menu. */
);

void menu_item_next(void);

/*!
	\brief Enters edit mode.
	\details The edit mode is a mechanism that allows the user to change values being displayed in the screen. For example, if a clock alarm is being displayed, then edit mode can be used to increase/decrease the values of hours and minutes. A good place to call this function is from the module's lstar_btn_fn function (see menu_add_entry()).
*/
void menu_editmode_start(
	/*! callback for up/down button presses; a integer is passed as argument telling how much to increment or decrement a value. */
	void (* value_fn)(int8_t),
	/*! callback for when the users selects another value in the screen. At this function you should take care of stopping blinking of previous value and start blinking the newly selected value. */
	void (* next_item_fn)(void),
	/*! callback for when the user exits from the edit mode.*/
	void (* complete_fn)(void)
);

/* Include function defined in even_in_range.s TODO: do we even need this?? */
unsigned short __even_in_range(unsigned short __value, unsigned short __bound);

/*!
	\brief Handy prototype typedef for helpers_loop_up() and helpers_loop_down() functions.
*/
typedef void(* helpers_loop_fn_t)(uint8_t *, uint8_t, uint8_t);

/*!
	\brief Increment value by one without exiting the [lower, upper] interval.
	\details Increment value by one without exiting the [lower, upper] interval. If the value meets the upper bound, it is restarted from lower bound.
	\sa menu_editmode_start
*/
void helpers_loop_up(
	uint8_t *value, /*!< value a pointer to the variable to be incremented. */
	uint8_t lower,  /*!< lower the lower bound for the loop interval. */
	uint8_t upper   /*!< upper the upper bound for the loop interval. */
);

/*!
	\brief Decrement value by one without exiting the [lower, upper] interval.
	\details Decrement value by one without exiting the [lower, upper] interval. If the value meets the lower bound, it is restarted from upper bound.
	\sa menu_editmode_start
*/
void helpers_loop_down(
	uint8_t *value, /*!< value a pointer to the variable to be decremented. */
	uint8_t lower,  /*!< lower the lower bound for the loop interval. */
	uint8_t upper   /*!< upper the upper bound for the loop interval. */
);

/*!
	\brief List of possible message types for the message bus.
*/
/* WARNING: the enum values are optimized to work with some drivers.
	If you need to add a new entry, append it to the end! */
enum sys_message {
	/* drivers/rtca */
	SYS_MSG_RTC_ALARM		= BIT0, /*!< alarm event from the hardware RTC. */
	SYS_MSG_RTC_MINUTE	= BIT1, /*!< minute event from the hardware RTC. */
	SYS_MSG_RTC_HOUR		= BIT2, /*!< hour event from the hardware RTC. */
	SYS_MSG_RTC_DAY		= BIT3, /*!< day event from the hardware RTC. */
	SYS_MSG_RTC_MONTH		= BIT4, /*!< month event from the hardware RTC. */
	SYS_MSG_RTC_YEAR		= BIT5, /*!< year event from the hardware RTC. */
	/* drivers/timer */
	SYS_MSG_TIMER_1HZ		= BIT6, /*!< 1HZ event from the hardware TIMER_0. */
	SYS_MSG_TIMER_10HZ	= BIT7, /*!< 10HZ event from the hardware TIMER_0. */
	SYS_MSG_TIMER_PROG	= BIT8, /*!< programmable event from TIMER_0. */
};

/*!
	\brief Linked list of nodes listening to the message bus.
*/
struct sys_messagebus {
	/*! callback for receiving messages from the system bus */
	void (*fn)(enum sys_message);
	/*! bitfield of message types that the node wishes to receive */
	enum sys_message listens;
	/*! pointer to the next node in the list */
	struct sys_messagebus *next;
};

/*!
	\brief Registers a node in the message bus.
	\details Registers (add) a node to the message bus. A node can filter what message(s) are to be received by setting the bitfield \b listens.
	\sa sys_message, sys_messagebus, sys_messagebus_unregister
*/
void sys_messagebus_register(
	/*! callback to receive messages from the message bus */
	void (*callback)(enum sys_message),
	/*! only receive messages of this type */
	enum sys_message listens
);

/*!
	\brief Unregisters a node from the message bus.
	\sa sys_messagebus_register
*/
void sys_messagebus_unregister(
	/*! the same callback used on sys_messagebus_register() */
	void (*callback)(enum sys_message)
);

#endif /* __EZCHRONOS_H__ */
