#include "minmea.h"


/* Private function prototype -------------------------------*/
// Checksum
static uint8_t hex2int(char c);
static uint8_t nmea_checksum(const char *s);
static bool nmea_check(const char *s);
// Tokenizer
static int nmea_split(char *sentence, char *fields[], int max_fields);
// String
static nmea_float_t nmea_parse_float(const char *s);
static int nmea_parse_int(const char *s);
static void parse_time(nmea_time_t *t, const char *s);

/* Private function reference -------------------------------*/
uint8_t nmea_checksum(const char *s)
{
    uint8_t chk = 0;

    if (*s == '$') s++;

    while (*s && *s != '*') {
        chk ^= (uint8_t)(*s);
        s++;
    }

    return chk;
}

static uint8_t hex2int(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

bool nmea_check(const char *s)
{
    const char *star = s;

    while (*star && *star != '*') star++;
    if (!*star) return false;

    uint8_t given =
        (hex2int(star[1]) << 4) |
        (hex2int(star[2]));

    return nmea_checksum(s) == given;
}


int nmea_split(char *sentence, char *fields[], int max_fields)
{
    int count = 0;

    if (*sentence == '$') sentence++;

    fields[count++] = sentence;

    while (*sentence && count < max_fields) {
        if (*sentence == ',' || *sentence == '*') {
            *sentence = '\0';
            fields[count++] = sentence + 1;
        }
        sentence++;
    }

    return count;
}


int nmea_parse_int(const char *s)
{
    int val = 0;
    int sign = 1;

    if (*s == '-') {
        sign = -1;
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (*s - '0');
        s++;
    }

    return val * sign;
}

nmea_float_t nmea_parse_float(const char *s)
{
    nmea_float_t f = {0, 1};
    int sign = 1;

    if (*s == '-') {
        sign = -1;
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        f.value = f.value * 10 + (*s - '0');
        s++;
    }

    if (*s == '.') {
        s++;
        while (*s >= '0' && *s <= '9') {
            f.value = f.value * 10 + (*s - '0');
            f.scale *= 10;
            s++;
        }
    }

    f.value *= sign;
    return f;
}

static void parse_time(nmea_time_t *t, const char *s)
{
    if (!s || !*s) return;

    int val = nmea_parse_int(s);

    t->hours   = val / 10000;
    t->minutes = (val / 100) % 100;
    t->seconds = val % 100;
}



/* Public function reference -------------------------------*/
nmea_sentence_t nmea_get_type(const char *s)
{
    if (!s || s[0] != '$')
        return NMEA_UNKNOWN;

    // Talker
    if (s[1]!='G' || s[2]!='P') return NMEA_UNKNOWN;

    // Ignora talker (GP, GN, GL, etc.)
    char a = s[3];
    char b = s[4];
    char c = s[5];

    if(a=='G')
    {
    	if (b == 'G' && c == 'A')
    	        return NMEA_GGA;
    	else if (b == 'S')
    	{
    		if(c=='A') return NMEA_GSA;
    		else if (c == 'V') return NMEA_GSV;
    	}
    }
    else if(a=='R')
    {
    	if (b == 'M' && c == 'C') return NMEA_RMC;
    }

    return NMEA_UNKNOWN;
}

bool nmea_parse_rmc(nmea_rmc_t *out, char *sentence)
{
    if (!nmea_check(sentence))
        return false;

    char *fields[NMEA_MAX_FIELDS];
    int n = nmea_split(sentence, fields, NMEA_MAX_FIELDS);

    if (n < 10)
        return false;

    // fields[0] = GPRMC
    parse_time(&out->time, fields[1]);

    out->valid = (fields[2][0] == 'A');

    out->lat = nmea_parse_float(fields[3]);
    out->lon = nmea_parse_float(fields[5]);

    return true;
}

bool nmea_parse_gga(nmea_gga_t *out, char *sentence)
{
    if (!nmea_check(sentence))
        return false;

    char *f[NMEA_MAX_FIELDS];
    int n = nmea_split(sentence, f, NMEA_MAX_FIELDS);

    if (n < 10)
        return false;

    parse_time(&out->time, f[1]);

    out->lat = nmea_parse_float(f[2]);
    if (f[3][0] == 'S') out->lat.value *= -1;

    out->lon = nmea_parse_float(f[4]);
    if (f[5][0] == 'W') out->lon.value *= -1;

    out->fix_quality = nmea_parse_int(f[6]);
    out->satellites  = nmea_parse_int(f[7]);

    out->hdop = nmea_parse_float(f[8]);
    out->altitude = nmea_parse_float(f[9]);

    return true;
}

bool nmea_parse_gsa(nmea_gsa_t *out, char *sentence)
{
    if (!nmea_check(sentence))
        return false;

    char *f[NMEA_MAX_FIELDS];
    int n = nmea_split(sentence, f, NMEA_MAX_FIELDS);

    if (n < 17)
        return false;

    out->mode = f[1][0];
    out->fix_type = nmea_parse_int(f[2]);

    for (int i = 0; i < 12; i++) {
        out->sats[i] = nmea_parse_int(f[3 + i]);
    }

    out->pdop = nmea_parse_float(f[15]);
    out->hdop = nmea_parse_float(f[16]);
    out->vdop = nmea_parse_float(f[17]);

    return true;
}

bool nmea_parse_gsv(nmea_gsv_t *out, char *sentence)
{
    if (!nmea_check(sentence))
        return false;

    char *f[NMEA_MAX_FIELDS];
    int n = nmea_split(sentence, f, NMEA_MAX_FIELDS);

    if (n < 4)
        return false;

    out->total_msgs = nmea_parse_int(f[1]);
    out->msg_num    = nmea_parse_int(f[2]);
    out->total_sats = nmea_parse_int(f[3]);

    int idx = 4;
    int sat_idx = 0;

    while (idx + 3 < n && sat_idx < 4) {
        out->sats[sat_idx].prn = nmea_parse_int(f[idx]);
        out->sats[sat_idx].elevation = nmea_parse_int(f[idx+1]);
        out->sats[sat_idx].azimuth   = nmea_parse_int(f[idx+2]);
        out->sats[sat_idx].snr       = nmea_parse_float(f[idx+3]);

        idx += 4;
        sat_idx++;
    }

    return true;
}


float nmea_tocoord(nmea_float_t *f)
{
    int deg = f->value / (f->scale * 100);
    int min = f->value % (f->scale * 100);

    return deg + (float)min / (60.0f * f->scale);
}
