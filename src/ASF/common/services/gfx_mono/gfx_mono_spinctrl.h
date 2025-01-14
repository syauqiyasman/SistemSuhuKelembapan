/**
 * \file
 *
 * \brief Spin control widget
 *
 * Copyright (c) 2011-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#ifndef GFX_MONO_SPINCTRL_H
#define GFX_MONO_SPINCTRL_H

#include "gfx_mono.h"
#include "conf_spinctrl.h"

/**
 * \ingroup gfx_mono
 * \defgroup gfx_mono_spinctrl Spinner widget for monochrome graphical displays
 *
 * This module provides a spinner widget system for monochrome graphical
 * displays.
 *
 * There is support for having one single spinner on the screen, or a
 * collection of spinners.
 *
 * Typical flow of an application using the spincollection system:
 *
 * 1) Define spinners.\n
 * 2) Initialize each spinners with \ref gfx_mono_spinctrl_init.\n
 * 3) Define a spincollection struct and initialize it with
 * \ref gfx_mono_spinctrl_spincollection_init.\n
 * 4) Add spinners to spincollection with
 * \ref gfx_mono_spinctrl_spincollection_add_spinner.\n
 * 5) Draw spincollection to screen with
 * \ref gfx_mono_spinctrl_spincollection_show.\n
 * 6) Define a result array to store the spinner choices.\n
 * 7) Update spinners and result array with user input using function
 * \ref gfx_mono_spinctrl_spincollection_process_key.\n
 * 8) Interpret \ref gfx_mono_spinctrl_spincollection_process_key return
 * value.\n
 * 9) Go to 7.\n
 *
 * Typical flow of an application using a single spinner:
 *
 * 1) Define spinner.\n
 * 2) Initialize the spinners with \ref gfx_mono_spinctrl_init with preferred y
 * position on screen.\n
 * 3) Draw spinner to screen with \ref gfx_mono_spinctrl_draw.\n
 * 4) Update spinner with user input using function
 * \ref gfx_mono_spinctrl_process_key.\n
 * 5) Interpret \ref gfx_mono_spinctrl_process_key return value.\n
 * 6) Go to 4.\n
 *
 * Before the spinners can be updated, you need input from the user. Methods
 * for getting input is not part of the spinner widget.
 *
 * \note The spinners will be linked together when added to a spincollection,
 * and can therefore not be used in two spincollections at the same time.
 *
 * As soon as input is received, inform the spincollection system or the single
 * spinner using the \ref gfx_mono_spinctrl_spincollection_process_key function
 * or the \ref gfx_mono_spinctrl_process_key function.
 * These functions will then return a status code and act depending on the
 * given keycode:
 *
 * GFX_MONO_SPINCTRL_KEYCODE_DOWN : Change selection to next spinner value or
 * to next spinner or OK button in a spincollection.
 *
 * GFX_MONO_SPINCTRL_KEYCODE_UP : Change selection to previous spinner value
 * or to previous spinner or OK button in a spincollection.
 *
 * GFX_MONO_SPINCTRL_KEYCODE_ENTER : Select spinner value or select spinner or
 * OK button in a spincollection.
 *
 * GFX_MONO_SPINCTRL_KEYCODE_BACK : Deselect spinner or cancel spincollection
 * application.
 *
 * The value of the keycodes used are defined in conf_spinctrl.h. These values
 * can be changed if needed.
 *
 * The graphical indicators used to indicate spinner selections are defined in
 * conf_spinctrl.h. These indicators can be changed if needed.
 * @{
 */

/** Spinner idle event */
#define GFX_MONO_SPINCTRL_EVENT_IDLE                0xFF
/** Spinner back button pressed event */
#define GFX_MONO_SPINCTRL_EVENT_BACK                0xFE
/** Spinner ok button pressed event */
#define GFX_MONO_SPINCTRL_EVENT_FINISH              0xFD

/** OK button */
#define GFX_MONO_SPINCTRL_BUTTON        0xFF

/** Maximum number of spinner elements on display */
#define GFX_MONO_SPINCTRL_ELEMENTS_PER_SCREEN \
	((GFX_MONO_LCD_HEIGHT / SYSFONT_LINESPACING) - 1)

/**
 * Maximum numbers of spinner elements in a spincollection - limited to
 * one screen.
 */
