/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/

/*
 *  $Id$
 *
 *
 *                    L O S  A L A M O S
 *              Los Alamos National Laboratory
 *               Los Alamos, New Mexico 87545
 *
 *  Copyright, 1986, The Regents of the University of California.
 *
 *
 *	Author Jeffrey O. Hill
 *	johill@lanl.gov
 *	505 665 1831
 *
 * NOTES:
 * 1) This interface is preliminary and will change in the future
 */

#ifndef dbChannelIOh
#define dbChannelIOh

#ifdef epicsExportSharedSymbols
#   define dbChannelIOh_restore_epicsExportSharedSymbols
#   undef epicsExportSharedSymbols
#endif

#include "cxxCompilerDependencies.h"

#ifdef dbChannelIOh_restore_epicsExportSharedSymbols
#   define epicsExportSharedSymbols
#endif

class dbChannelIO : public cacChannel, public dbServicePrivateListOfIO {
public:
    dbChannelIO ( cacChannelNotify &notify, 
        const dbAddr &addr, dbServiceIO &serviceIO );
    ~dbChannelIO ();
    void destroy ();
    void callReadNotify ( unsigned type, unsigned long count, 
            cacReadNotify &notify );
    void callStateNotify ( unsigned type, unsigned long count, 
            const struct db_field_log *pfl, cacStateNotify &notify );
    void show ( unsigned level ) const;
    const char *pName () const;
    void * operator new ( size_t size, tsFreeList < dbChannelIO > & );
#   ifdef CXX_PLACEMENT_DELETE
    void operator delete ( void *, tsFreeList < dbChannelIO > & );
#   endif
private:
    dbServiceIO & serviceIO;
    dbAddr addr;
    void initiateConnect ();
    ioStatus read ( unsigned type, unsigned long count, 
        cacReadNotify &, ioid * );
    void write ( unsigned type, unsigned long count, 
        const void *pvalue );
    ioStatus write ( unsigned type, unsigned long count, 
        const void *pvalue, cacWriteNotify &, ioid * );
    void subscribe ( unsigned type, unsigned long count, 
        unsigned mask, cacStateNotify &notify, ioid * );
    void ioCancel ( const ioid & );
    void ioShow ( const ioid &, unsigned level ) const;
    short nativeType () const;
    unsigned long nativeElementCount () const;
	dbChannelIO ( const dbChannelIO & );
	dbChannelIO & operator = ( const dbChannelIO & );
    void * operator new ( size_t size );
    void operator delete ( void * );
};


inline unsigned long dbChannelIO::nativeElementCount () const 
{
    if ( this->addr.no_elements >= 0u ) {
        return static_cast < unsigned long > ( this->addr.no_elements );
    }
    return 0u;
}

inline const char *dbChannelIO::pName () const 
{
    return addr.precord->name;
}

inline short dbChannelIO::nativeType () const 
{
    return this->addr.dbr_field_type;
}

inline void dbChannelIO::callReadNotify ( unsigned type, unsigned long count, 
        cacReadNotify &notify )
{
    this->serviceIO.callReadNotify ( this->addr, type, count, notify );
}

inline void dbChannelIO::callStateNotify ( unsigned type, unsigned long count, 
        const struct db_field_log *pfl, cacStateNotify &notify )
{
    this->serviceIO.callStateNotify ( this->addr, type, count, pfl, notify );
}


#endif // dbChannelIOh

