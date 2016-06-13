/*
 * scenario.cpp
 *
 * Scenario Processing
 *
 * Copyright (C) 2016 Terence Kelleher. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <libxml2/libxml/xmlreader.h>
#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xpathInternals.h>

#include "../include/scenario.h"

#ifdef LIBXML_READER_ENABLED


struct xml_level xmlLevels[10];
int xml_current_level = 0;
int line_number = 0;
const char *xml_filename;
int verbose = 1;

struct scenario_data *scenario;
struct scenario_scene *new_scene;
struct scenario_trigger *new_trigger;

/**
 * cleanString
 *
 * remove leading and trailing spaces. Reduce internal spaces to single. Remove tabs, newlines, CRs.
*/
#define WHERE_START		0
#define WHERE_TEXT		1
#define WHERE_SPACE		2

void
cleanString(const xmlChar *strIn )
{
	char *in;
	char *out;
	int where = WHERE_START;
	
	in = (char *)strIn;
	out = (char *)strIn;
	while ( *in )
	{
		if ( isspace( *in ) )
		{
			switch ( where )
			{
				case WHERE_START:
				case WHERE_SPACE:
					break;
				
				case WHERE_TEXT:
					*out = ' ';
					out++;
					where = WHERE_SPACE;
					break;
			}
		}
		else
		{
			where = WHERE_TEXT;
			*out = *in;
			out++;
		}
		in++;
	}
	*out = 0;
}

#define PARSE_STATE_NONE	0
#define PARSE_STATE_INIT	1
#define PARSE_STATE_SCENE	2

#define PARSE_INIT_STATE_NONE			0
#define PARSE_INIT_STATE_CARDIAC		1
#define PARSE_INIT_STATE_RESPIRATION	2
#define PARSE_INIT_STATE_GENERAL		3
#define PARSE_INIT_STATE_SCENE			4

#define PARSE_SCENE_STATE_NONE				0
#define PARSE_SCENE_STATE_INIT				1
#define PARSE_SCENE_STATE_INIT_CARDIAC		2
#define PARSE_SCENE_STATE_INIT_RESPIRATION	3
#define PARSE_SCENE_STATE_INIT_GENERAL		4
#define PARSE_SCENE_STATE_TRIGS				5
#define PARSE_SCENE_STATE_TRIG				6

int parse_state = PARSE_STATE_NONE;
int parse_init_state = PARSE_INIT_STATE_NONE;
int parse_scene_state = PARSE_SCENE_STATE_NONE;

