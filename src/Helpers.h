#ifndef HELPERS_h
#define HELPERS_h

#include "stdafx.h"

#define DBG(x, e) {if(x) O("%s\n", e);}
#define Q(x, e) if(x) { krr((S)e); R 0; }


inline F zu(I u) { return u / 8.64e4 - 10957; }  // kdb+ datetime from unix
inline I uz(F f) { return 86400 * (f + 10957); } // unix from kdb+ datetime
inline J pu(I u) { return 8.64e13 * (u / 8.64e4 - 10957); } // kdb+ timestamp from unix, use ktj(Kj,n) to create timestamp from n
inline I up(J f) { return (f / 8.64e13 + 10957) * 8.64e4; } // unix from kdb+ timestamp

inline struct tm *lt(int kd) {
    time_t t = uz(kd);
    return localtime(&t);
}

inline struct tm *lt_r(int kd, struct tm *res) {
    time_t t = uz(kd);
    return localtime_r(&t, res);
}

inline struct tm *gt(int kd) {
    time_t t = uz(kd);
    return gmtime(&t);
}

inline struct tm *gt_r(int kd, struct tm *res) {
    time_t t = uz(kd);
    return gmtime_r(&t, res);
}

inline char *fdt(struct tm *ptm, char *d) {
    strftime(d, 10, "%Y.%m.%d", ptm);
    return d;
}

inline void tsms(unsigned ts, char *h, char *m, char *s, short *mmm) {
    *h = ts / 3600000;
    ts -= 3600000 * (*h);
    *m = ts / 60000;
    ts -= 60000 * (*m);
    *s = ts / 1000;
    ts -= 1000 * (*s);
    *mmm = ts;
}

inline char *ftsms(unsigned ts, char *d) {
    char h, m, s;
    short mmm;
    tsms(ts, &h, &m, &s, &mmm);
    sprintf(d, "%02d:%02d:%02d.%03d", h, m, s, mmm);
    return d;
}

static inline F toKTime(const DATE dt) { return roundf((dt - 36526) * 1e10) / 1e10; }

static double toCOleTime(K t) {
    double d;
    struct tm tmBuf = *lt(t->i);
    CO2GDateUtils::CTimeToOleTime(&tmBuf, &d);
    return d;
}

inline K convert(const char *x) { R ks((S)x); }
inline K convert(const std::string &x) { R ks((S)x.c_str()); }
inline K convert(const int x) { R ki(x); }
inline K convert(const double x) { R kf(x); }
inline K convert(const bool x) { R kb(x); }

template <class T>
K vector_to_k_list(const std::vector<T> &vector) {
    K list;
    for (auto it = vector.begin(); it != vector.end(); ++it) {
        jk(&list, convert(&it));
    }
    R list;
}

template <class Key, class Val>
K map_to_k_dict(const std::map<Key, Val> &map) {
    K keys, vals;
    for (auto it = map.begin(); it != map.end(); ++it) {
        jk(&keys, convert(&it->first));
        jk(&vals, convert(&it->second));
    }
    R xD(keys, vals);
}


// http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

static void consumeEvent(const char *fun, K args) {
    if (args->t < 0 || args->t == 10 || args->t > 19)
        args = knk(1, args); // ensure we're always sending a list
    K r = k(0, (S)".fxcm.onrecv .", knk(2, ks((S)fun), args), (K)0);

    if (r->t == -128)
        O("%s\n", r->s);
//        krr(r->s);
    
    r0(r);
}

static void consumeEvent(const char *fun) {
    K identity = ka(101);
    identity->g = 0;
    consumeEvent(fun, identity);
}

#endif