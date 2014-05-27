/*
 *  PriorityManager.hh
 *  
 *
 *  Created by Ewnetu Bayuh on 11/9/13.
 *  Copyright 2013 Umeå. All rights reserved.
 */

#ifndef PRIORITYMANAGER_HH
#define PRIORITYMANAGER_HH



#include <hash_map>
#include "PriorityClass.hh"
#include "VirtualManager.hh"




//using namespace __gnu_cxx;



class PriorityManager
{
public:
	
	typedef __gnu_cxx::pair <int, PriorityClass*> priority2PriorityClass;
	typedef __gnu_cxx::hash_map<int, PriorityClass*> Priorities;
	
	
	
	PriorityManager();
	~PriorityManager();

	
	PriorityClass::Applications* getApplications(int _priority);// get applications in a given priority class
	void addApplication(int priority, Application &_app);
	void removeApplication(std::string _name);
	Application* getApplication(std::string _name);
	void updateMeasuredPerformance(std::string name,double throughput,double responseTime);
	void resetMeasuredPerformance(std::string name);
	inline int getTotalCores(){return totalCores;};
	inline Priorities* getClasses(){return priorityList;};
	void init(std::string _configFile);// initialize the data structure from configuration file
	void print();//for debug purpose
	int getTotalNApplication();// returns the total number of applications in all classes
	void setCapacity(double _cap, std::string name,double controlInterval);
	//Model getModel(std::string _name);
	void allocate(double controlInterval, bool _usingRelativePenalty);//  _usingRelativePenalty determines per class(if the value is set to false) or across class optimization
	//void allocate(int priority,double controlInterval,double _capacityToDistribute);
	void estimate(double controlInterval);//across class model estimator
	void estimate(PriorityClass *_obj,double controlInterval);//per class model estimator
	double  setMinimumCapacity(double controlInterval);//set minimum capacity to each application and return the extra capacity
	double getPerClassMinCap(PriorityClass *_obj,double controlInterval);//return the total minimum capacity required per class
	void setCapToVM(VirtualManager & _vmMgr);
	void resetMeasuredPerformance();
	void save( double _controlInterval);
	PriorityClass* getClass(int priority);
	void updateArrivalRate(std::string name);
	

private:
	double checkCapacity(double controlInterval);//return true if totalCores can satisfy  all applications in  all classes(return value <0 - insuficient(meaning we need optimization),>=0 sufficient)
	double checkCapacity(PriorityClass *apps,double _capacityToDistribute, double controlInterval);//return true if totalCores can satisfy  all application  in a given classes(return value <0 - insuficient(meaning we need optimization),>=0 sufficient)
	int totalCores;
	 Priorities* priorityList; //list of priority classes
	
	
};

#endif /* PRIORITYMANAGER_HH */
