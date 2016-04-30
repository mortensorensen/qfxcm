#ifndef HELPERS_h
#define HELPERS_h

#include "stdafx.h"
#include "socketpair.h"

#define DBG(x, e) {if(x) O("%s\n", e);}
#define Q(x, e) if(x) { krr((S)e); R 0; }

inline F zu(I u) { return u / 8.64e4 - 10957; }  // kdb+ datetime from unix
inline I uz(F f) { return 86400 * (f + 10957); } // unix from kdb+ datetime
inline struct tm *gt(double kd) {
    time_t t = uz(kd);
    return gmtime(&t);
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

template<int X, int P>
struct Pow
{
    enum { result = X*Pow<X,P-1>::result };
};
template<int X>
struct Pow<X,0>
{
    enum { result = 1 };
};
template<int X>
struct Pow<X,1>
{
    enum { result = X };
};

template <int PRECISION>
float roundP(float f)
{
    const int temp = Pow<10, PRECISION>::result;
    return roundf(f * temp) / temp;
}

static K toKTime(const DATE &dt) {
    struct tm cTime;
    CO2GDateUtils::OleTimeToCTime(roundP<9>(dt), &cTime);
    time_t unixTime = timegm(&cTime);
    R kz(zu(unixTime));
}

static DATE toOleTime(K dt) {
    DATE oleTime;
    CO2GDateUtils::CTimeToOleTime(gt(dt->f), &oleTime);
//    r0(dt);
    R oleTime;
}

static void writeToSocket(int sockets[], const char* func, K x)
{
    // TODO: Lock with mutex
    static char buffer[8192];
    
    K bytes = b9(-1, knk(2, ks((S)func), x));
    r0(x);
    
    memcpy(&buffer, (char*) &bytes->n, sizeof(J));
    memcpy(&buffer[sizeof(J)], kG(bytes), (size_t)bytes->n);
    
    send(sockets[0], buffer, (int)sizeof(J) + bytes->n, 0);
    r0(bytes);
}

static void writeToSocket(int sockets[], const char* func)
{
    K identity = ka(101);
    identity->g = 0;
    writeToSocket(sockets, func, identity);
}

#endif