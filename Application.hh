
/*
 *  Application.hh
 *  
 *
 *  Created by Ewnetu Bayuh on 11/8/13.
 *  Copyright 2013 Umeå. All rights reserved.
 *
 */

#ifndef APPLICATION_HH
#define APPLICATION_HH

#include <string>
#include "Model.hh"
#include "Penalty.hh"
#include "ApacheMonitor.hh"
#include "QueueLength.hh"
//using namespace std; 

class Application
{
public:
	Application(std::string name,double baseCapacity,double capacity,std::string ip, double targetPerformance,double _alpha,double _targetRestponseTime);
	Application(const Application& _obj);
	~Application();
	
	
	double getCapacity();
	void setCapacity(double newCap,double _controlInterval,bool setModel);
	std::string getName() ;
	unsigned long getVMip();
	//void copyName(const char* _name);
	double getBaseCapacity();
	double getTargetPerformance();
	void updateMeasuredPerformance(double throughput, double responseTime);
	 void resetMeasuredPerformance();
	 double getMeasuredPerformance();
	inline double getMeasuredResponseTime(){return measuredRTPerInterval;};
	inline double getTargetResponseTime(){return targetResponseTime;};
	inline void resetMeasuredResponseTime(){measuredResponseTime=0.0;rtCounter=0;};
	void  updateMeasuredResponseTime(double rt);
	inline double getAlpha(){ return alpha;};
	inline void setAlpha(double _alpha){  alpha=_alpha;};
	double estimateAlpha(double controlInterval);
	void print();
	double getViolation(){return penalty.getViolation();};
	void updateViolation(double measuredPerformance);
	inline Model getApplicationModel(){return model;}
	void save(double _controlInterval);
	const std::string currentDateTime();
	void updateArrivalRate();
	void resetArrivalRate();
	inline double getArrivalRate(){ return arrivalPerControlInterval;/*model.getPredictedArrivalRate();*/};
	void saveDebug(double alpha,double requestIn, double requestOut,double responsetime);
	inline double getAverageQueueLength(){return averageQueueLength;};
private:
	std::string name;//domain name of the vm in xen (application)
	double targetPerformance;//througput
	double measuredPerformance,prevMeasuredPerformance;//throughput
	double measuredResponseTime;
	double targetResponseTime;
	double alpha;//model parameter, the same variable is used for both alpha and gamma specified on the paper
	double arrivalRate,prevArrivalRate;
	double baseCapacity; //minimum capacity assigned for the application
	double capacity; //the capacity assigned due to changes in application demand(capacity>=baseCapacity)
	unsigned long ip;//the ip address where the app(vm) is running
	Model model; // model estimator for alpha and gamma
	
	Penalty penalty; //currently it is used for setting the penalty weights per class. May be useful in the future to include complex penalty model
	ApacheMonitor apacheMonitor;//to get arrival rate from apache
	double performancePerControlInterval; // measured througput per control interval
	//double arrivalPerControlInterval; // 
	double rtCounter;// this should not be its place but it is a counter that is used to calculate average response time (similar to  weighted average)
	double measuredRTPerInterval;//measured response time per control interval
	 QueueLength queueLength; //a class that does the magic of calculating queue lenght
	double averageQueueLength;// the queue length per control interval
	
	
};

#endif /* APPLICATION_HH */
