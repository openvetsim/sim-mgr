/*
 * scenario.h
 *
 * Scenario Processing
 *
 * Copyright (C) 2016 Terence Kelleher. All rights reserved.
 *
 */

#ifndef _SCENARIO_H
#define _SCENARIO_H

#include "../include/llist.h"
#include "../include/simmgr.h"

#define LONG_STRING_SIZE	128
#define NORMAL_STRING_SIZE	32

#define PARSE_STATE_NONE	0
#define PARSE_STATE_INIT	1
#define PARSE_STATE_SCENE	2

#define PARSE_INIT_STATE_NONE			0
#define PARSE_INIT_STATE_CARDIAC		1
#define PARSE_INIT_STATE_RESPIRATION	2
#define PARSE_INIT_STATE_GENERAL		3
#define PARSE_INIT_STATE_SCENE			4
#define PARSE_INIT_STATE_VOCALS			5

#define PARSE_SCENE_STATE_NONE				0
#define PARSE_SCENE_STATE_INIT				1
#define PARSE_SCENE_STATE_INIT_CARDIAC		2
#define PARSE_SCENE_STATE_INIT_RESPIRATION	3
#define PARSE_SCENE_STATE_INIT_GENERAL		4
#define PARSE_SCENE_STATE_INIT_VOCALS		5
#define PARSE_SCENE_STATE_TIMEOUT			6
#define PARSE_SCENE_STATE_TRIGS				7
#define PARSE_SCENE_STATE_TRIG				8

// Scenario
struct scenario_data
{
	char author[LONG_STRING_SIZE];
	char title[LONG_STRING_SIZE];
	char date_created[NORMAL_STRING_SIZE];
	char description[LONG_STRING_SIZE];
	
	// Initialization Parameters for the scenario
	struct instructor initParams;
	
	struct snode scene_list;
};

struct scenario_scene
{
	struct snode scene_list;
	int id;				// numeric ID - 1 is always the entry scene, 2 is always the end scene
	char name[32];		// 
	
	// Initialization Parameters for the scene
	struct instructor initParams;
	
	// Timeout in Seconds
	int timeout;
	int timeout_scene;
	
	struct snode trigger_list;
};

	
// A trigger is defined as a setting of a parameter, or the setting of a trend. A trend time of 0 indicates immediate.

#define TRIGGER_NAME_LENGTH 	32
#define TRIGGER_TEST_EQ			0
#define TRIGGER_TEST_LTE		1
#define TRIGGER_TEST_LT			2
#define TRIGGER_TEST_GTE		3
#define TRIGGER_TEST_GT			4
#define TRIGGER_TEST_INSIDE		5
#define TRIGGER_TEST_OUTSIDE	6

struct scenario_trigger
{
	struct	snode trigger_list;
	char 	param_class[TRIGGER_NAME_LENGTH];	// Class eg: cardiac, resipration, ...
	char 	param_element[TRIGGER_NAME_LENGTH];	// Parameter eg: rate, transfer_time, ...
	int		test;
	int		value;		// Comaprison value
	int		value2;		// Comaprison value (only for Inside/Outside)
	int 	scene;		// ID of next scene
};

	
// For Parsing the XML:
#define PARAMETER_NAME_LENGTH	128
struct xml_level
{
	int num;
	char name[PARAMETER_NAME_LENGTH];
};

#endif // _SCENARIO_H