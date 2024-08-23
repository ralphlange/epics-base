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

#include "searchBuckets.h"

#include <limits.h>
#include <stdexcept>
#include <string> // vxWorks 6.0 requires this include

#include "iocinf.h"
#include "nciu.h"

static const unsigned initialTriesPerFrame = 1u; // initial UDP frames per search try
static const unsigned maxTriesPerFrame = 64u;    // max UDP frames per search try

//
// searchBuckets::searchBuckets ()
//
searchBuckets::searchBuckets(searchTimerNotify &iiuIn,
                             epicsTimerQueue &queueIn,
                             epicsMutex &mutexIn)
    : timeAtLastSend(epicsTime::getCurrent())
    , timer(queueIn.createTimer())
    , iiu(iiuIn)
    , mutex(mutexIn)
    , framesPerTry(initialTriesPerFrame)
    , framesPerTryCongestThresh(DBL_MAX)
    , retry(0)
    , searchAttempts(0u)
    , searchResponses(0u)
    , index(indexIn)
    , dgSeqNoAtTimerExpireBegin(0u)
    , dgSeqNoAtTimerExpireEnd(0u)
    , boostPossible(boostPossibleIn)
    , stopped(false)
{}

void searchBuckets::start(epicsGuard<epicsMutex> &guard)
{
    guard.assertIdenticalMutex(this->mutex);
    this->timer.start(*this, this->period(guard));
}

searchBuckets::~searchBuckets()
{
    assert(this->chanListReqPending.count() == 0);
    assert(this->chanListRespPending.count() == 0);
    this->timer.destroy();
}

void searchBuckets::shutdown(epicsGuard<epicsMutex> &cbGuard, epicsGuard<epicsMutex> &guard)
{
    this->stopped = true;
    {
        epicsGuardRelease<epicsMutex> unguard(guard);
        {
            epicsGuardRelease<epicsMutex> cbUnguard(cbGuard);
            this->timer.cancel();
        }
    }

    while (nciu *pChan = this->chanListReqPending.get()) {
        pChan->channelNode::listMember = channelNode::cs_none;
        pChan->serviceShutdownNotify(cbGuard, guard);
    }
    while (nciu *pChan = this->chanListRespPending.get()) {
        pChan->channelNode::listMember = channelNode::cs_none;
        pChan->serviceShutdownNotify(cbGuard, guard);
    }
}

void searchBuckets::installChannel(epicsGuard<epicsMutex> &guard, nciu &chan)
{
    this->chanListReqPending.add(chan);
    chan.channelNode::setReqPendingState(guard, this->index);
}

void searchBuckets::hurryUp(epicsGuard<epicsMutex> &guard)
{
    // TODO: Do a quick round of the wheel
}

//
// searchBuckets::expire ()
//
epicsTimerNotify::expireStatus searchBuckets::expire(const epicsTime &currentTime)
{
    epicsGuard<epicsMutex> guard(this->mutex);

    // -> moving the channels to the next timer is not necessary
    // while (nciu *pChan = this->chanListRespPending.get()) {
    //     pChan->channelNode::listMember = channelNode::cs_none;
    //     this->iiu.noSearchRespNotify(guard, *pChan, this->index);
    // }

    this->timeAtLastSend = currentTime;

    // boost search period for channels not recently
    // searched for if there was some success
    if (this->searchResponses) {
        // boosting not done this way
        // while (nciu *pChan = this->chanListReqPending.get()) {
        //     pChan->channelNode::listMember = channelNode::cs_none;
        //     this->iiu.boostChannel(guard, *pChan);
        // }
    }

    if (this->searchAttempts) {
        if (this->searchResponses == this->searchAttempts) {
            // increase UDP frames per try if we have a good score
            if (this->framesPerTry < maxTriesPerFrame) {
                // a congestion avoidance threshold similar to TCP is now used
                if (this->framesPerTry < this->framesPerTryCongestThresh) {
                    double doubled = 2 * this->framesPerTry;
                    if (doubled > this->framesPerTryCongestThresh) {
                        this->framesPerTry = this->framesPerTryCongestThresh;
                    } else {
                        this->framesPerTry = doubled;
                    }
                } else {
                    this->framesPerTry += 1.0 / this->framesPerTry;
                }
                debugPrintf(("Increasing frame count to %g t=%u r=%u\n",
                             this->framesPerTry,
                             this->searchAttempts,
                             this->searchResponses));
            }
        } else {
            this->framesPerTryCongestThresh = this->framesPerTry / 2.0;
            this->framesPerTry = 1u;
            debugPrintf(("Congestion detected - set frames per try to %g t=%u r=%u\n",
                         this->framesPerTry,
                         this->searchAttempts,
                         this->searchResponses));
        }
    }

    this->dgSeqNoAtTimerExpireBegin = this->iiu.datagramSeqNumber(guard);

    this->searchAttempts = 0;
    this->searchResponses = 0;

    unsigned nFrameSent = 0u;
    while (true) {
        nciu *pChan = this->chanListReqPending.get();
        if (!pChan) {
            break;
        }

        pChan->channelNode::listMember = channelNode::cs_none;

        bool success = pChan->searchMsg(guard);
        if (!success) {
            if (this->iiu.datagramFlush(guard, currentTime)) {
                nFrameSent++;
                if (nFrameSent < this->framesPerTry) {
                    success = pChan->searchMsg(guard);
                }
            }
            if (!success) {
                this->chanListReqPending.push(*pChan);
                pChan->channelNode::setReqPendingState(guard, this->index);
                break;
            }
        }

        this->chanListRespPending.add(*pChan);
        pChan->channelNode::setRespPendingState(guard, this->index);

        if (this->searchAttempts < UINT_MAX) {
            this->searchAttempts++;
        }
    }

    // flush out the search request buffer
    if (this->iiu.datagramFlush(guard, currentTime)) {
        nFrameSent++;
    }

    this->dgSeqNoAtTimerExpireEnd = this->iiu.datagramSeqNumber(guard) - 1u;

#ifdef DEBUG
    if (this->searchAttempts) {
        char buf[64];
        currentTime.strftime(buf, sizeof(buf), "%M:%S.%09f");
        debugPrintf(("sent %u delay Rts=%s\n", nFrameSent, buf));
    }
#endif

    return expireStatus(restart, this->period(guard));
}

