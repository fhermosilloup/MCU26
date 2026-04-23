#ifndef __NMEA_H
#define __NMEA_H
	/* Includes --------------------------------------------*/
	#include <stdint.h>
	#include <stdbool.h>

	/* Defines --------------------------------------------*/
	#define NMEA_MAX_FIELDS 20

	/* Structs --------------------------------------------*/
	typedef struct {
		int32_t value;
		int32_t scale;
	} nmea_float_t;

	typedef struct {
		int hours;
		int minutes;
		int seconds;
	} nmea_time_t;

	typedef struct {
		nmea_time_t time;
		nmea_float_t lat;
		nmea_float_t lon;
		bool valid;
	} nmea_rmc_t;

	typedef struct {
		    nmea_time_t time;
		    nmea_float_t lat;
		    nmea_float_t lon;
		    int fix_quality;
		    int satellites;
		    nmea_float_t hdop;
		    nmea_float_t altitude;
		} nmea_gga_t;

		typedef struct {
		    int mode;        // A/M
		    int fix_type;    // 1,2,3
		    int sats[12];    // PRNs
		    nmea_float_t pdop;
		    nmea_float_t hdop;
		    nmea_float_t vdop;
		} nmea_gsa_t;

		typedef struct {
		    int prn;
		    int elevation;
		    int azimuth;
		    nmea_float_t snr;
		} nmea_sat_t;

		typedef struct {
		    int total_msgs;
		    int msg_num;
		    int total_sats;
		    nmea_sat_t sats[4];   // max 4 por mensaje
		} nmea_gsv_t;

	/* Enums --------------------------------------------*/
	typedef enum {
		NMEA_UNKNOWN = 0,
		NMEA_RMC,
		NMEA_GGA,
		NMEA_GSA,
		NMEA_GSV
	} nmea_sentence_t;

	/* Prototype function ---------------------------------*/
	nmea_sentence_t nmea_get_type(const char *s);

	bool nmea_parse_rmc(nmea_rmc_t *out, char *sentence);
	bool nmea_parse_gga(nmea_gga_t *out, char *sentence);
	bool nmea_parse_gsa(nmea_gsa_t *out, char *sentence);
	bool nmea_parse_gsv(nmea_gsv_t *out, char *sentence);

	float nmea_tocoord(nmea_float_t *f);

#endif /* __NMEA_H */
