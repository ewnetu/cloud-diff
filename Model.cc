
/*
 *  Model.cc
 *  
 *
 *  Created by Ewnetu Bayuh on 11/18/13.
 *  Copyright 2013 Umeå. All rights reserved.
 *
 */
#include "Model.hh"
#include <iostream>
#include <fstream>
#include <math.h>

//using namespace std;
//using namespace dlib;

//using namespace std;

//-------------------------------------------------------------------------------------------------
Model::Model()
{
	
}




//-------------------------------------------------------------------------------------------------
Model::~Model()
{
	
}



//-------------------------------------------------------------------------------------------------
void Model::init(double _taret,double _core,double _performance,double _alpha, double _targetRestponseTime)
{
	//lamda=0.2;//default value-forgetting factor
	lamda=0.2;//default value-forgetting factor
	
	p=1000;
	capacity=_core;
    throughput= _performance;
	alpha=_alpha;
	threshold=5.0;
	targetThroughput=_taret;
	targetResponseTime= _targetRestponseTime;
	 predictedArrivalRate=0;
	 step=0;
	 errorValue=1;
	regressionCofficient=0.05;
	averageArrivalRate=0;
	averageRT=0;
	
}

//-------------------------------------------------------------------------------------------------
void Model::init(double _taret,double _core,double _performance,double _alpha, double forget_factor,double C)
{
	lamda=forget_factor;
	p=C;
	capacity=_core;
    throughput= _performance;
	alpha=_alpha;
	threshold=5.0;
	targetThroughput=_taret;
	//nonLinear.init(_alpha,1.0);
	predictedArrivalRate=0;
	step=0;
	errorValue=1;
	regressionCofficient=1;
	averageArrivalRate=0;
	averageRT=0;
	
}
//-------------------------------------------------------------------------------------------------
void Model::setInput(double _core,double _performance, double _alpha)
{
    
    capacity=_core;
    throughput= _performance;
	alpha=_alpha;
	
	
    
}

//___estimating alpha forthroughput(throughput=alpha*capacity)------------- 
double Model::estimateAlpha(double currentperformance)// this is rls implementation with forgetting factor
{
	//throughput=currentperformance;

	if(currentperformance==0)
		return alpha;// this indicates that the application is doing nothing-> thus no need to compute alpha 
	//double deviation= ((targetThroughput-currentperformance)/targetThroughput)*100;
	///if(abs(deviation)<threshold)
		//return alpha; // no need to estimate alpha
	//otherwise perform estimation
	
	// the gain vector at step n;
	double k=(p*capacity)/(lamda+capacity*capacity*p);
	
	//the estimation error at step n
	double error=currentperformance-alpha*capacity;
	
	//estimated alpha at step n
	alpha=alpha+ k*error;
	
	//update the inverse covariance 
	p=(((1/lamda)*p)*(1 -k*capacity));  

	return alpha;
}

///__________estimating alpha for  response time based on queue length ____________
//double Model::estimateAlpha(double queueLength, double responseTime)
//{
	/* https://en.wikipedia.org/wiki/Recursive_least_squares_filter#RLS_algorithm_summary */

	/* Rename variables for RLS */
	//double x = (queueLength + 1) / capacity;
	//double d = responseTime;
	//double w = alpha; /* old value of alpha */

	/* RLS */
	//double e, g;

	/*e = d - x * w;
	g = p * x / (lamda + x * p * x);
	p = (1.0 / lamda) * p - g * x * (1.0 / lamda) * p;
	w = w + e * g;*/

	/* Rename back */
	//alpha = w;

	/* Return */
	//return alpha;
//}


///__________estimating alpha for  response time(throughput+ 1/currentResponseTime)=alpha*capacity)____________
double Model::estimateAlpha( double responseTime, double averageQueuelength)// this is rls implementation with forgetting factor
{

	double queueLength=averageQueuelength;
	/* https://en.wikipedia.org/wiki/Recursive_least_squares_filter#RLS_algorithm_summary */
	
	/* Rename variables for RLS */
	double x = (queueLength + 1) / capacity;
	double d = responseTime;
	double w = alpha; /* old value of alpha */
	
	/* RLS */
	double e, g;
	
	e = d - x * w;
	g = p * x / (lamda + x * p * x);
	p = (1.0 / lamda) * p - g * x * (1.0 / lamda) * p;
	w = w + e * g;
	
	/* Rename back */
	alpha = w;
	
	/* Return */
	return alpha;
	
}


//-------------------------------------------------------------------------------------------------
void Model::print()
{
	fprintf(stderr,"Performance:%f,  Capacity:%f,  alpha:%f \n",throughput,capacity,alpha);
}

