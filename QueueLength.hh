/*
 *  QueueLength.hh
 *  
 *
 *  Created by Ewnetu Bayuh on 5/2/14.
 *  Copyright 2014 Umeå. All rights reserved.
 *
 */


#ifndef QUEUELENGTH_HH
#define QUEUELENGTH_HH
#include <list>
class QueueLength
	{
	public:
		// ----------------------------------------------------------------------------------------
		
	
		
		// ----------------------------------------------------------------------------------------
		
		
		QueueLength();
		~QueueLength();
		
		void updateQueueLength(double requestIn,double requestOut);
		void init();
		double computeAverageQueueLength();
		void clearList();
		void setLastQLChangeTime(double time);
		
		
	private:
		
		std::list<double> currentQL;// list of queue length in the current control interval- helps to compute average queue length
		std::list<double> qlDurations;// corresponding duration for currentQL(i)
		double lastQLChangeTime;
		
	};

#endif /* QUEUELENGTH_HH */