/**
* initializeParameterStruct
* &initParams: Pointer to a "struct instructor"
*
* Initialize the paraneters structure to be "no set" on all parameters.
* For strings, they are null. For numerics, they are -1.
*/
static void
initializeParameterStruct(struct instructor *initParams )
{
	memset(initParams, 0, sizeof(struct instructor) );
	
	initParams->cardiac.vpc_freq = -1;
	initParams->cardiac.pea = -1;
	initParams->cardiac.rate = -1;
	initParams->cardiac.nibp_rate = -1;
	initParams->cardiac.transfer_time = -1;
	initParams->cardiac.pr_interval = -1;
	initParams->cardiac.qrs_interval = -1;
	initParams->cardiac.bps_dia = -1;
	
	initParams->respiration.inhalation_duration = -1;
	initParams->respiration.exhalation_duration = -1;
	initParams->respiration.spo2 = -1;
	initParams->respiration.rate = -1;
	initParams->respiration.transfer_time = -1;
	
	initParams->general.temperature = -1;
	initParams->general.transfer_time = -1;
}
/**
* processInit
* &initParams: Pointer to a "struct instructor"
*
* Transfer the instructions from the initParams to the instructor portion of
* the shared data space, to activate all the controls.
*/
static void
processInit(struct instructor *initParams  )
{
	
}
/**
 * saveData:
 * @xmlName: name of the entry
 * @xmlValue: Text data to convert and save in structure
 *
 * Take the current Text entry and save as appropriate in the currently
 * named data structure
*/
static void
saveData(const xmlChar *xmlName, const xmlChar *xmlValue )
{
	char *name = (char *)xmlName;
	char *value = (char *)xmlValue;
	
	switch ( parse_state )
	{
		case PARSE_STATE_NONE:
			if ( verbose )
			{
				printf("NONE: Name is %s, Value, %s\n", name, value );
			}
			break;
		case PARSE_STATE_INIT:
			switch ( parse_init_state )
			{
				case PARSE_INIT_STATE_NONE:
					if ( verbose )
					{
						printf("INIT_NONE: Name is %s, Value, %s\n", name, value );
					}
					break;
				case PARSE_INIT_STATE_CARDIAC:
					break;
					
			}
			break;
			
		case PARSE_STATE_SCENE:
			break;
		
		default:
			if ( verbose && strcmp(xmlLevels[1].name, "header" ) == 0 )
			{
				if ( xml_current_level == 2 )
				{
					name = xmlLevels[2].name;
					if ( strcmp(name, "author" ) == 0 )
					{
						sprintf(scenario->author, "%s", value );
						if ( verbose )
						{
							printf("Author: %s\n", scenario->author );
						}
					}
					else if ( strcmp(name, "title" ) == 0 )
					{
						sprintf(scenario->title, "%s", value );
						if ( verbose )
						{
							printf("Title: %s\n", scenario->title );
						}
					}
					else if ( strcmp(name, "date_of_creation" ) == 0 )
					{
						sprintf(scenario->date_created, "%s", value );
						if ( verbose )
						{
							printf("Created: %s\n", scenario->date_created );
						}
					}
					else if ( strcmp(name, "description" ) == 0 )
					{
						sprintf(scenario->description, "%s", value );
						if ( verbose )
						{
							printf("Description: %s\n", scenario->description );
						}
					}
					else
					{
						if ( verbose)
						{
							printf("In Header, Unhandled: Name is %s, Value, %s\n", name, value );
						}
					}
				}
				else
				{
					if ( verbose )
					{
						printf("In Header, Level is %d, Name is %s, Value, %s\n", xml_current_level, name, value );
					}
				}
			}
			break;
	}
}
static void
insert_llist(struct snode *entry, struct snode *list )
{	
	int limit = 100;
	
	// Scan to the first NULL entry (end of list)
	while ( list->next )
	{
		list = list->next;
		if ( limit-- == 0 )
		{
			fprintf(stderr, "insert_llist: Limit exceeded\n" );
			exit ( -1 );
		}
	}
	list->next = entry;
}


