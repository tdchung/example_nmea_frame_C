/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

  tdchung
  tdchung.9@gmail.com
  xx-Jun-2019

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef bool
#define bool   int
#define true   1
#define false  0
#endif

#define MAX_NMEA_FRAME_STRING 1023
#define MAX_NMEA_FRAME_SIZE   MAX_NMEA_FRAME_STRING + 1
#define MAX_POISITION_LENGHT  64

#define LOG_DEBUG 1

// define print info message
#define PRINT_MESSAGE(Msg, ...) { printf("INFO. " Msg, ##__VA_ARGS__); }
#define PRINT_MESSAGE_ERROR(Msg, ...) { printf("ERROR. " Msg, ##__VA_ARGS__); }

#ifdef  LOG_DEBUG
#define PRINT_MESSAGE_DEBUG(Msg, ...) { printf("DEBUG. " Msg, ##__VA_ARGS__); }
#else
#define PRINT_MESSAGE_DEBUG(Msg, ...) {}
#endif

// test gnss frame
char *gnss_nmea = "$GPDTM,999,,0.08,N,0.07,E,-47.7,W84*1C\r\n"
  "$EIGAQ,RMC*2B\r\n"
  "$EIGBQ,RMC*28\r\n"
  "$GPGBS,235458.00,1.4,1.3,3.1,03,,-21.4,3.8,1,0*5B\r\n"
  "$GPGGA,092725.00,4717.11399,N,00833.91590,E,1,08,1.01,499.6,M,48.0,M,,*5B\r\n"
  "$GPGLL,4717.11364,N,00833.91565,E,092321.00,A,A*60\r\n"
  "$GPGNS,091547.00,5114.50897,N,00012.28663,W,AA,10,0.83,111.1,45.6,,,V*71\r\n";


// strAppendCharater, NOTE modify buff
// TODO: bug if lenght out of range.
void strAppendCharater(char *buff, char insert, int pos)
{
    char temp[MAX_NMEA_FRAME_SIZE] = {0};
    PRINT_MESSAGE_DEBUG ("BUFF before: %s\n", buff);
    strncpy(temp, buff, pos); // copy at most first pos characters
    PRINT_MESSAGE_DEBUG ("temp 1: %s\n", temp);
    *(temp+pos) = insert;
    PRINT_MESSAGE_DEBUG ("temp 2: %s\n", temp);
    strcpy((temp+pos+1), buff+pos); 
    PRINT_MESSAGE_DEBUG ("temp 3: %s\n", temp);
    strcpy(buff, temp);   // copy it back to subject
    PRINT_MESSAGE_DEBUG ("BUFF after : %s\n", buff);
}


// update nmea buffer
// TODO: bug if lenght out of range.
void updateNmeaBuff(char *buff)
{
    int len = strlen(buff);
    // PRINT_MESSAGE_DEBUG("Lenght: %d\n",len);
    for(int i=0; i<len-1; i++)
    {
        if (',' == *(buff+i) && (',' == *(buff+i+1)))
        {
            //  PRINT_MESSAGE_DEBUG("i : %d\n",i);
            strAppendCharater(buff, '#', i+1);
        }
    }
}


static bool getNmeaFrame (char *buff, char *nmea, char *output, size_t lenght)
{
    char *substr = NULL;
    char *token = NULL;
    char *token_ptr = NULL;
#ifdef __cplusplus
    char *nmea_buff = (char*)malloc(MAX_NMEA_FRAME_SIZE);
#else
    char *nmea_buff = malloc(MAX_NMEA_FRAME_SIZE);
#endif

    memset(nmea_buff, 0, MAX_NMEA_FRAME_SIZE);

    strncpy(nmea_buff, buff, MAX_NMEA_FRAME_STRING);
    substr = strstr (nmea_buff, nmea);
    if (NULL == substr)
    {
        PRINT_MESSAGE_ERROR ("No sub string\n");
        goto getNmeaFrame_False;
    }
    else
    // nmea is available
    {
        PRINT_MESSAGE_DEBUG ("substr = : %s\n", substr);
    }

    // now start get line
    token = strtok_r(substr, "\r", &token_ptr);
    PRINT_MESSAGE_DEBUG("token = : %s\n", token);
    PRINT_MESSAGE_DEBUG("token_ptr = : %s\n", token_ptr);
    if (NULL == token)
    {
        PRINT_MESSAGE_ERROR("get token error\n");
        goto getNmeaFrame_False;
    }

    // got data
    strncpy(output, token, lenght);

    free(nmea_buff);
    return true;
getNmeaFrame_False:
    free(nmea_buff);
    return false;
}

