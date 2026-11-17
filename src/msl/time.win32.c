#include <stdlib.h>
#include <time.h>
#include "msl/time_api.h"

#pragma avoid_ebp on

time_t __systemtime_to_time_t(SYSTEMTIME* volatile time) {
    SYSTEMTIME* file = time;
    long years, leap_years, leap_days, days, i, secs;

    static int MonthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    years = file->wYear - 1970;

    leap_years = years;
    if (file->wMonth > 2) {
        leap_years++;
    }
    leap_days = (leap_years + 1) / 4;

    days = (years * 365) + leap_days;

    for (i = 1; i < file->wMonth; i++) {
        days += MonthDays[i - 1];
    }
    days += file->wDay - 1;

    secs = (((((days * 24) + file->wHour) * 60) + file->wMinute) * 60) + file->wSecond;

    return secs;
}

#pragma avoid_ebp off

time_t __get_time(void) {
    SYSTEMTIME time;

    GetLocalTime(&time);
    return __systemtime_to_time_t(&time);
}

int __to_gm_time(time_t* time) {
    TIME_ZONE_INFORMATION tzi;
    long diff = 0;
    DWORD result = GetTimeZoneInformation(&tzi);
    switch (result) {
        case TIME_ZONE_ID_UNKNOWN:
            diff = tzi.Bias;
            break;
        case TIME_ZONE_ID_STANDARD:
            diff = tzi.Bias + tzi.StandardBias;
            break;
        case TIME_ZONE_ID_DAYLIGHT:
            diff = tzi.Bias + tzi.DaylightBias;
            break;
        default:
            return 0;
    }

    *time += diff * 60;
    return 1;
}

int __isdst(void) {
    TIME_ZONE_INFORMATION tzi;

    switch (GetTimeZoneInformation(&tzi)) {
        case TIME_ZONE_ID_UNKNOWN:
            return -1;
        case TIME_ZONE_ID_STANDARD:
            return 0;
        case TIME_ZONE_ID_DAYLIGHT:
            return 1;
        default:
            return -1;
    }
}
