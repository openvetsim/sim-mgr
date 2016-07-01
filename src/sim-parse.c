/*
 * scenario.cpp
 *
 * Parser for "init" and Instructor commands. Shared by simstatus.cpp and scenario.c
 *
 * Copyright (C) 2016 Terence Kelleher. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

#include "../include/scenario.h"

int
cardiac_parse(const char *elem,  const char *value, struct cardiac *card )
{
	int sts = 0;
	
	if ( ( ! elem ) || ( ! value) || ( ! card ) )
	{
		return ( -11 );
	}
	if ( strcmp(elem, ("rhythm" ) ) == 0 )
	{
		sprintf(card->rhythm, "%s", value );
	}
	else if ( strcmp(elem, ("vpc" ) ) == 0 )
	{
		sprintf(card->vpc, "%s", value );
	}
	else if ( strcmp(elem, ("pea" ) ) == 0 )
	{
		card->pea = atoi(value );
	}
	else if ( strcmp(elem, ("vpc_freq" ) ) == 0 )
	{
		card->vpc_freq = atoi(value );
	}
	else if ( strcmp(elem, ("vfib_amplitude" ) ) == 0 )
	{
		sprintf(card->vfib_amplitude, "%s", value );
	}
	else if ( strcmp(elem, ("pwave" ) ) == 0 )
	{
		sprintf(card->pwave, "%s", value );
	}
	else if ( strcmp(elem, ("rate" ) ) == 0 )
	{
		card->rate = atoi(value );
	}
	else if ( strcmp(elem, ("transfer_time" ) ) == 0 )
	{
		card->transfer_time = atoi(value );
	}
	else if ( strcmp(elem, ("pr_interval" ) ) == 0 )
	{
		card->pr_interval = atoi(value );
	}
	else if ( strcmp(elem, ("qrs_interval" ) ) == 0 )
	{
		card->qrs_interval = atoi(value );
	}
	else if ( strcmp(elem, ("bps_sys" ) ) == 0 )
	{
		card->bps_sys = atoi(value );
	}
	else if ( strcmp(elem, ("bps_dia" ) ) == 0 )
	{
		card->bps_dia = atoi(value );
	}
	else if ( strcmp(elem, ("nibp_rate" ) ) == 0 )
	{
		card->nibp_rate = atoi(value );
	}
	else if ( strcmp(elem, ("ecg_indicator" ) ) == 0 )
	{
		card->ecg_indicator = atoi(value );
	}
	else if ( strcmp(elem, ("heart_sound" ) ) == 0 )
	{
		sprintf(card->heart_sound, "%s", value );
	}
	else if ( strcmp(elem, ("heart_sound_volume" ) ) == 0 )
	{
		card->heart_sound_volume = atoi(value );
	}
	else if ( strcmp(elem, ("heart_sound_mute" ) ) == 0 )
	{
		card->heart_sound_mute = atoi(value );
	}
	else if ( strcmp(elem, ("pulse_strength" ) ) == 0 )
	{
		if ( (strcmp(value, "none" ) ) == 0 )
		{
			card->pulse_strength = 0;
		}
		else if ( (strcmp(value, "weak" ) ) == 0 )
		{
			card->pulse_strength = 1;
		}
		else if ( (strcmp(value, "medium" ) ) == 0 )
		{
			card->pulse_strength = 2;
		}
		else if ( (strcmp(value, "strong" ) ) == 0 )
		{
			card->pulse_strength = 3;
		}
		else
		{
			sts = 3;
		}
	}
	else
	{
		sts = 1;
	}
	return ( sts );
}

int
respiration_parse(const char *elem,  const char *value, struct respiration *resp )
{
	int sts = 0;
	
	if ( ( ! elem ) || ( ! value) || ( ! resp ) )
	{
		return ( -12 );
	}
	if ( strcmp(elem, "left_lung_sound" ) == 0 )
	{
		sprintf(resp->left_lung_sound, "%s", value );
	}
	else if ( strcmp(elem, "right_lung_sound" ) == 0 )
	{
		sprintf(resp->right_lung_sound, "%s", value );
	}
	else if ( strcmp(elem, "inhalation_duration" ) == 0 )
	{
		resp->inhalation_duration = atoi(value );
	}
	else if ( strcmp(elem, "exhalation_duration" ) == 0 )
	{
		resp->exhalation_duration = atoi(value );
	}
	else if ( strcmp(elem, "left_lung_sound_volume" ) == 0 )
	{
		resp->left_lung_sound_volume = atoi(value );
	}
	else if ( strcmp(elem, "left_lung_sound_mute" ) == 0 )
	{
		resp->left_lung_sound_mute = atoi(value );
	}
	else if ( strcmp(elem, "right_lung_sound_volume" ) == 0 )
	{
		resp->right_lung_sound_volume = atoi(value );
	}
	else if ( strcmp(elem, "right_lung_sound_volume" ) == 0 )
	{
		resp->right_lung_sound_volume = atoi(value );
	}
	else if ( strcmp(elem, "right_lung_sound_mute" ) == 0 )
	{
		resp->right_lung_sound_mute = atoi(value );
	}
	else if ( strcmp(elem, "rate" ) == 0 )
	{
		resp->rate = atoi(value );
	}
	else if ( strcmp(elem, "spo2" ) == 0 )
	{
		resp->spo2 = atoi(value );
	}
	else if ( strcmp(elem, "etco2" ) == 0 )
	{
		resp->etco2 = atoi(value );
	}
	else if ( strcmp(elem, "transfer_time" ) == 0 )
	{
		resp->transfer_time = atoi(value );
	}
	else if ( strcmp(elem, "etco2_indicator" ) == 0 )
	{
		resp->etco2_indicator = atoi(value );
	}
	else if ( strcmp(elem, "spo2_indicator" ) == 0 )
	{
		resp->spo2_indicator = atoi(value );
	}
	else if ( strcmp(elem, "chest_movement" ) == 0 )
	{
		resp->chest_movement = atoi(value );
	}
	else
	{
		sts = 1;
	}
	return ( sts );
}
int
general_parse(const char *elem,  const char *value, struct general *gen )
{
	int sts = 0;
	if ( ( ! elem ) || ( ! value) || ( ! gen ) )
	{
		return ( -13 );
	}
	if ( strcmp(elem, "temperature" ) == 0 )
	{
		gen->temperature = atoi(value );
	}
	else if ( strcmp(elem, "transfer_time" ) == 0 )
	{
		gen->transfer_time = atoi(value );
	}
	else
	{
		sts = 1;
	}
	return ( sts );
}
	
int
vocals_parse(const char *elem,  const char *value, struct vocals *voc )
{
	int sts = 0;
	
	if ( ( ! elem ) || ( ! value) || ( ! voc ) )
	{
		return ( -14 );
	}
	if ( strcmp(elem, "filename" ) == 0 )
	{
		sprintf(voc->filename, "%s", value );
	}
	else if ( strcmp(elem, "repeat" ) == 0 )
	{
		voc->repeat = atoi(value );
	}
	else if ( strcmp(elem, "volume" ) == 0 )
	{
		voc->volume = atoi(value );
	}
	else if ( strcmp(elem, "play" ) == 0 )
	{
		voc->play = atoi(value );
	}
	else if ( strcmp(elem, "mute" ) == 0 )
	{
		voc->mute = atoi(value );
	}
	else
	{
		sts = 1;
	}
	return ( sts );
}
int
media_parse(const char *elem,  const char *value, struct media *med )
{
	int sts = 0;
	
	if ( ( ! elem ) || ( ! value) || ( ! med ) )
	{
		return ( -14 );
	}
	if ( strcmp(elem, "filename" ) == 0 )
	{
		sprintf(med->filename, "%s", value );
	}
	else if ( strcmp(elem, "play" ) == 0 )
	{
		med->play = atoi(value );
	}
	else
	{
		sts = 1;
	}
	return ( sts );
}

/**
* initializeParameterStruct
* @initParams: Pointer to a "struct instructor"
*
* Initialize the paraneters structure to be "no set" on all parameters.
* For strings, they are null. For numerics, they are -1.
*/

