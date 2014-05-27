/*
 *  PriorityClass.hh
 *  
 *
 *  Created by Ewnetu Bayuh on 11/9/13.
 *  Copyright 2013 Umeå. All rights reserved.
 *
 */
#ifndef PRIORITYCLASS_HH
#define PRIORITYCLASS_HH

#include <string.h>
//#include <cstddef>        // std::size_t
#include <hash_map>
#include "Application.hh"
#include "HashFunctions.hh"

//#include<unordered_map>
//using namespace std;
/** BEGIN FIX **/

/** END FIX **/

//using namespace __gnu_cxx;
//using namespace std::tr1;

//using namespace std::tr1;


class PriorityClass
{
public:
	//typedef hash_set<Application*, MyHasher, MyComparator> Applications;
	typedef __gnu_cxx::pair <std::string, Application*> VMip2Application;
	typedef __gnu_cxx::hash_map<std::string, Application*> Applications;
	//typedef hash_Map<string, Application> Applications;
	
	//typedef unordered_map <string, Application*> Applications;
	
	PriorityClass(int priority);
	PriorityClass(int priority, Applications* apps);
	PriorityClass();
	~PriorityClass();

	
	PriorityClass::Applications* getApplications();
	void addApplication(Application &app);
	void removeApplication(std::string name);
	Application* getApplication(std::string name);
	void updateMeasuredPerformance(std::string name,double throughput,double responseTime);
	void resetMeasuredPerformance(std::string name);
	inline int size()  { return applications->size(); };
	inline double getRelativePenalty()  { return relativePenalty; };
	inline int getPriority(){return priority;};
	void setRelativePenalty(double _penalty);
	void setCapacity(double _cap,std::string name, double _controlInterval);
	void print();
	void updateArrivalRate(std::string name);
	//Model getModel(std::string name);
	void save(double _controlInterval);
	double getPenalty(double targetPerformance, double violations, int priority);

private:
	double stepWisePenalty(int priority, double percentageOfViolation);
	double relativePenalty;// the penalty assumed for each class
	int priority; // priority of applications in this class(1 being the highest priority, 2- the next highest, ... )
	 Applications *applications; //list of applications in this priority class
	
	
};

#endif /* PRIORITYCLASS_HH */
