#ifndef _MSL_LOCALE_H
#define _MSL_LOCALE_H

typedef struct {
    char CmptName[8];
    const unsigned short* ctype_map_ptr;
    const unsigned char* upper_map_ptr;
    const unsigned char* lower_map_ptr;
    const unsigned short* wctype_map_ptr;
    const wchar_t* wupper_map_ptr;
    const wchar_t* wlower_map_ptr;
    int (*decode_mb)(wchar_t* pwc, const char* s, size_t n);
    int (*encode_wc)(char* s, wchar_t wchar);
} _loc_ctype_cmpt;

typedef struct _loc_time_cmpt {
    char* am_pm;
    char* DateTime_Format;
    char* Twelve_hr_format;
    char* Date_Format;
    char* Time_Format;
    char* Day_Names;
    char* MonthNames;
    char* TimeZone;
} _loc_time_cmpt;

typedef struct __locale {
    struct __locale* next_locale;
    char locale_name[48];
    void* coll_cmpt_ptr;
    _loc_ctype_cmpt* ctype_cmpt_ptr;
    void* mon_cmpt_ptr;
    void* num_cmpt_ptr;
    _loc_time_cmpt* time_cmpt_ptr;
} __locale;

struct lconv {
    char* decimal_point;
    char* thousands_sep;
    char* grouping;
    char* int_curr_symbol;
    char* currency_symbol;
    char* mon_decimal_point;
    char* mon_thousands_sep;
    char* mon_grouping;
    char* positive_sign;
    char* negative_sign;
    char int_frac_digits;
    char frac_digits;
    char p_cs_precedes;
    char p_sep_by_space;
    char p_sign_posn;
    char n_cs_precedes;
    char n_sep_by_space;
    char n_sign_posn;
    char int_p_cs_precedes;
    char int_p_sep_by_space;
    char int_p_sign_posn;
    char int_n_cs_precedes;
    char int_n_sep_by_space;
    char int_n_sign_posn;
};

extern __locale _current_locale;
extern struct lconv __lconv;

#endif
