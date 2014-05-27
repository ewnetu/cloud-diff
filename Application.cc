
/*
 *  Optimizer.hh
 *  
 *
 *  Created by Ewnetu Bayuh on 11/8/13.
 *  Copyright 2013 Umeå. All rights reserved.
 *
 */
#include "Application.hh"
#include <iostream>
#include <fstream>
#include <sys/time.h>
//using namespace std;

int flag=0,flag1=0;//why o! why!!
double inline now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_usec  + 1000000 *tv.tv_sec;
}
//-------------------------------------------------------------------------------------------------
Application::Application(std::string  _name,double _baseCapacity,double _capacity,std::string _ip, double _tergetPerformance,double _alpha, double _targetRestponseTime)
{
	name.assign(_name);
	baseCapacity=_baseCapacity;
	capacity=_capacity;
	//ip=_ip;
	targetPerformance=_tergetPerformance;
	targetResponseTime=_targetRestponseTime;
	measuredPerformance=0.0;
	measuredResponseTime=0.0;
	prevMeasuredPerformance=0.0;
	arrivalRate=0.0;
	prevArrivalRate=0.0;

	alpha=_alpha;
	performancePerControlInterval=0.0;
	//arrivalPerControlInterval=0.0;
	rtCounter=0.0;
	
	//apacheMonitor.init(_ip);
	model.init(_tergetPerformance, capacity,0.0,alpha,_targetRestponseTime);
	if(_tergetPerformance!=0)
		penalty.init( _tergetPerformance);
	else
		penalty.init( _targetRestponseTime);
	queueLength.setLastQLChangeTime(now());
}

// deep copying the object, well you know it is c++
//-------------------------------------------------------------------------------------------------
Application::Application(const Application& _obj)
{
	
	//copyName(_obj.name);
	name.assign(_obj.name);
	baseCapacity=_obj.baseCapacity;
	capacity=_obj.capacity;
	ip=_obj.ip;
	targetPerformance=_obj.targetPerformance;
	measuredPerformance=_obj.measuredPerformance;
	
	measuredResponseTime=_obj.measuredResponseTime;
	targetResponseTime=_obj.targetResponseTime;
	
	alpha=_obj.alpha;
	model=_obj.model;
	penalty=_obj.penalty;
	arrivalRate=_obj.arrivalRate;
	apacheMonitor=_obj.apacheMonitor;
	prevArrivalRate=_obj.prevArrivalRate;
	prevMeasuredPerformance=_obj.prevMeasuredPerformance;
	performancePerControlInterval=_obj.performancePerControlInterval;
	//arrivalPerControlInterval==_obj.arrivalPerControlInterval;
	rtCounter=_obj.rtCounter;
	
}

//-------------------------------------------------------------------------------------------------
Application::~Application()
{
	
}


//_________________
void  Application::updateMeasuredResponseTime(double rt){
	
	//assumes that this method is called after throughput is updated
	
	//measuredResponseTime=((measuredResponseTime*(measuredPerformance-1))+rt)/measuredPerformance;
	rtCounter+=1;
	measuredResponseTime=((measuredResponseTime*(rtCounter-1))+rt)/rtCounter;
	queueLength.updateQueueLength( arrivalRate, measuredPerformance);
	//measuredResponseTime+=rt;
	//printf("rt time:%f average:%f",rt,measuredResponseTime);
}

//-------------------------------------------------------------------------------------------------
/*void Application::copyName(const char* _name)
{
	if (name)
	{
		delete name;
		name = 0;
	}
	if (_name)
	{
		name = new char(*_name);
	}
}
*/
//-------------------------------------------------------------------------------------------------
double Application::getCapacity()
{
	return capacity;
}

//-------------------------------------------------------------------------------------------------
void Application::setCapacity(double newCap, double _controlInterval, bool setModel/* the model value is changed when this is set to true*/)
{
	capacity=newCap;
	//if(setModel)
	model.setInput(getCapacity(),getMeasuredPerformance()/_controlInterval,alpha);
}

//------------------currently not used-------------------------------------------------------------------------------
unsigned long Application::getVMip()
{
	return ip;
}

//-------------------------------------------------------------------------------------------------
std::string Application::getName() 
{
	return name;
}

//-------------------------------------------------------------------------------------------------
double Application::getBaseCapacity()
{
	return baseCapacity;
}

//-------------------------------------------------------------------------------------------------
void Application::updateMeasuredPerformance(double throughput,double responseTime)
{
	
	measuredPerformance+=throughput;
	updateMeasuredResponseTime(responseTime);
}

void Application::updateArrivalRate(){
	arrivalRate+=1;
}

