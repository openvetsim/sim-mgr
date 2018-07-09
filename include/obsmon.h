/*
 * obsmon.h
 *
 * Signals for Open Broadcast Studio control
 *
 * Copyright (C) 2018 Terence Kelleher. All rights reserved.
 *
 */

#ifndef _OBSMON_H
#define _OBSMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
	
#include <iostream>

#define MSG_BUFF_MAX	32
#define MOD		%


struct obsShmData
{
	int buff_size;
	int next_write;
	int next_read;
	int buff[MSG_BUFF_MAX];
	int obsRunning;
	char newFilename[512];
};
	
#define OBSMON_START	100
#define OBSMON_STOP		101
#define OBSMON_OPEN		102
#define OBSMON_CLOSE	103

#define OBS_SHM_NAME	"obs_shm"

#define	OPEN_WITH_CREATE		1
#define OPEN_ACCESS				0

#endif // _OBSMON_H