void
initializeParameterStruct(struct instructor *initParams )
{
	memset(initParams, 0, sizeof(struct instructor) );
	
	initParams->cardiac.vpc_freq = -1;
	initParams->cardiac.pea = -1;
	initParams->cardiac.rate = -1;
	initParams->cardiac.nibp_rate = -1;
	initParams->cardiac.pr_interval = -1;
	initParams->cardiac.qrs_interval = -1;
	initParams->cardiac.bps_sys = -1;
	initParams->cardiac.bps_dia = -1;
	initParams->cardiac.pulse_strength = -1;
	initParams->cardiac.heart_sound_volume = -1;
	initParams->cardiac.heart_sound_mute = -1;
	initParams->cardiac.ecg_indicator = -1;
	initParams->cardiac.transfer_time = -1;
	
	initParams->respiration.inhalation_duration = -1;
	initParams->respiration.exhalation_duration = -1;
	initParams->respiration.left_lung_sound_volume = -1;
	initParams->respiration.left_lung_sound_mute = -1;
	initParams->respiration.right_lung_sound_volume = -1;
	initParams->respiration.right_lung_sound_mute = -1;
	initParams->respiration.spo2 = -1;
	initParams->respiration.rate = -1;
	initParams->respiration.etco2 = -1;
	initParams->respiration.etco2_indicator = -1;
	initParams->respiration.spo2_indicator = -1;
	initParams->respiration.chest_movement = -1;
	initParams->respiration.transfer_time = -1;
	
	initParams->general.temperature = -1;
	initParams->general.transfer_time = -1;
	
	initParams->vocals.repeat = -1;
	initParams->vocals.volume = -1;
	initParams->vocals.play = -1;
	initParams->vocals.mute = -1;
	
	initParams->media.play = -1;
	
	initParams->scenario.record = -1;
}

