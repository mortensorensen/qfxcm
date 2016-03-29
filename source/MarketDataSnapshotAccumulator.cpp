#include "MarketDataSnapshotAccumulator.h"

MarketDataSnapshotAccumulator::MarketDataSnapshotAccumulator()
{
}

MarketDataSnapshotAccumulator::~MarketDataSnapshotAccumulator()
{
}


void MarketDataSnapshotAccumulator::addReader(O2G2Ptr<IO2GMarketDataSnapshotResponseReader> reader)
{
    mReaders.push_back(reader);
}

const K MarketDataSnapshotAccumulator::getTable()
{
    P(mReaders.size() == 0, 0);
    
    if (mReaders[0]->isBar())
        R getBarTable();
    else
        R getTickTable();
}

const J MarketDataSnapshotAccumulator::totalSize()
{
    J N = 0;
    for (auto it = mReaders.begin(), end = mReaders.end(); it != end; ++it)
        N += (*it)->size();
    R N;
}

const K MarketDataSnapshotAccumulator::getTickTable()
{
    K cols = ktn(KS, 3);
    kS(cols)[0] = ss((S) "DateTime");
    kS(cols)[1] = ss((S) "Bid");
    kS(cols)[2] = ss((S) "Ask");
    
    J N = totalSize();
    
    K dateTime = ktn(KF, N);
    K bid = ktn(KF, N);
    K ask = ktn(KF, N);
    
    J i = 0;
    for (auto reader = mReaders.begin(), end = mReaders.end(); reader != end; ++reader)
    {
        for (J j = 0, jj = (*reader)->size(); j < jj; i++, j++)
        {
            kF(dateTime)[i] = zo((*reader)->getDate(j));
            kF(bid)[i] = (*reader)->getBid(j);
            kF(ask)[i] = (*reader)->getAsk(j);
        }
    }
    
    R xT(xD(cols, knk(3, dateTime, bid, ask)));
}

const K MarketDataSnapshotAccumulator::getBarTable()
{
    K cols = ktn(KS, 10);
    kS(cols)[0] = ss((S) "DateTime");
    kS(cols)[1] = ss((S) "BidOpen");
    kS(cols)[2] = ss((S) "BidHigh");
    kS(cols)[3] = ss((S) "BidLow");
    kS(cols)[4] = ss((S) "BidClose");
    kS(cols)[5] = ss((S) "AskOpen");
    kS(cols)[6] = ss((S) "AskHigh");
    kS(cols)[7] = ss((S) "AskLow");
    kS(cols)[8] = ss((S) "AskClose");
    kS(cols)[9] = ss((S) "Volume");
    
    J N = totalSize();
    
    K dateTime = ktn(KZ, N);
    K bidOpen = ktn(KF, N);
    K bidHigh = ktn(KF, N);
    K bidLow = ktn(KF, N);
    K bidClose = ktn(KF, N);
    K askOpen = ktn(KF, N);
    K askHigh = ktn(KF, N);
    K askLow = ktn(KF, N);
    K askClose = ktn(KF, N);
    K volume = ktn(KJ, N);
    
    J i = 0;
    for (auto reader = mReaders.begin(), end = mReaders.end(); reader != end; ++reader)
    {
        for (J j = 0, jj = (*reader)->size(); j < jj; i++, j++)
        {
            kF(dateTime)[i] = zo((*reader)->getDate(j));
            kF(bidOpen)[i] = (*reader)->getBidOpen(j);
            kF(bidHigh)[i] = (*reader)->getBidHigh(j);
            kF(bidLow)[i] = (*reader)->getBidLow(j);
            kF(bidClose)[i] = (*reader)->getBidClose(j);
            kF(askOpen)[i] = (*reader)->getAskOpen(j);
            kF(askHigh)[i] = (*reader)->getAskHigh(j);
            kF(askLow)[i] = (*reader)->getAskLow(j);
            kF(askClose)[i] = (*reader)->getAskClose(j);
            kF(volume)[i] = (*reader)->getVolume(j);
        }
    }
    
    return xT(xD(cols, knk(10, dateTime, bidOpen, bidHigh, bidLow, bidClose, askOpen, askHigh, askLow, askClose, volume)));;
}