#define GFX_MONO_SPINCTRL_MAX_ELEMENTS_IN_SPINCOLLECTION \
	GFX_MONO_SPINCTRL_ELEMENTS_PER_SCREEN

/** Width of string spinner choices */
#define GFX_MONO_SPINCTRL_STRING_SPINNER_WIDTH 9
/** Width of integer spinner choices */
#define GFX_MONO_SPINCTRL_INT_SPINNER_WIDTH    9

/** Enum to specify what kind of data spinner should spin */
typedef enum gfx_mono_spinctrl_type_enum {
	SPINTYPE_STRING,
	SPINTYPE_INTEGER
} gfx_mono_spinctrl_type_t;

/** String struct */
struct gfx_mono_spinctrl_string {
	/** Pointer to progmem strings to spin through
	 * \note Each string must be shorter than
	 * \ref  GFX_MONO_SPINCTRL_STRING_SPINNER_WIDTH characters.
	 * If not, printing it to the screen will corrupt the spinner
	 * appearance.
	 */
	PROGMEM_STRING_T *data;
	/** Index in string array */
	uint8_t index;
};

/** Spin control struct */
struct gfx_mono_spinctrl {
	/** Spinner title */
	PROGMEM_STRING_T title;
	/** Type of data to spin */
	gfx_mono_spinctrl_type_t datatype;
	/** Spinner data, depends on spinner datatype. */
	union {
		/** Spinner strings and index */
		struct gfx_mono_spinctrl_string strings;
		/** Spinner integer data */
		int16_t integer_data;
	};
	/** Variable to store the last selected spinner value */
	uint16_t last_saved_value;

	/**
	 * Lower limit for spinning, must be positive and fit in uin8_t for
	 * spinner type SPINTYPE_STRING
	 */
	int16_t lower_limit;

	/**
	 * Upper limit for spinning, must be positive and fit in uin8_t for
	 * spinner type SPINTYPE_STRING
	 */
	int16_t upper_limit;
	/** Y coordinate for placement of spinner on screen */
	gfx_coord_t y;
	/** Boolean to tell if spinner is in focus or not */
	bool in_focus;
	/** Pointer to next spinner in a spincollection */
	struct gfx_mono_spinctrl *next;
	/** Pointer to previous spinner in a spincollection */
	struct gfx_mono_spinctrl *prev;
};

/** Collection of spinners struct */
struct gfx_mono_spinctrl_spincollection {
	/** Pointer to the first spinner in the collection */
	struct gfx_mono_spinctrl *collection;
	/** Pointer to the last spinner in the collection */
	struct gfx_mono_spinctrl *collection_last;
	/** Number of spinners in collection */
	uint8_t number_of_spinners;
	/** Current spinner/button */
	uint8_t current_selection;
	/** Return value from selected spinner */
	uint16_t selection;
	/** Boolean to tell if input should be sent directly to a spinner */
	bool active_spinner;
	/** Boolean to initialize results array when starting key processing */
	bool init;
};

void gfx_mono_spinctrl_init(struct gfx_mono_spinctrl *spinner,
		gfx_mono_spinctrl_type_t datatype, PROGMEM_STRING_T title,
		PROGMEM_STRING_T *data, int16_t lower_limit,
		int16_t upper_limit,
		gfx_coord_t y);
void gfx_mono_spinctrl_draw(struct gfx_mono_spinctrl *spinner, bool redraw);
void gfx_mono_spinctrl_spincollection_init(struct
		gfx_mono_spinctrl_spincollection *collection);
void gfx_mono_spinctrl_spincollection_add_spinner(struct gfx_mono_spinctrl
		*spinner, struct gfx_mono_spinctrl_spincollection *spinners);
void gfx_mono_spinctrl_spincollection_show(struct
		gfx_mono_spinctrl_spincollection *spinners);
int16_t gfx_mono_spinctrl_process_key(struct gfx_mono_spinctrl *spinner,
		uint8_t keycode);

int16_t gfx_mono_spinctrl_spincollection_process_key(struct
gfx_mono_spinctrl_spincollection *spinners, uint8_t keycode,
int16_t results[]);

/** @} */

#endif /* GFX_MONO_SPINCTRL_H */