/**
* processInit
* @initParams: Pointer to a "struct instructor"
*
* Transfer the instructions from the initParams to the instructor portion of
* the shared data space, to activate all the controls.
*/
void
processInit(struct instructor *initParams  )
{
	takeInstructorLock();
		
	// Copy initParams to shared instructor interface (not the sema or scenario sections)

	memcpy(&simmgr_shm->instructor.cardiac, &initParams->cardiac, sizeof(struct cardiac) );
	memcpy(&simmgr_shm->instructor.respiration, &initParams->respiration, sizeof(struct respiration) );
	memcpy(&simmgr_shm->instructor.general, &initParams->general, sizeof(struct general) );
	memcpy(&simmgr_shm->instructor.vocals, &initParams->vocals, sizeof(struct vocals) );
	memcpy(&simmgr_shm->instructor.media, &initParams->media, sizeof(struct media) );
	
	releaseInstructorLock();
	
	// Delay to allow simmgr to pick up the changes
	usleep(500000 );
	
}
int
getValueFromName(char *param_class, char *param_element )
{
	int rval = -1;
	
	if ( strcmp(param_class, "cardiac" ) == 0 )
	{
		if ( strcmp(param_element, "vpc_freq" ) == 0 )
			rval = simmgr_shm->status.cardiac.vpc_freq;
		else if ( strcmp(param_element, "pea" ) == 0 )
			rval = simmgr_shm->status.cardiac.pea;
		else if ( strcmp(param_element, "rate" ) == 0 )
			rval = simmgr_shm->status.cardiac.rate;
		else if ( strcmp(param_element, "nibp_rate" ) == 0 )
			rval = simmgr_shm->status.cardiac.nibp_rate;
		else if ( strcmp(param_element, "pr_interval" ) == 0 )
			rval = simmgr_shm->status.cardiac.pr_interval;
		else if ( strcmp(param_element, "qrs_interval" ) == 0 )
			rval = simmgr_shm->status.cardiac.qrs_interval;
		else if ( strcmp(param_element, "bps_sys" ) == 0 )
			rval = simmgr_shm->status.cardiac.bps_sys;
		else if ( strcmp(param_element, "bps_dia" ) == 0 )
			rval = simmgr_shm->status.cardiac.bps_dia;
		else if ( strcmp(param_element, "ecg_indicator" ) == 0 )
			rval = simmgr_shm->status.cardiac.ecg_indicator;
	}
	else if ( strcmp(param_class, "respiration" ) == 0 )
	{
		if ( strcmp(param_element, "spo2" ) == 0 )
			rval = simmgr_shm->status.respiration.spo2;
		else if ( strcmp(param_element, "rate" ) == 0 )
			rval = simmgr_shm->status.respiration.rate;
		else if ( strcmp(param_element, "etco2" ) == 0 )
			rval = simmgr_shm->status.respiration.etco2;
		else if ( strcmp(param_element, "etco2_indicator" ) == 0 )
			rval = simmgr_shm->status.respiration.etco2_indicator;
		else if ( strcmp(param_element, "spo2_indicator" ) == 0 )
			rval = simmgr_shm->status.respiration.spo2_indicator;
		else if ( strcmp(param_element, "chest_movement" ) == 0 )
			rval = simmgr_shm->status.respiration.chest_movement;
	}
	else if ( strcmp(param_class, "general" ) == 0 )
	{
		if ( strcmp(param_element, "temperature" ) == 0 )
			rval = simmgr_shm->status.general.temperature;
	}
	return ( rval );
}
