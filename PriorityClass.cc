/*
 *  PriorityClass.cc
 *  
 *
 *  Created by Ewnetu Bayuh on 11/9/13.
 *  Copyright 2013 Umeå. All rights reserved.
 *
 */

#include "PriorityClass.hh"
#include <iostream>

//using namespace std;

PriorityClass::PriorityClass()
{
	//default constructor
	applications=new PriorityClass::Applications();
	
}

//-------------------------------------------------------------------------------------------------
PriorityClass::PriorityClass(int p, Applications *apps)
{
	priority=p;
	applications=apps;
	
}


//-------------------------------------------------------------------------------------------------
PriorityClass::PriorityClass(int p)
{
	priority=p;
	applications=new PriorityClass::Applications();
	
}

//-------------------------------------------------------------------------------------------------
PriorityClass::~PriorityClass()
{
	delete applications;
}



//-------------------------------------------------------------------------------------------------
void PriorityClass::removeApplication(std::string name)
{
	
	Applications::iterator iter = applications->find(name);	
	if (iter != applications->end())
	{
		Application *app = (Application*)iter->second;
		applications->erase(iter);
		delete app;
		
	}
}


//-------------------------------------------------------------------------------------------------
void PriorityClass::setRelativePenalty(double _penalty)
{
	relativePenalty=_penalty;
}

//-------------------------------------------------------------------------------------------------
void PriorityClass::addApplication(Application &_app)
{
	
	Application *app= new Application(_app);
	//cout<<"inside PriorityClass\n";
	//app->print();
	applications->insert(VMip2Application(app->getName(),app));
}


//-------------------------------------------------------------------------------------------------
/*Model PriorityClass::getModel(std::string _name)
{
	Model model;
	Application *app=getApplication(_name);
	if(app!=NULL)
		model= app->getModel();
	
	return  model;;
}*/

//-------------------------------------------------------------------------------------------------
Application* PriorityClass::getApplication(std::string name)
{
	Application *app = NULL;
	
	Applications::iterator iter = applications->find(name);	
	if (iter != applications->end())
	{
		app = (Application*)iter->second;
	}
	return app;	
}


//-------------------------------------------------------------------------------------------------
 double PriorityClass::getPenalty(double targetPerformance, double violations, int priority)
{
	double percentageOfViolation=((targetPerformance-abs(violations))/targetPerformance)*100;
	switch(priority)
	{
		case 1: return 3;
			//return stepWisePenalty(1,percentageOfViolation);
			break;
		case 2:
			return 1;
			//return stepWisePenalty(2,percentageOfViolation);
			break;
		case 3:
			return 0;
			//return stepWisePenalty(3,percentageOfViolation);
			break;
		default:
			printf("wrong paremanter");
			return 0;
	}
								  
}

double PriorityClass::stepWisePenalty(int priority, double percentageOfViolation)
{
	if(priority==1)
	{
	if(percentageOfViolation==100)
		return 0;
	else if(percentageOfViolation<=99)
		return 0.1;
	else if(percentageOfViolation>=98)
		return 0.4;
	else if(percentageOfViolation>=97)
		return 0.8;
	else if(percentageOfViolation>=96)
		return 1.6;
	else if(percentageOfViolation>=95)
		return 3.2;
	else if(percentageOfViolation>=90)
		return 6.4;
	else if(percentageOfViolation>=85)
		return 12.8;
	else if(percentageOfViolation>=80)
		return 25.6;
	else if(percentageOfViolation>=70)
		return 51.2;
	else
		return 102.2;
	}
	else if(priority==2)
	{
		if(percentageOfViolation==100)
			return 0;
		else if(percentageOfViolation<=99)
			return 0.05;
		else if(percentageOfViolation>=98)
			return 0.2;
		else if(percentageOfViolation>=97)
			return 	0.4;
		else if(percentageOfViolation>=96)
			return 0.8;
		else if(percentageOfViolation>=95)
			return 1.6;
		else if(percentageOfViolation>=90)
			return 3.2;
		else if(percentageOfViolation>=85)
			return 6.4;
		else if(percentageOfViolation>=80)
			return 12.8;
		else if(percentageOfViolation>=70)
			return 25.6;
		else
			return 51.2;
	}
	else if(priority==3)
	{
		if(percentageOfViolation==100)
			return 0;
		else if(percentageOfViolation<=99)
			return 0.025;
		else if(percentageOfViolation>=98)
			return 0.1;
		else if(percentageOfViolation>=97)
			return 0.2;
		else if(percentageOfViolation>=96)
			return 0.4;
		else if(percentageOfViolation>=95)
			return 0.8;
		else if(percentageOfViolation>=90)
			return 1.6;
		else if(percentageOfViolation>=85)
			return 3.2;
		else if(percentageOfViolation>=80)
			return 6.4;
		else if(percentageOfViolation>=70)
			return 12.8;
		else
			return 25.6;
	}
	else
		return 0;
}

//-------------------------------------------------------------------------------------------------
void PriorityClass::updateMeasuredPerformance(std::string name, double throughput,double responseTime)
{
	Application *app = NULL;
	
	Applications::iterator iter = applications->find(name);	
	if (iter != applications->end())
	{
		app = (Application*)iter->second;
		app->updateMeasuredPerformance(throughput,responseTime);
	}
}


void PriorityClass::updateArrivalRate(std::string name){

	Application *app = NULL;
	
	Applications::iterator iter = applications->find(name);	
	if (iter != applications->end())
	{
		app = (Application*)iter->second;
		app->updateArrivalRate();
	}
	

}

//-------------------------------------------------------------------------------------------------
void PriorityClass::resetMeasuredPerformance(std::string name)
{
	Application *app = NULL;
	
	Applications::iterator iter = applications->find(name);	
	if (iter != applications->end())
	{
		app = (Application*)iter->second;
		app->resetMeasuredResponseTime();
		app->resetMeasuredPerformance();
	}
}

//-------------------------------------------------------------------------------------------------
void PriorityClass::setCapacity(double _cap,std::string name, double _controlInterval)
{
	Application *app = NULL;
	
	Applications::iterator iter = applications->find(name);	
	if (iter != applications->end())
	{
		app = (Application*)iter->second;
		app->setCapacity(_cap,_controlInterval,false);
	}
	
}
//-------------------------------------------------------------------------------------------------
 PriorityClass::Applications* PriorityClass::getApplications()
{
	
	return applications;
}

//-------------------------------------------------------------------------------------------------
void PriorityClass::print()
{
	fprintf(stderr,"\n\npriority:%i\n",priority);
	for (Applications::iterator iter = applications->begin(); iter != applications->end(); iter++)
	{
		Application *app = (Application*)iter->second;
		app->print();
		
	}
	
}


//-------------------------------------------------------------------------------------------------
void PriorityClass::save( double _controlInterval)
{
	Application *app = NULL;
	
	for (Applications::iterator iter = applications->begin(); iter != applications->end(); iter++)
	{
		if (iter != applications->end())
		{
			app = (Application*)iter->second;
			app->save(_controlInterval);
		}
	}
	
}