static void
startParseState(int lvl, char *name )
{
	switch ( lvl )
	{
		case 0:		// Top level - no actions
			break;
			
		case 1:	// header, profile, media, events have no action
			if ( strcmp(name, "init" ) == 0 )
			{
				parse_state = PARSE_STATE_INIT;
			}
			else if ( strcmp(name, "scene" ) == 0 )
			{
				// Allocate a scene
				new_scene =  (struct scenario_scene *)calloc(1, sizeof(struct scenario_scene ) );
				initializeParameterStruct(&new_scene->initParams );
				insert_llist(&new_scene->scene_list, &scenario->scene_list );
				parse_state = PARSE_STATE_SCENE;
			}
			break;
		case 2:	// 
			switch ( parse_state )
			{
				case PARSE_STATE_INIT:
					if ( strcmp(name, "cardiac" ) == 0 )
					{
						parse_init_state = PARSE_INIT_STATE_CARDIAC;
					}
					else if ( strcmp(name, "respiration" ) == 0 )
					{
						parse_init_state = PARSE_INIT_STATE_RESPIRATION;
					}
					else if ( strcmp(name, "general" ) == 0 )
					{
						parse_init_state = PARSE_INIT_STATE_GENERAL;
					}
					else if ( strcmp(name, "scene" ) == 0 )
					{
						parse_init_state = PARSE_INIT_STATE_SCENE;
					}
					else
					{
						parse_init_state = PARSE_INIT_STATE_NONE;
					}
					break;
					
				case PARSE_STATE_SCENE:
					if ( strcmp(name, "init" ) == 0 )
					{
						parse_scene_state = PARSE_SCENE_STATE_INIT;
					}
					else if ( strcmp(name, "triggers" ) == 0 )
					{
						parse_scene_state = PARSE_SCENE_STATE_TRIGS;
					}
					else
					{
						parse_init_state = PARSE_INIT_STATE_NONE;
					}
					break;
					
				default:
					break;
			}
			break;
			
		case 3:
			if ( ( parse_scene_state == PARSE_SCENE_STATE_TRIGS ) &&
				 ( strcmp(name, "trigger" ) == 0 ) )
			{
				new_trigger = (struct scenario_trigger *)calloc(1, sizeof(struct scenario_trigger ) );
				insert_llist(&new_trigger->trigger_list, &new_scene->trigger_list );
				parse_scene_state = PARSE_SCENE_STATE_TRIG;
			}
			break;
			
		default:
			break;
	}
}
static void
endParseState(int lvl )
{
	switch ( lvl )
	{
		case 0:	// Parsing Complete
			// Send Init Parameters to SIMMGR
			processInit(&scenario->initParams );
			break;
			
		case 1:	// Section End
			parse_state = PARSE_STATE_NONE;
			break;
			
		case 2:
			switch ( parse_state )
			{
				case PARSE_STATE_SCENE:
					parse_scene_state = PARSE_SCENE_STATE_NONE;
					break;
					
				case PARSE_STATE_INIT:
					parse_scene_state = PARSE_INIT_STATE_NONE;
					break;
			}
			break;
		case 3:
			if ( ( parse_scene_state == PARSE_SCENE_STATE_TRIG ) &&
				 ( new_trigger ) )
			{
				new_trigger = (struct scenario_trigger *)0;
				parse_scene_state = PARSE_SCENE_STATE_TRIGS;
			}
			break;
			
		default:
			break;
	}
}
/**
 * processNode:
 * @reader: the xmlReader
 *
 * Dump information about the current node
 */
static void
processNode(xmlTextReaderPtr reader)
{
    const xmlChar *name;
	const xmlChar *value;
	int lvl;
	
    name = xmlTextReaderConstName(reader);
    if ( name == NULL )
	{
		name = BAD_CAST "--";
	}
	line_number = xmlTextReaderGetParserLineNumber(reader);
	
    value = xmlTextReaderConstValue(reader);
	// Node Type Definitions in http://www.gnu.org/software/dotgnu/pnetlib-doc/System/Xml/XmlNodeType.html
	
	switch ( xmlTextReaderNodeType(reader) )
	{
		case 1: // Element
			xml_current_level = xmlTextReaderDepth(reader);
			xmlLevels[xml_current_level].num = xml_current_level;
			if ( xmlStrlen(name) >= PARAMETER_NAME_LENGTH )
			{
				fprintf(stderr, "XML Parse Error: %s[%d]: Name %s exceeds Max Length of %d\n",
					xml_filename, line_number, name, PARAMETER_NAME_LENGTH-1 );
					
			}
			else
			{
				sprintf(xmlLevels[xml_current_level].name, "%s", name );
			}
			// printf("Start %d %s\n", xml_current_level, name );
			startParseState(xml_current_level, (char *)name );
			break;
		
		case 3:	// Text
			cleanString(value );
			saveData(name, value );
			if ( verbose )
			{
				for ( lvl = 0 ; lvl <= xml_current_level ; lvl++ )
				{
					printf("[%d]%s:", lvl, xmlLevels[lvl].name );
				}
				printf(" %s\n", value );
			}
			break;
			
		case 13: // Whitespace
		case 14: // Significant Whitespace 
		case 8: // Comment
			break;
		case 15: // End Element
			// printf("End %d\n", xml_current_level );
			endParseState(xml_current_level );
			xml_current_level--;
			break;
		
			
		case 0: // None
		case 2: // Attribute
		case 4: // CDATA
		case 5: // EntityReference
		case 6: // Entity
		case 7: // ProcessingInstruction
		case 9: // Document
		case 10: // DocumentType
		case 11: // DocumentTragment
		case 12: // Notation
		case 16: // EndEntity
		case 17: // XmlDeclaration
		
		default:
			printf("%d %d %s %d %d", 
				xmlTextReaderDepth(reader),
				xmlTextReaderNodeType(reader),
				name,
				xmlTextReaderIsEmptyElement(reader),
				xmlTextReaderHasValue(reader));
			if (value == NULL)
			{
				printf("\n");
			}
			else
			{
				if (xmlStrlen(value) > 60)
				{
					printf(" %.80s...\n", value);
				}
				else
				{
					printf(" %s\n", value);
				}
			}
	}
}