void searchBuckets ::show(unsigned level) const
{
    epicsGuard<epicsMutex> guard(this->mutex);
    ::printf("searchBuckets with period %f\n", this->period(guard));
    if (level > 0) {
        ::printf("channels with search request pending = %u\n", this->chanListReqPending.count());
        if (level > 1u) {
            tsDLIterConst<nciu> pChan = this->chanListReqPending.firstIter();
            while (pChan.valid()) {
                pChan->show(level - 2u);
                pChan++;
            }
        }
        ::printf("channels with search response pending = %u\n", this->chanListRespPending.count());
        if (level > 1u) {
            tsDLIterConst<nciu> pChan = this->chanListRespPending.firstIter();
            while (pChan.valid()) {
                pChan->show(level - 2u);
                pChan++;
            }
        }
    }
}

//
// Reset the delay to the next search request if we get
// at least one response. However, don't reset this delay if we
// get a delayed response to an old search request.
//
void searchBuckets::uninstallChanDueToSuccessfulSearchResponse(epicsGuard<epicsMutex> &guard,
                                                               nciu &chan,
                                                               ca_uint32_t respDatagramSeqNo,
                                                               bool seqNumberIsValid,
                                                               const epicsTime &currentTime)
{
    guard.assertIdenticalMutex(this->mutex);
    this->uninstallChan(guard, chan);

    if (this->stopped) {
        return;
    }

    bool validResponse = true;
    if (seqNumberIsValid) {
        validResponse = this->dgSeqNoAtTimerExpireBegin <= respDatagramSeqNo
                        && this->dgSeqNoAtTimerExpireEnd >= respDatagramSeqNo;
    }

    // if we receive a successful response then reset to a
    // reasonable timer period
    if (validResponse) {
        double measured = currentTime - this->timeAtLastSend;
        this->iiu.updateRTTE(guard, measured);

        if (this->searchResponses < UINT_MAX) {
            this->searchResponses++;
            if (this->searchResponses == this->searchAttempts) {
                if (this->chanListReqPending.count()) {
                    //
                    // when we get 100% success immediately
                    // send another search request
                    //
                    debugPrintf(("All requests succesful, set timer delay to zero\n"));
                    this->timer.start(*this, currentTime);
                }
            }
        }
    }
}

void searchBuckets::uninstallChan(epicsGuard<epicsMutex> &cacGuard, nciu &chan)
{
    cacGuard.assertIdenticalMutex(this->mutex);
    unsigned ulistmem = static_cast<unsigned>(chan.channelNode::listMember);
    unsigned uReqBase = static_cast<unsigned>(channelNode::cs_searchReqPending0);
    if (ulistmem == this->index + uReqBase) {
        this->chanListReqPending.remove(chan);
    } else {
        unsigned uRespBase = static_cast<unsigned>(channelNode::cs_searchRespPending0);
        if (ulistmem == this->index + uRespBase) {
            this->chanListRespPending.remove(chan);
        } else {
            throw std::runtime_error("uninstalling channel search timer, but channel "
                                     "state is wrong");
        }
    }
    chan.channelNode::listMember = channelNode::cs_none;
}

double searchBuckets::period(epicsGuard<epicsMutex> &guard) const
{
    guard.assertIdenticalMutex(this->mutex);
    return (1 << this->index) * this->iiu.getRTTE(guard);
}