void Application::resetArrivalRate(){
	prevArrivalRate=arrivalRate;
	//arrivalRate=0;
}
//----------------------returns target throughput---------------------------------------------------------------------------
double Application::getTargetPerformance()
{
	return targetPerformance;
}

//----------------------returns measured throughput per control interval---------------------------------------------------------------------------
double Application::getMeasuredPerformance(){ 
	
	return performancePerControlInterval;
}


///_____________________updates measured throughput and response time_____________________________________
void Application::resetMeasuredPerformance(){	
 prevMeasuredPerformance=measuredPerformance;
	resetMeasuredResponseTime();

}

//-------------------------------persisting the results ------------------------------------------------------------------
void Application::save(double _controlInterval)
{
	double throughput=getMeasuredPerformance();
	double arrival=getArrivalRate();
	//double measuredResponsetime=(measuredResponseTime)/throughput;
	std::ofstream myfile;
	std::string fileName="results/"+name+".csv";
	myfile.open (fileName.c_str(), std::ios::out | std::ios::app); 
	if(flag==0)// put the header
	{
		myfile<<currentDateTime()<<"\n";
	myfile<<" Target Performance,request out, Measured performance, Base Capacity, Optimized Capacity,Non-ptimized Capacity, Estimated Alpha, target ResponseTime, measured responseTime,arrival requests,averageQueuelength \n";
		flag=1;
	}
	myfile<<targetPerformance<<","<<measuredPerformance<<","<<performancePerControlInterval/_controlInterval<<","<<baseCapacity<<","<<capacity<<","<<model.getCapacity()<<","<<alpha<<","<<targetResponseTime<<","<<measuredRTPerInterval<<","<<arrivalRate<<","<<averageQueueLength<<"\n";
	
}


//--------------------------------printing debug info-----------------------------------------------------------------
void Application::saveDebug(double alpha,double requestIn, double requestOut,double responsetime)
{
	
	//double measuredResponsetime=(measuredResponseTime)/throughput;
	std::ofstream myfile1;
	std::string fileName1="results/debug_"+name+".csv";
	myfile1.open (fileName1.c_str(), std::ios::out | std::ios::app); 
	if(flag1==0)// put the header
	{
		myfile1<<currentDateTime()<<"\n";
		myfile1<<" alpha, requestIn, requestOut, responsetime(target) \n";
		flag1=1;
	}
	myfile1<<alpha<<","<<requestIn<<","<<requestOut<<","<<responsetime<<"\n";
	
}

///_____------------------estimating alpha and gamma------------------------------------

double Application::estimateAlpha(double controlInterval)
{
	performancePerControlInterval=measuredPerformance-prevMeasuredPerformance;
	//arrivalPerControlInterval=arrivalRate;
	measuredRTPerInterval=measuredResponseTime;
		
	double _alpha=0.0;
	//is it the right way???
	if(targetPerformance==0)//performance is in terms of response time, the default is throughput 
	{
		//printf("measured response time:%f",measuredResponseTime);
		//double arrival=apacheMonitor.getRequestArrival();
		//double throughput=(measuredPerformance-prevMeasuredPerformance);
		double responseTime=measuredRTPerInterval;
		double arrival=arrivalRate-prevArrivalRate;
		prevArrivalRate=arrivalRate;
		 averageQueueLength=queueLength.computeAverageQueueLength();
		queueLength.clearList();
		_alpha=model.estimateAlpha(responseTime,averageQueueLength);
		
		updateViolation( measuredResponseTime);
	}
	else{
		_alpha=model.estimateAlpha(performancePerControlInterval/controlInterval);
		
		updateViolation( performancePerControlInterval);//it is not really necessary, just to see aggregated violations per service
	}
	return _alpha;
}


//_________________________collect violations, I don't see its importance in its current state ( some improvement needed)__________________________________________________________________________
void Application::updateViolation(double measuredPerformance)
{
	penalty.updateViolation(measuredPerformance);
}

//-------------------------------------------------------------------------------------------------
// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string Application::currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	
    return buf;
}

//-------------------------------print performance results on screen------------------------------------------------------------------
void Application::print()
{
	fprintf(stderr,"Name:%s, Target Performance:%f,Measured Performance:%f, Base Capacity:%f, capacity:%f, alpha:%f, TargetResponseTime:%f, measured Reasponse Time:%f,arrival rate:%f, violation:%f \n",name.c_str(),targetPerformance,getMeasuredPerformance(),baseCapacity,capacity,alpha,targetResponseTime,measuredResponseTime,getArrivalRate(),penalty.getViolation());
	model.print();
}