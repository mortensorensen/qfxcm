#include "stdafx.h"
#include "TableListener.h"
#include "Helpers.h"

TableListener::TableListener()
{
    mRefCount = 1;
    mInstrument = "";
}

TableListener::~TableListener(void)
{
}

long TableListener::addRef()
{
    return InterlockedIncrement(&mRefCount);
}

long TableListener::release()
{
    InterlockedDecrement(&mRefCount);
    if(mRefCount == 0)
        delete this;
    return mRefCount;
}

void TableListener::setInstrument(const char *sInstrument)
{
    mInstrument = sInstrument;
}

void TableListener::onAdded(const char *rowID, IO2GRow *row)
{
}

void TableListener::onChanged(const char *rowID, IO2GRow *row)
{
    if (row->getTableType() == Offers) {
        printOffer((IO2GOfferTableRow *)row, mInstrument.c_str());
    }
}

void TableListener::onDeleted(const char *rowID, IO2GRow *row)
{
    consumeEvent("ondeleted", ks((S)rowID));
}

void TableListener::onStatusChanged(O2GTableStatus status)
{
    consumeEvent("onstatuschanged", ki(status));
}

void TableListener::printOffers(IO2GOffersTable *offersTable, const char *sInstrument)
{
    IO2GOfferTableRow *offerRow = NULL;
    IO2GTableIterator iterator;
    while (offersTable->getNextRow(iterator, offerRow)) {
        printOffer(offerRow, sInstrument);
        offerRow->release();
    }
}

void TableListener::printOffer(IO2GOfferTableRow *offerRow, const char *sInstrument)
{
    if(!(offerRow->isTimeValid() && offerRow->isBidValid() && offerRow->isAskValid()))
        return;

    K tick = knk(4,
                 toKTime(offerRow->getTime()),
                 ks((S)offerRow->getInstrument()),
                 kf(offerRow->getBid()),
                 kf(offerRow->getAsk()));
    consumeEvent("onoffer", tick);
}

void TableListener::subscribeEvents(IO2GTableManager *manager)
{
    O2G2Ptr<IO2GOffersTable> offersTable = (IO2GOffersTable *)manager->getTable(Offers);
    
    offersTable->subscribeUpdate(Update, this);
}

void TableListener::unsubscribeEvents(IO2GTableManager *manager)
{
    O2G2Ptr<IO2GOffersTable> offersTable = (IO2GOffersTable *)manager->getTable(Offers);
    
    offersTable->unsubscribeUpdate(Update, this);
}
