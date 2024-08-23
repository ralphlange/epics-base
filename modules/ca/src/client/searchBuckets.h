/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* Copyright (c) 2024 ITER Organization.
* SPDX-License-Identifier: EPICS
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/

/* Author: Ralph Lange */
#ifndef INC_searchBuckets_H
#define INC_searchBuckets_H

#include "epicsMutex.h"
#include "epicsTimer.h"

#include "searchTimer.h"

class searchBuckets : private epicsTimerNotify
{
public:
    searchBuckets(class searchTimerNotify &, epicsTimerQueue &, epicsMutex &);
    virtual ~searchBuckets() override;
    enum class SearchKind { initial, check };

    void start(epicsGuard<epicsMutex> &);
    void shutdown(epicsGuard<epicsMutex> &cbGuard, epicsGuard<epicsMutex> &guard);
    void hurryUp(epicsGuard<epicsMutex> &);
    void installChannel(epicsGuard<epicsMutex> &, nciu &);
    void uninstallChan(epicsGuard<epicsMutex> &, nciu &);
    void uninstallChanDueToSuccessfulSearchResponse(epicsGuard<epicsMutex> &,
                                                    nciu &,
                                                    ca_uint32_t respDatagramSeqNo,
                                                    bool seqNumberIsValid,
                                                    const epicsTime &currentTime);
    void show(unsigned level) const override;

private:
    tsDLList<nciu> chanListReqPending;
    tsDLList<nciu> chanListRespPending;
    epicsTime timeAtLastSend;
    epicsTimer &timer;
    searchTimerNotify &iiu;
    epicsMutex &mutex;
    double framesPerTry;              /* # of UDP frames per search try */
    double framesPerTryCongestThresh; /* one half N tries w congest */
    unsigned retry;
    unsigned searchAttempts;  /* num search tries after last timer expiration */
    unsigned searchResponses; /* num search resp after last timer expiration */
    const unsigned index;
    ca_uint32_t dgSeqNoAtTimerExpireBegin;
    ca_uint32_t dgSeqNoAtTimerExpireEnd;
    bool stopped;

    expireStatus expire(const epicsTime &currentTime) override;
    double period(epicsGuard<epicsMutex> &) const;
    searchBuckets(const searchBuckets &);            // not implemented
    searchBuckets &operator=(const searchBuckets &); // not implemented
};

#endif // INC_searchBuckets_H
