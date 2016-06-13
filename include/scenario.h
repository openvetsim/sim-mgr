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

#define PARAMETER_NAME_LENGTH 128
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
	char 	parameter[PARAMETER_NAME_LENGTH];	// This is the full ascii name of the parameter, including path. eg: cardiac:rate
	int		test;
	int		value;		// Comaprison value
	int		value2;		// Comaprison value (only for Inside/Outside)
	int 	scene;		// ID of next scene
};

	
// For Parsing the XML:

struct xml_level
{
	int num;
	char name[PARAMETER_NAME_LENGTH];
};

#endif // _SCENARIO_H