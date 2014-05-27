/*
 *  Optimizer.hh
 *  
 *
 *  Created by Ewnetu Bayuh on 11/14/13.
 *  Copyright 2013 Umeå. All rights reserved.
 *
 */


#ifndef OPTIMIZER_HH
#define OPTIMIZER_HH

#include "PriorityClass.hh"
#include "PriorityManager.hh"


//using namespace std; 

class Optimizer
	{
	public:
		
		Optimizer();
		~Optimizer();
		
		
		void optimizeBasedOnRelativePenalty(PriorityManager* _obj,double controlInterval);//implements Minimize( {{\displaystyle \sum_{i=1}^{n}}w_{i}|\Delta p_{i}|} ) i.e., minimize the sum of relative penalty
		void optimisePerPriority(PriorityClass *_obj,double controlInterval,double _capacityToDistribute);////implements Minimize( {{\displaystyle \sum_{i=1}^{n}}|\Delta p_{i}|} ) i.e., minimize the sum of  performance deviation for a single class
		
		
			
	};

#endif /* APPLICATION_HH */