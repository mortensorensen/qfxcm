#ifndef _HELPERS
#define _HELPERS

#include "k.h"
#include "stdafx.h"

F zu(I u) { return u / 8.64e4 - 10957; }  // kdb+ datetime from unix
I uz(F f) { return 86400 * (f + 10957); } // unix from kdb+ datetime
J pu(I u) {
    return 8.64e13 * (u / 8.64e4 - 10957);
} // kdb+ timestamp from unix, use ktj(Kj,n) to create timestamp from n
I up(J f) { return (f / 8.64e13 + 10957) * 8.64e4; } // unix from kdb+ timestamp
struct tm *lt(int kd) {
    time_t t = uz(kd);
    return localtime(&t);
}
struct tm *lt_r(int kd, struct tm *res) {
    time_t t = uz(kd);
    return localtime_r(&t, res);
}
struct tm *gt(int kd) {
    time_t t = uz(kd);
    return gmtime(&t);
}
struct tm *gt_r(int kd, struct tm *res) {
    time_t t = uz(kd);
    return gmtime_r(&t, res);
}
char *fdt(struct tm *ptm, char *d) {
    strftime(d, 10, "%Y.%m.%d", ptm);
    return d;
}
void tsms(unsigned ts, char *h, char *m, char *s, short *mmm) {
    *h = ts / 3600000;
    ts -= 3600000 * (*h);
    *m = ts / 60000;
    ts -= 60000 * (*m);
    *s = ts / 1000;
    ts -= 1000 * (*s);
    *mmm = ts;
}
char *ftsms(unsigned ts, char *d) {
    char h, m, s;
    short mmm;
    tsms(ts, &h, &m, &s, &mmm);
    sprintf(d, "%02d:%02d:%02d.%03d", h, m, s, mmm);
    return d;
}

static bool KTimeToOleTime(K t, double *mDateTo) {
    struct tm tmBuf = *lt(t->i);
    CO2GDateUtils::CTimeToOleTime(&tmBuf, mDateTo);
    R kb(1);
}

// kdb+ time from COleTime
static F zo(const double dt) {
//    struct tm *t = NULL;
//    CO2GDateUtils::OleTimeToCTime(dt, t);
//    R mktime(t);
//    R zu(mktime(t));
    R dt - 36526;
}

K consume_event(const std::string &fun, K x) {
    K ret = k(0, (S)fun.c_str(), x, (K)0);
    
    if (!ret)
        O("Broken socket\n");
    else if (ret->t == -128)
        O("Error calling %s: %s\n", fun.c_str(), ret->s);

    r0(ret);
    R 0;
}

#endif