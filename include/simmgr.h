#ifndef _SIMMGR_H
#define _SIMMGR_H


// Defines
//
#define SIMMGR_VERSION		1
#define STR_SIZE			64
#define SIMMGR_SHM_NAME		"simmgr"
#define LOG_TO_FILE			0

// When initSHM is called, only the simmgr daemon should set OPEN_WITH_CREATE
// All other open with _OPEN_ACCESS
#define	OPEN_WITH_CREATE	1
#define OPEN_ACCESS			0


// Data Structures
//
struct cardiac
{
	char rhythm[STR_SIZE];
	int rate;	// Heart Rate in Beats per Minute
	char pwave[STR_SIZE];
	int pr_interval;	// PR interval in msec
	int qrs_interval;		// QRS in msec
	int bps_sys;	// Systolic
	int bps_dia;	// Diastolic
};
struct scenario
{
	char active[STR_SIZE];	// Name of active scenario
	char start[STR_SIZE];		// Date/Time scenario started
	char start_msec;			// msec time at start of scenarion
	
	// We should add additional elements to show where in the scenario we are currently executing
};

struct respiration
{
	// Sounds for Inhalation, Exhalation and Background
	char left_sound_in[STR_SIZE];
	char left_sound_out[STR_SIZE];
	char left_sound_back[STR_SIZE];
	char right_sound_in[STR_SIZE];
	char right_sound_out[STR_SIZE];
	char right_sound_back[STR_SIZE];
	
	// Duration of in/ex
	int inhalation_duration;	// in msec
	int exhalation_duration;	// in msec
	
	int rate;					// Breaths per minute
};

struct auscultation
{
	int side;	// 0 - None, 1 - L:eft, 2 - Right
	int row;	// Row 0 is closest to spine
	int col;	// Col 0 is closets to head
};
struct pulse
{
	int position;	// Position code of active pulse check
};
struct cpr
{
	int last;			// msec time of last compression
	int	compression;	// 0 to 100%
	int release;		// 0 to 100%
};
struct defibrulation
{
	int last;			// msec time of last shock
	int energy;			// Energy in Joules of last shock
};

struct hdr
{
	int	version;
	int size;
};
struct server
{
	char name[STR_SIZE]; 		// SimCtl Hostname
	char ip_addr[STR_SIZE];		// Public Network IP Address
	char server_time[STR_SIZE];	// Linux date/timestamp
	int msec_time;				// msec timer.
};
struct status
{
	struct cardiac			cardiac;
	struct scenario 		scenario;
	struct respiration		respiration;
	
	// Status of sensed actions
	struct auscultation		auscultation;
	struct pulse			pulse;
	struct cpr				cpr;
	struct defibrulation	defibrulation;
};
struct instructor
{
	struct cardiac		cardiac;
	struct scenario 	scenario;
	struct respiration	respiration;
};
	
// Data Structure of Shared memory file
struct simmgr_shm
{
	// Static data describing the data simmgr_shm structure for verification.
	struct hdr hdr;
	
	// Dynamic data relating to the Server System
	struct server server;
	
	// SimMgr Status - Written by the SimMgr only. Read from all others
	struct status status;
	
	// Commands from Instructor Interface. Written by the II Ajax calls. Cleared by SimMgr when processed.
	struct instructor instructor;

};

// Prototypes
// 
int	initSHM(int create );
void log_message(const char *filename, const char* message);
void daemonize(void );
int kbhit(void);
int checkExit(void );
char *nth_occurrence(char *haystack, char *needle, int nth);
char *do_command_read(const char *cmd_str, char *buffer, int max_len );
void get_date(char *buffer );
char *getETH0_IP();
char *itoa(int val, char *buf, int radix );

// Global Data
//
#ifndef SIMUTIL
extern int simmgrSHM;					// The File for shared memory
extern struct simmgr_shm *simmgr_shm;	// Data structure of the shared memory
#endif

// Used for deamonize only
extern char *program_invocation_name;
extern char *program_invocation_short_name;

#endif // _SIMMGR_H