static bool getNmeaField(const char *nmea, int indexField, char *valueOut)
{
    char *token = NULL;
    char *token_ptr = NULL;
#ifdef __cplusplus
    char* nmea_buff = (char*)malloc(MAX_NMEA_FRAME_SIZE);
#else
    char* nmea_buff = malloc(MAX_NMEA_FRAME_SIZE);
#endif
    memset(nmea_buff, 0, MAX_NMEA_FRAME_SIZE);
    strncpy(nmea_buff, nmea, MAX_NMEA_FRAME_SIZE);

    // // now start get line
    // if (0 == indexField)
    // {
    //     if ( NULL != (token = strtok_r(nmea_buff, ",", &token_ptr)))
    //     {
    //         // got data
    //         strncpy(valueOut, token, MAX_POISITION_LENGHT);
    //         goto getNmeaField_Passed;
    //     }
    //     else
    //         goto getNmeaField_Failed;
    // }
    
    token_ptr = nmea_buff;
    // else
    // {
    for (int i=0; i<indexField+1; i++)
    {
        if ( NULL != (token = strtok_r(token_ptr, ",", &token_ptr)))
        {
            if (indexField == i)
            {
                // got data
                strncpy(valueOut, token, MAX_POISITION_LENGHT);
                goto getNmeaField_Passed;
            }
        }
        else
            goto getNmeaField_Failed;
    }
    // }

getNmeaField_Passed:
    free(nmea_buff);
    return true;
getNmeaField_Failed:
    free(nmea_buff);
    PRINT_MESSAGE_ERROR ("Something failed in getNmeaField\n");
    return false;
}


// for correction
// 4717.11399N, 00833.91590E --> 47.2852332°, 008.5652650°
// N, E --> positive
// S, W --> negative
static bool dddmm2Degrees(char *dddmmIn, char* degreesOut)
{
    // 1. get degrees. = dddmmIn.slip('.')[0]/100
    // 2. get minutes. = (dddmmIn.slip('.')[0]%100 + 0.dddmmIn.slip('.')[1])
    // 3. decimal = degrees + minutes/60
    // 4. convert to string and return

    char* str_ptr = NULL;
    double position = 0.0;
#ifdef __cplusplus
    char* degrees = (char*)malloc(MAX_POISITION_LENGHT);
#else
    char* degrees = malloc(MAX_POISITION_LENGHT);
#endif
    memset(degrees, 0, MAX_POISITION_LENGHT);


    if (NULL == degreesOut)
    {
        PRINT_MESSAGE_ERROR ("pointer out is NULL\n");
        goto dddmm2Degrees_Failed;
    }

    position = strtod(dddmmIn, &str_ptr);
    if ((0.0 == position) && (str_ptr == dddmmIn))
    {
        PRINT_MESSAGE_ERROR ("Data format not correct\n");
        goto dddmm2Degrees_Failed;
    }

    PRINT_MESSAGE_DEBUG ("pointer: %p\n", str_ptr);
    PRINT_MESSAGE_DEBUG ("pointer: %p\n", dddmmIn);
    PRINT_MESSAGE_DEBUG ("position: %4.8lf\n", position);

    position = (int) position/100 + (position - (int) position/100*100)/60;
    PRINT_MESSAGE_DEBUG ("degrees: %4.8lf\n", position);

    // not check pointer out lenght
    snprintf(degrees, MAX_POISITION_LENGHT, "%.8lf", position);
    PRINT_MESSAGE_DEBUG ("degrees: %s\n", degrees);

    // TODO: add argument lenght. use strncpy
    strcpy(degreesOut, degrees);

    free(degrees);
    return true;

dddmm2Degrees_Failed:
    free(degrees);
    return false;
}

