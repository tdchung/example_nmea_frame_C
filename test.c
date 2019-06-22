/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <stdio.h>
// #include <string.h>

#ifndef bool
#define bool   int
#define true   1
#define false  0
#endif

#define MAX_NMEA_FRAME_STRING 1023
#define MAX_NMEA_FRAME_SIZE   MAX_NMEA_FRAME_STRING + 1
#define MAX_POISITION_LENGHT  127

char *gnss_nmea = "$GPDTM,999,,0.08,N,0.07,E,-47.7,W84*1C\r\n"
  "$EIGAQ,RMC*2B\r\n"
  "$EIGBQ,RMC*28\r\n"
  "$GPGBS,235458.00,1.4,1.3,3.1,03,,-21.4,3.8,1,0*5B\r\n"
  "$GPGGA,092725.00,4717.11399,N,00833.91590,E,1,08,1.01,499.6,M,48.0,M,,*5B\r\n"
  "$GPGLL,4717.11364,N,00833.91565,E,092321.00,A,A*60\r\n"
  "$GPGNS,091547.00,5114.50897,N,00012.28663,W,AA,10,0.83,111.1,45.6,,,V*71\r\n";


static bool getNmeaFrame (char *buff, char *nmea, char *output, size_t lenght)
{
    char *substr = NULL;
    char *token = NULL;
    char *token_ptr = NULL;
    char *nmea_buff = malloc(MAX_NMEA_FRAME_SIZE);
    
    memset(nmea_buff, 0, MAX_NMEA_FRAME_SIZE);

    strncpy(nmea_buff, buff, MAX_NMEA_FRAME_STRING);
    substr = strstr (nmea_buff, nmea);
    if (NULL == substr)
    {
        printf ("ERROR\n");
        goto getNmeaFrame_False;
    }
    else
    // nmea is available
    printf ("substr = : %s\n", substr);

    // now start get line
    token = strtok_r(substr, "\r", &token_ptr);
    printf("token = : %s\n", token);
    printf("token_ptr = : %s\n", token_ptr);
    if (NULL == token)
    {
        printf("ERROR token\n");
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

static bool getNmeaField(char *nmea, int indexField, char *valueOut)
{
    char *token = NULL;
    char *token_ptr = NULL;
    char* nmea_buff = malloc(MAX_NMEA_FRAME_SIZE);
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
    return false;
}


static bool dddmm2Degrees(char *dddmmIn, char* degreesOut)
{
    // TODO:
    // 1. get degrees. = dddmmIn.slip('.')[0]/100
    // 2. get minutes. = (dddmmIn.slip('.')[0]%100 + 0.dddmmIn.slip('.')[1])
    // 3. decimal = degrees + minutes/60
    // 4. convert to string and return
    return true;
}

static bool getPosition(char *nmea, char *lat, char *lon, char *utctime)
{
    if (NULL != strstr(nmea, "GNS"))
    {
        printf("NMEA: GNSS fix data GNS \n");
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
        printf("NMEA: GNSS Latitude and longitude, with time of position fix and status GLL \n");
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
        printf("NMEA: GNSSLatitude Global positioning system fix data GGA \n");
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
        printf("NMEA: not supported \n");
        return false;
    }
    
    return true;
}


int main ()
{
    printf ("+=========================================+\n");
    printf ("%s\n", gnss_nmea);
    
    char output[MAX_NMEA_FRAME_SIZE] = { 0 };
    char lat[MAX_POISITION_LENGHT] = {0};
    char lon[MAX_POISITION_LENGHT] = {0};
    char utctime[MAX_POISITION_LENGHT] = {0};

    if (!getNmeaFrame (gnss_nmea, "GPGGA", &output, sizeof(output)))
        printf("falied to get nmea frame GPGGA\n");
    else
    {
        printf("nmea frame GPGGA: %s\n", output);
        getPosition(&output, &lat, &lon, &utctime);
    }

    if (!getNmeaFrame (gnss_nmea, "GPGNS", &output, sizeof(output)))
        printf("falied to get nmea frame GPGNS\n");
    else
    {
        printf("nmea frame GPGNS: %s\n", output);
        getPosition(&output, &lat, &lon, &utctime);
        for (int i=0; i<20; i++)
        {
            if (getNmeaField(&output, i, &lat))
                printf("LAT: %s\n", lat);
        }
    }
    
    dddmm2Degrees("1.2", NULL);

    return 0;
}