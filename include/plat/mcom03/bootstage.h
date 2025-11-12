/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2025 RnD Center "ELVEES", JSC  */

#ifndef _BOOTSTAGE_H
#define _BOOTSTAGE_H

#include <stdbool.h>
#include <stdint.h>

/*
 * A list of boot stages that we know about. Each of these indicates the
 * state that we are at, and the action that we are about to perform.
 */
enum bootstage_id {
	BOOTSTAGE_ID_RESERVED = 0,
	BOOTSTAGE_ID_SBL_S1_START,
	BOOTSTAGE_ID_DDRINIT_START,
	BOOTSTAGE_ID_SBL_S2_START,
	BOOTSTAGE_ID_SBL_S2_LOAD_START,
	BOOTSTAGE_ID_SBL_S2_LOAD_COMPLETE,
	BOOTSTAGE_ID_SBL_S3_START,
	BOOTSTAGE_ID_TF_A_START,

	// The IDs below has the same value as in the U-Boot in order to reused it
	BOOTSTAGE_ID_RUN_OS = 15,
	BOOTSTAGE_ID_START_UBOOT_F = 178,
	BOOTSTAGE_ID_START_TFTF = BOOTSTAGE_ID_START_UBOOT_F,
};

/**
 * Record bootstage with passed id
 * @id: Bootstage id to record this timestamp against
 *
 * Return: recorded time stamp
 */
uint64_t bootstage_mark(enum bootstage_id id);

/**
 * Get timestamp for Bootstage ID
 *
 * @param id	Bootstage ID to get timestamp from
 * Return: timestamp if found, -EBOOTSTAGE if error
 */
int64_t bootstage_get_timestamp(enum bootstage_id id);

/**
 * Export bootstage data into memory
 *
 * @param base	Base address of memory buffer
 * @param size	Size of memory buffer
 * Return: 0 if exported ok, -EBOOTSTAGE if error
 */
int bootstage_export(void *base, int size);

/**
 * Import bootstage data from memory
 *
 * Bootstage data is read from memory and placed in the bootstage table
 * in the user records.
 *
 * @param base	Base address of memory buffer
 * @param size	Size of memory buffer (-1 if unknown)
 * Return: 0 if imported ok, -EBOOTSTAGE if error
 */
int bootstage_import(const void *base, int size);

/**
 * Prepare bootstage for use
 *
 * @param id	Bootstage ID to init
 */
int bootstage_init(enum bootstage_id id);

#endif
