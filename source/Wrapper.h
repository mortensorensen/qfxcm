#ifndef WRAPPER_H
#define WRAPPER_H

#include "stdafx.h"
#include "ResponseListener.h"
#include "SessionStatusListener.h"
#include "LoginParams.h"
#include "SampleParams.h"
#include "CommonSources.h"
#include "MarketDataAccumulator.h"

#ifdef __cplusplus
class Wrapper
{
public:
	Wrapper();
	virtual ~Wrapper();

    IO2GSession *session;
    SessionStatusListener *sessionListener;
    
    K connect(K host, K user, K password);
    K connected(K ignore);
    
};
#endif /* __cplusplus */

//#ifdef __cplusplus
//extern "C" {
//#endif
//    struct wrapper;
//    
//    struct wrapper *wrapper_create();
//    struct wrapper *wrapper_create_init();
//    void            wrapper_destroy(struct wrapper *w);
//    
//    K connect(K host, K user, K password, K connection);
//    K disconnect(K ignore);
//#ifdef __cplusplus
//}
//#endif

#endif
