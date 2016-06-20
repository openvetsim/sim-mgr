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