/**
 * readScenario:
 * @filename: the file name to parse
 *
 * Parse the scenario
 */
 
static void
readScenario(const char *filename)
{
    xmlTextReaderPtr reader;
    int ret;

    xmlLineNumbersDefault(1);
    
	reader = xmlReaderForFile(filename, NULL, 0);
	if ( reader != NULL )
	{
		while ( ( ret = xmlTextReaderRead(reader) ) == 1 )
		{
            processNode(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0)
		{
            fprintf(stderr, "%s : failed to parse\n", filename);
        }
    } 
	else
	{
        fprintf(stderr, "Unable to open %s\n", filename);
    }
}
/*
static void
processChild1(xmlNode * a_node, int depth )
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
        if (cur_node->type == XML_ELEMENT_NODE)
		{
            printf("node type: Element, depth %d, name: %s\n", depth, cur_node->name);
        }
		else
		{
			printf("node type: %d, depth %d, name: %s\n", cur_node->type, depth, cur_node->name);
		}
        processChild1(cur_node->children, depth+1 );
    }
}
*/
/**
 * processDoc:
 * @a_node: the initial xml node to consider.
 *
 * Process the XML document
 */
/*
static void
processDoc(xmlNode * a_node, int depth )
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
        if (cur_node->type == XML_ELEMENT_NODE)
		{
            printf("node type: Element, depth %d, name: %s\n", depth, cur_node->name);
        }
		else
		{
			printf("node type: %d, depth %d, name: %s\n", cur_node->type, depth, cur_node->name);
		}
        processChild1(cur_node->children, depth+1 );
    }
}
*/
/**
 * parser:
 * @filename: a filename or an URL
 *
 * Parse the resource and free the resulting tree
 */
/*
static void
parser(const char *filename)
{
    xmlDocPtr doc = NULL; // the resulting document tree 
	xmlNode *root_element = NULL;
	
    doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL) 
	{
        fprintf(stderr, "Failed to parse %s\n", filename);
		return;
    }
	else
	{
		// Get the root element node
		root_element = xmlDocGetRootElement(doc);

		processDoc(root_element, 0);
	}
    xmlFreeDoc(doc);
}
*/
char usage[] = "[xml file name]";

int 
main(int argc, char **argv)
{
	if ( argc != 2 ) 
	{
		printf("%s: %s\n", argv[0], usage );
        return(1);
	}
	
	// Allocate and clear the base scenario structure
	scenario = (struct scenario_data *)calloc(1, sizeof(struct scenario_data) );
	initializeParameterStruct(&scenario->initParams );
				
    // Initialize the library and check potential ABI mismatches 
    LIBXML_TEST_VERSION

    readScenario(argv[1]);
	// parser(argv[1]);
	
    xmlCleanupParser();
	
    // this is to debug memory for regression tests
    xmlMemoryDump();
	
	// Continue scenario execution
	
	
    return(0);
}


#else
int main(void) {
    fprintf(stderr, "XInclude support not compiled in\n");
    exit(1);
}
#endif