/*
 *  QueueLength.cc
 *  
 *
 *  Created by Ewnetu Bayuh on 5/2/14.
 *  Copyright 2014 Umeå. All rights reserved.
 *
 */
#include "QueueLength.hh"
#include <sys/time.h>
#include <stddef.h>

double inline now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_usec  + 1000000 *tv.tv_sec;
}

// ----------------------------------------------------------------------------------------
QueueLength::QueueLength(){}

void QueueLength::setLastQLChangeTime(double time){
	lastQLChangeTime=time;
}

// ----------------------------------------------------------------------------------------
QueueLength::~QueueLength(){}

// -----------------------initialize starting time-----------------------------------------------------------------
void QueueLength::init(){
	lastQLChangeTime=now();
}
// ------------------invoked to update queue lenght every time response time is received----------------------------------------------------------------------
void QueueLength::updateQueueLength(double requestIn,double requestOut)
{
	double currentTime=now();// the time returned is in microsecond in order to be precis
	currentQL.push_back(requestIn-requestOut);
	qlDurations.push_back(currentTime-lastQLChangeTime);
	
    lastQLChangeTime=currentTime;
}

// ----------------------compute average queue lenght at  the end of every control interval------------------------------------------------------------------
double QueueLength::computeAverageQueueLength()
{
	double qlSum=0;
	double durSum=0;
	std::list<double>::iterator qlIt, durIt;
	for(qlIt=currentQL.begin(), durIt=qlDurations.begin(); qlIt != currentQL.end() && durIt != qlDurations.end(); ++qlIt, ++durIt)
	{
		qlSum+=(*qlIt)*(*durIt);
		durSum+=*durIt;
		
	}
	if(qlSum==0||durSum==0) return 0.0;
	
	return qlSum/(durSum);
}

// --------------------clear the data at the end of every control interval--------------------------------------------------------------------
void QueueLength:: clearList(){
	lastQLChangeTime=now();
	currentQL.clear();
	qlDurations.clear();
	
}