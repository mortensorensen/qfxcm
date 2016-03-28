#ifndef MarketDataAccumulator_h
#define MarketDataAccumulator_h

#include "stdafx.h"
#include "ResponseListener.h"
#include "CommonSources.h"
//#include "Helpers.h"

class MarketDataAccumulator
{
public:
    MarketDataAccumulator();
    virtual ~MarketDataAccumulator();
    
    void addReader(O2G2Ptr<IO2GMarketDataSnapshotResponseReader> reader);
    const K getTable();
    
private:
    std::vector<O2G2Ptr<IO2GMarketDataSnapshotResponseReader> > mReaders;
    
    const J totalSize();
    
    const K getTickTable();
    const K getBarTable();
};

#endif