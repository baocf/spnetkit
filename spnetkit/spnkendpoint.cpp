/*
 * Copyright 2008 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "spnkendpoint.hpp"

#include "spnklist.hpp"
#include "spnkstr.hpp"

SP_NKEndPointList :: SP_NKEndPointList()
{
	mList = new SP_NKVector();
}

SP_NKEndPointList :: ~SP_NKEndPointList()
{
	for( int i = 0; i < mList->getCount(); i++ ) {
		SP_NKEndPoint_t * iter = (SP_NKEndPoint_t*)mList->getItem( i );
		free( iter );
	}

	delete mList;
	mList = NULL;
}

int SP_NKEndPointList :: getCount() const
{
	return mList->getCount();
}

const SP_NKEndPoint_t * SP_NKEndPointList :: getEndPoint( int index ) const
{
	return (SP_NKEndPoint_t*)mList->getItem( index );
}

const SP_NKEndPoint_t * SP_NKEndPointList :: getRandomEndPoint() const
{
	static unsigned int suiSeed = 0;

	int totalWeight = 0;

	time_t nowTime = time( NULL );
	for( int i = 0; i < mList->getCount(); i++ ) {
		SP_NKEndPoint_t * iter = (SP_NKEndPoint_t*)mList->getItem( i );

		if( iter->mEnableTime < nowTime ) totalWeight += iter->mWeight;
	}

	if( 0 == suiSeed ) suiSeed = getpid();

	unsigned int seed = suiSeed++;
	int weight = rand_r( &seed ) % totalWeight;

	SP_NKEndPoint_t * ret = NULL;

	for( int i = 0; i < mList->getCount(); i++ ) {
		SP_NKEndPoint_t * iter = (SP_NKEndPoint_t*)mList->getItem( i );

		if( iter->mEnableTime < nowTime ) {
			ret = iter;
			weight -= iter->mWeight;
			if( weight < 0 ) break;
		}
	}

	return ret;
}

void SP_NKEndPointList :: addEndPoint( const char * ip, int port, int weight )
{
	SP_NKEndPoint_t * endpoint = (SP_NKEndPoint_t*)malloc( sizeof( SP_NKEndPoint_t ) );

	SP_NKStr::strlcpy( endpoint->mIP, ip, sizeof( endpoint->mIP ) );
	endpoint->mPort = port;
	endpoint->mWeight = weight;
	endpoint->mEnableTime = 0;

	mList->append( endpoint );
}

void SP_NKEndPointList :: markPause( const char * ip, int port, int pauseSeconds )
{
	for( int i = 0; i < mList->getCount(); i++ ) {
		SP_NKEndPoint_t * iter = (SP_NKEndPoint_t*)mList->getItem( i );

		if( 0 == strcmp( iter->mIP, ip ) && iter->mPort == port ) {
			iter->mEnableTime = time( NULL ) + pauseSeconds;
		}
	}
}

void SP_NKEndPointList :: markStart( const char * ip, int port )
{
	for( int i = 0; i < mList->getCount(); i++ ) {
		SP_NKEndPoint_t * iter = (SP_NKEndPoint_t*)mList->getItem( i );

		if( 0 == strcmp( iter->mIP, ip ) && iter->mPort == port ) {
			iter->mEnableTime = 0;
		}
	}
}

//===================================================================

SP_NKEndPointTable :: SP_NKEndPointTable( uint32_t tableKeyMax )
{
	mList = new SP_NKVector();
	mTableKeyMax = tableKeyMax;
}

SP_NKEndPointTable :: ~SP_NKEndPointTable()
{
	for( int i = 0; i < mList->getCount(); i++ ) {
		SP_NKEndPointBucket_t * iter = (SP_NKEndPointBucket_t*)mList->getItem( i );
		free( iter );
	}

	delete mList;
	mList = NULL;
}

int SP_NKEndPointTable :: getCount() const
{
	return mList->getCount();
}

const SP_NKEndPointBucket_t * SP_NKEndPointTable :: getBucket( int index ) const
{
	return (SP_NKEndPointBucket_t*)mList->getItem( index );
}

SP_NKEndPointList * SP_NKEndPointTable :: getList( uint32_t key ) const
{
	key = key % mTableKeyMax;

	for( int i = 0; i < mList->getCount(); i++ ) {
		SP_NKEndPointBucket_t * iter = (SP_NKEndPointBucket_t*)mList->getItem( i );

		if( iter->mKeyMin <= key && key <= iter->mKeyMax ) {
			return iter->mList;
		}
	}

	return NULL;
}

const SP_NKEndPoint_t * SP_NKEndPointTable :: getRandomEndPoint( uint32_t key ) const
{
	SP_NKEndPointList * list = getList( key );

	return NULL == list ? NULL : list->getRandomEndPoint();
}

void SP_NKEndPointTable :: addBucket( uint32_t keyMin, uint32_t keyMax, SP_NKEndPointList * list )
{
	SP_NKEndPointBucket_t * bucket =
			(SP_NKEndPointBucket_t*)malloc( sizeof( SP_NKEndPointBucket_t ) );
	bucket->mKeyMin = keyMin;
	bucket->mKeyMax = keyMax;
	bucket->mList = list;

	mList->append( bucket );
}

uint32_t SP_NKEndPointTable :: getTableKeyMax()
{
	return mTableKeyMax;
}