static bool getPosition(char *nmea, char *lat, char *lon, char *utctime)
{
    if (NULL == nmea)
    {
        PRINT_MESSAGE_ERROR("nmea frame is NULL\n");
        return false;
    }

    char data[MAX_POISITION_LENGHT] = {0};
    char str_out[MAX_NMEA_FRAME_SIZE] = {0};
    int index = 0;

    // working on temp buff.
#ifdef __cplusplus
    char *nmea_buff = (char*)malloc(MAX_NMEA_FRAME_SIZE);
#else
    char *nmea_buff = malloc(MAX_NMEA_FRAME_SIZE);
#endif
    memset(nmea_buff, 0, MAX_NMEA_FRAME_SIZE);
    strncpy(nmea_buff, nmea, MAX_NMEA_FRAME_STRING);

    char* GgaInfo[] = {"0 xxGGA", "1 time", "2 lat", "3 NS", "4 long",
                       "5 EW", "6 quality", "7 numSV", "8 HDOP", "9 alt",
                       "10 uAlt", "11 sep", "12 uSep", "13 diffAge", 
                       "14 diffStation", "15 cs", "16 <CR><LF>", NULL};
    
    char* GllInfo[] = {"0 xxGLL", "1 lat", "2 NS", "3 long", "4 EW", 
                       "5 time", "6 status", "7 posMode", "8 cs",
                       "9 <CR><LF>", NULL};
    
    char* GnsInfo[] = {NULL};
    
    char** GnssFrame[] = {GgaInfo, GllInfo, GnsInfo, NULL};


    if (NULL != strstr(nmea, "GNS"))
    {
        PRINT_MESSAGE("NMEA: GNSS fix data GNS \n");
        index =2;
        // 0 xxGNS - string $GPGNS GNS Message ID (xx = current Talker ID)
        // 1 time - hhmmss.ss 091547.00 UTC time, see note on UTC representation
        // 2 lat - ddmm.mmmmm
        // 3 NS - character N North/South indicator
        // 4 long - dddmm.mmmmm
        // 5 EW - character E East/West indicator
        // ...
        
        // TODO:
        // step to do
        // 1. check is valid value -- not for GNS
        // 2. get latitude index2 (caculate) + N/S indicator index3
        // 3. get longitude index4 (caculate) + E/W indicator index5
        // 4. get UTC time index1
    }

    else if (NULL != strstr(nmea, "GLL"))
    {
        PRINT_MESSAGE("NMEA: GNSS Latitude and longitude, with time of position fix and status GLL \n");
        index =1;
        // 0 xxGLL - string $GPGLL GLL Message ID (xx = current Talker ID)
        // 1 lat - ddmm.mmmmm 4717.11364 Latitude (degrees & minutes), see format description
        // 2 NS - character N North/South indicator
        // 3 long - dddmm.mmmmm 00833.91565 Longitude (degrees & minutes), see format description
        // 4 EW - character E East/West indicator
        // 5 time - hhmmss.ss 092321.00 UTC time, see note on UTC representation
        // 6 status - character A V = Data invalid or receiver warning, A = Data valid. See position fix flags description.
        // ...

        // TODO:
        // step to do
        // 1. check is valid value -- index6 status: A
        // 2. get latitude index1 (caculate) + N/S indicator index2
        // 3. get longitude index3 (caculate) + E/W indicator index4
        // 4. get UTC time index5
    }

    else if (NULL != strstr(nmea, "GGA"))
    {
        PRINT_MESSAGE("NMEA: GNSSLatitude Global positioning system fix data GGA \n");
        index =0;
        // 0 xxGGA - string $GPGGA GGA Message ID (xx = current Talker ID)
        // 1 time - hhmmss.ss 092725.00 UTC time, see note on UTC representation
        // 2 lat - ddmm.mmmmm 4717.11399 Latitude (degrees & minutes), see format description
        // 3 NS - character N North/South indicator
        // 4 long - dddmm.mmmmm
        // 00833.91590 Longitude (degrees & minutes), see format description
        // 5 EW - character E East/West indicator
        // 6 quality - digit 1 Quality indicator for position fix:
        // 0 = No Fix / Invalid
        // ...

        // TODO:
        // step to do
        // 1. check is valid value -- index6 quality: not 0
        // 2. get latitude index2 (caculate) + N/S indicator index3
        // 3. get longitude index4 (caculate) + E/W indicator index5
        // 4. get UTC time index1
    }

    else
    {
        PRINT_MESSAGE_ERROR("NMEA: not supported \n");
        free(nmea_buff);
        return false;
    }

    int i = 0;
    while (NULL != GnssFrame[index][i])
    {
        if (getNmeaField(nmea_buff, i, data))
        {
            if ('#' == data[0])
                strncpy(data, "null", 5);

            snprintf(str_out, MAX_NMEA_FRAME_SIZE, "GNS: Field %s: %s",
                     GnssFrame[index][i], data);
            PRINT_MESSAGE("%s\n", str_out);
        }
        i++;
    }

    free(nmea_buff);
    return true;
}


int main ()
{
    PRINT_MESSAGE ("+=========================================+\n");
    PRINT_MESSAGE ("%s\n", gnss_nmea);
    
    char output[MAX_NMEA_FRAME_SIZE] = { 0 };
    char lat[MAX_POISITION_LENGHT] = {0};
    char lon[MAX_POISITION_LENGHT] = {0};
    char utctime[MAX_POISITION_LENGHT] = {0};

    if (!getNmeaFrame (gnss_nmea, "GPGGA", &output, sizeof(output)))
    {
        PRINT_MESSAGE("falied to get nmea frame GPGGA\n");
    }
    else
    {
        PRINT_MESSAGE("nmea frame GPGGA: %s\n", output);
        getPosition(&output, &lat, &lon, &utctime);
    }

    // test get field GPGNS
    if (!getNmeaFrame (gnss_nmea, "GPGNS", &output, sizeof(output)))
    {
        PRINT_MESSAGE("falied to get nmea frame GPGNS\n");
    }
    else
    {
        PRINT_MESSAGE("nmea frame GPGNS: %s\n", output);
        getPosition(&output, &lat, &lon, &utctime);
        for (int i=0; i<20; i++)
        {
            if (getNmeaField(&output, i, &lat))
            {
                PRINT_MESSAGE("Data: %s\n", lat);
            }
        }
    }
    
    if (dddmm2Degrees("4717.11399", lat))
    {
        PRINT_MESSAGE("Converted: %s\n", lat);
    }


    return 0;
}
