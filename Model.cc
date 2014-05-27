
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

//__________estimating alpha for  response time based on arrival rate (r=1/(alpha*c-lambda) ____________
double Model::estimateAlpha(double arrivalRate, double responseTime)
{
/* https://en.wikipedia.org/wiki/Recursive_least_squares_filter#RLS_algorithm_summary */

/* Rename variables for RLS */
double x = -alpha/((alpha*capacity-arrivalRate)*(alpha*capacity-arrivalRate));

double d = responseTime;
double w = alpha; /* old value of alpha */

/* RLS */
double e, g;

e = d -1/( capacity * w-arrivalRate);
 g = p * x / (lamda + x * p * x);
 p = (1.0 / lamda) * p - g * x * (1.0 / lamda) * p;
 w = w + e * g;

/* Rename back */
alpha = w;

/* Return */
return alpha;
}

///__________estimating alpha for  response time(throughput+ 1/currentResponseTime)=alpha*capacity)____________
double Model::estimateAlpha( double responseTime, double averageQueuelength)// this is rls implementation with forgetting factor
{
	//predictedArrivalRate=currentArrivalRate/controlInterval;
	//return estimateAlpha( currentArrivalRate/controlInterval,  responseTime);
	//double queueLength=fabsf(currentArrivalRate-currentThroughput);
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
	/*
	//averageArrivalRate= (currentArrivalRate +averageArrivalRate*step)/(step+1);
	//predictedArrivalRate=currentArrivalRate+(currentArrivalRate -averageArrivalRate)*regressionCofficient+errorValue;
	//predictedArrivalRate=currentArrivalRate+averageArrivalRate*0.
	//averageRT=(currentResponseTime+ averageRT*step)/(step+1);
	 //averageRT=(1.0-0.01)*currentResponseTime+averageRT*0.01;
	//currentResponseTime=averageRT;
	//averageRT=currentResponseTime;
	//step+=1;
	
	//throughput=currentThroughput;
	//nonLinear.estimateParameters(capacity,currentThroughput,currentResponseTime);
	//alpha=nonLinear.getAlpha();
	//return alpha;
	//if(currentThroughput==0)
		//return alpha;// this indicates that the application is doing nothing-> thus no need to compute alpha 
	//double deviation= ((targetThroughput-currentperformance)/targetThroughput)*100;
	///if(abs(deviation)<threshold)
	//return alpha; // no need to estimate alpha
	//otherwise perform estimation
	
	//for(int i=0;i<100;i++)
	//{// the gain vector at step n;
	double k=(p*capacity)/(lamda+capacity*capacity*p);
	//double arrivals=std::max(currentArrivalRate/controlInterval,currentThroughput/controlInterval);
	//the estimation error at step n
	//double error=(currentThroughput/controlInterval+ 1/(currentResponseTime))-alpha*capacity;
	//double error=(arrivals+ 1/(currentResponseTime))-alpha*capacity;
	//double error=(1/currentResponseTime)-alpha*capacity;
		//double error= (fabsf(currentArrivalRate-currentThroughput)+1)/currentResponseTime-capacity*alpha;
	double error= alpha*(fabsf(currentArrivalRate-currentThroughput)+1)/capacity-currentResponseTime;
//	double error=((fabsf(currentArrivalRate/controlInterval-currentThroughput/controlInterval)+1)*alpha)/capacity-currentResponseTime;
	//double error=(currentArrivalRate/controlInterval+ 1/(currentResponseTime))-alpha*capacity;;
		//double error=(currentThroughput+ 1/currentResponseTime)-capacity/alpha;
	//
	//double error= 1/currentResponseTime-alpha*capacity;
	//double error= currentResponseTime -alpha/capacity;
	//double error=alpha*(currentThroughput+ 1/currentResponseTime)-capacity;
	//estimated alpha at step n
	//double error=(currentThroughput/(currentResponseTime))-alpha*capacity;
	alpha=alpha+ k*error;
	//update the inverse covariance 
	p=(((1/lamda)*p)*(1 -k*capacity));  
	//}
	
	//fprintf(stderr, "alpa:%f error:%f K:%f p:%f\n",  alpha, error, k,p);
	return alpha;
	//return alpha/(averageRT/targetResponseTime);
	//return alpha/sqrt(currentResponseTime/targetResponseTime);
	//return alpha/pow(currentResponseTime/targetResponseTime,1.0/10.0);
	 */
}


//-------------------------------------------------------------------------------------------------
void Model::print()
{
	fprintf(stderr,"Performance:%f,  Capacity:%f,  alpha:%f \n",throughput,capacity,alpha);
}

