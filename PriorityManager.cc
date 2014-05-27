
/*
 *  PriorityManager.cc
 *  
 *
 *  Created by Ewnetu Bayuh on 11/9/13.
 *  Copyright 2013 Umeå. All rights reserved.
 */

#include "PriorityManager.hh"
#include <json/json.h>//libjson_linux-gcc-4.6_libmt.so must be copied to /usr/lib
#include <json/reader.h>
#include <fstream>
#include<math.h>
#include "Optimizer.hh"
#include "PriorityClass.hh"

PriorityManager::PriorityManager()
{
	//default constructor
	//applications=new Applications();
	priorityList=new Priorities();
	
}


//-------------------------------------------------------------------------------------------------
PriorityManager::~PriorityManager()
{
	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		delete apps;
	}
	delete priorityList;
}


//-------------------------------------------------------------------------------------------------
void PriorityManager::removeApplication(std::string name)
{
	
	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		apps->removeApplication(name);// remove application
		if(apps->size()==0)//if the class is empty then remove it from the list(this may need also upgrading lower classes????)
		{
			priorityList->erase(iter);
			delete apps;
		}
		
		
	}
}


//-------------------------------------------------------------------------------------------------
/*Model PriorityManager::getModel(std::string _name)
{
	Model model;
	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		Application*app=apps->getApplication(_name);
		if( app!=NULL)
			 model=apps->getModel(_name);
		
	}
	return model;
}*/

//-------------------------------------------------------------------------------------------------
void PriorityManager::addApplication(int priority,Application &_app)
{
	
	Priorities::iterator iter = priorityList->find(priority);	
	if (iter != priorityList->end()) // add the application in the list of application with the same priority
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		apps->addApplication(_app);
		
	}
	else// create a new list if the application is the first in this priority class
	{
		//cout<<"inside PriorityManager\n";
		PriorityClass *apps=new PriorityClass(priority);
		apps->addApplication(_app);
		priorityList->insert(priority2PriorityClass(priority,apps));
	}
}


//-------------------------------------------------------------------------------------------------
Application* PriorityManager::getApplication(std::string name)
{
	Application *app = NULL;
	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		app=apps->getApplication(name);
		if( app!=NULL)
			return app;
		
	}
	return app;	
}


//-------------------------------------------------------------------------------------------------
void PriorityManager::updateMeasuredPerformance(std::string name,double throughput,double responseTime)
{
	
	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		apps->updateMeasuredPerformance(name, throughput, responseTime);
		
		
	}
	
}

void PriorityManager::updateArrivalRate(std::string name){

	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		apps->updateArrivalRate(name);
		
		
	}
	
}

//-------------------------------------------------------------------------------------------------
void PriorityManager::resetMeasuredPerformance(std::string name)
{
	
	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		apps->resetMeasuredPerformance(name);
		
		
	}
}





//-------------------------------------------------------------------------------------------------
void PriorityManager::setCapacity(double _cap,std::string name, double controlInterval)
{
	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		apps->setCapacity(_cap,name, controlInterval);
		
		
	}
	
}

//-------------------------------------------------------------------------------------------------
 PriorityClass::Applications* PriorityManager::getApplications(int priority)
{
	
	PriorityClass::Applications *apps = NULL;
	
	Priorities::iterator iter = priorityList->find(priority);
	if (iter != priorityList->end())
	{
		PriorityClass* pClass = (PriorityClass*)iter->second;
		apps=pClass->getApplications();
		
	}
	return apps;	
}


//-------------------------------------------------------------------------------------------------
PriorityClass* PriorityManager::getClass(int priority)
{
	
	//PriorityClass::Applications *apps = NULL;
	
	Priorities::iterator iter = priorityList->find(priority);
	if (iter != priorityList->end())
	{
		return (PriorityClass*)iter->second;
		
		
	}
	return NULL;	
}


//-------------------------------------------------------------------------------------------------
//This is not a well thought method. It doesn't work if the json structure changes
void PriorityManager::init(std::string _configFile)
{
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;
	std::ifstream file(_configFile.c_str());
	bool parsingSuccessful = reader.parse( file, root );
	if ( !parsingSuccessful )
	{
		std::cout<< "Failed to parse configuration\n";//<<reader.getFormattedErrorMessages().c_str();
		return;
	}
	 totalCores = root.get("totalcores",0 ).asInt();
	const Json::Value classes = root["classes"];
	for(Json::Value::iterator it = classes.begin(); it !=classes.end(); ++it) // Iterates over the classes and generate c++ data structure( look the json file to understand this).
	{
		Json::Value key = it.key();
		int _priority= atoi(key.asString().c_str());
		Json::Value priorityClass = (*it);
		//cout<<"\n\npriority: "<<_priority<<"\n";
		//cout<<"Value: "<<priorityClass.toStyledString()<<"\n";
		for(Json::Value::iterator classIt = priorityClass.begin(); classIt !=priorityClass.end(); ++classIt)
		{
			//Json::Value key = classIt.key();
			Json::Value value = (*classIt);
			for(Json::Value::iterator apps = value.begin(); apps !=value.end(); ++apps)
			{
				Json::Value key = apps.key();
				Json::Value value = (*apps);
				std::string _name=key.asString();
				double _targetPerformance=value.get("targetThroughput",0.0).asDouble();
				double _targetResponseTime=value.get("targetResponseTime",0.0).asDouble();
				double _baseCapacity=value.get("baseCapacity",0.0).asDouble();
				double _capacity=value.get("capacity",0.0).asDouble();
				double _alpha=value.get("alpha",0.0).asDouble();
				printf("alpha:%f",_alpha);
				std::string ip=value.get("ip","0.0").asString();
				printf("IP:%s",ip.c_str());
				//cout<<"Key: "<<_name;
				//cout<<"\ntargetPerformance: "<<_targetPerformance<<"\n";
				//cout<<"baseC: "<<_baseCapacity<<"\n";
				//cout<<"Cap: "<<_capacity<<"\n";
				
				Application *_app= new Application(_name,_baseCapacity,_capacity,ip, _targetPerformance,_alpha,_targetResponseTime);
				//_app->setAlpha(_alpha);
				addApplication(_priority,*_app);//copy the application object it is proper priority group
				delete _app;// to avoid memory leak
				
			}
			
		}
	}
	
}

//-------------------------------------------------------------------------------------------------
void PriorityManager::print()
{
	fprintf(stderr,"TotalCores:%d \n",totalCores);
	
	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		apps->print();
		
	}
	
}

//__________________________________________________________________________
int PriorityManager::getTotalNApplication()
{
	int total=0;
	
	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		total+=apps->size();
		
	}
	
	return total;
}


//__________________________________________________________________________
void PriorityManager::save(double _controlInterval)
{
	
	
	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		apps->save(_controlInterval);
		
	}
	
	
}


//-------------------------------------------------------------------------------------------------
void  PriorityManager::resetMeasuredPerformance()
{
	
	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		PriorityClass::Applications *applications =apps->getApplications();
		for ( PriorityClass::Applications::iterator iter = applications->begin(); iter != applications->end(); iter++)
		{
			Application *app = (Application*)iter->second;
			app->resetMeasuredResponseTime();
			app->resetMeasuredPerformance();
			app->resetArrivalRate();
			
		}
		
	}
}


//-------------------------------------------------------------------------------------------------
//return value <0 - insuficient(meaning we need optimization),>=0 sufficient
double  PriorityManager::checkCapacity(double controlInterval)
{
	double totalCoresNeeded=0;
	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		PriorityClass::Applications *applications =apps->getApplications();
		for ( PriorityClass::Applications::iterator iter = applications->begin(); iter != applications->end(); iter++)
		{
			
			Application *app = (Application*)iter->second;
			
			double coresPerApp=app->getTargetPerformance()/app->getAlpha();
			if(app->getTargetPerformance()==0){
				double alpha=app->getAlpha();
				double arrivalRate=app->getArrivalRate();
				double throughput=app->getMeasuredPerformance();
				
				//coresPerApp=(((fabsf(arrivalRate-throughput))+1)/app->getTargetResponseTime())*alpha;
				//printf("cores:%f\n",coresPerApp);
				//coresPerApp=(((fabsf(arrivalRate-throughput))+1)/app->getTargetResponseTime())*alpha;
				coresPerApp=(((fabsf(app->getAverageQueueLength()))+1)/app->getTargetResponseTime())*alpha;
				//app->saveDebug(alpha,arrivalRate,throughput,app->getTargetResponseTime());
				//double arrivals=std::max(arrivalRate/controlInterval,throughput/controlInterval);
				//coresPerApp=(((fabsf(arrivalRate-throughput))+1)/app->getTargetResponseTime())/alpha;
				//coresPerApp=(arrivalRate+1/app->getTargetResponseTime())/alpha;
				//coresPerApp=((app->getMeasuredPerformance()/controlInterval)+1/(app->getTargetResponseTime()))/(app->getAlpha());
				////coresPerApp=1/(app->getAlpha()*app->getTargetResponseTime());
				//coresPerApp=((app->getMeasuredPerformance()/controlInterval)/app->getTargetResponseTime())/(app->getAlpha());
				//coresPerApp=1/(app->getTargetResponseTime()*app->getAlpha());//coresPerApp=((app->getMeasuredPerformance()/controlInterval)+1/(app->getTargetResponseTime()))/(app->getAlpha());
				//coresPerApp=1/(app->getAlpha()*app->getTargetResponseTime());
				//coresPerApp=(app->getAlpha()/app->getTargetResponseTime());
					printf("measured performance:%f alpha:%f arrival:%f core:%f\n",throughput,app->getAlpha(),arrivalRate,coresPerApp);
			}
		
			app->setCapacity(coresPerApp,controlInterval,true);
			//printf("measured performance:%f alpha:%f cores:%f\n",app->getMeasuredPerformance()/controlInterval,app->getAlpha(),coresPerApp);
			totalCoresNeeded+=coresPerApp;
			
		}
		
	}
	

	return totalCores-totalCoresNeeded;
}


//-------------------------------------------------------------------------------------------------
//return value <0 - insuficient(meaning we need optimization),>=0 sufficient 
double  PriorityManager::checkCapacity(PriorityClass *apps,double _coresToDistribute, double controlInterval)
{
	double totalCoresNeeded=0;
	
	PriorityClass::Applications *applications =apps->getApplications();
	for ( PriorityClass::Applications::iterator iter = applications->begin(); iter != applications->end(); iter++)
	{
		
		Application *app = (Application*)iter->second;
		//printf("measured performance:%f",app->getMeasuredPerformance()/_controlInterval);
		double coresPerApp=app->getTargetPerformance()/app->getAlpha();
		if(app->getTargetPerformance()==0)//for response time
			{
				double alpha=app->getAlpha();
				double arrivalRate=app->getArrivalRate();
				double throughput=app->getMeasuredPerformance();
				double arrivals=std::max(arrivalRate/controlInterval,throughput/controlInterval);
				//coresPerApp=(((fabsf(arrivalRate-throughput))+1)/app->getTargetResponseTime())*alpha;
				coresPerApp=(((fabsf(app->getAverageQueueLength()))+1)/app->getTargetResponseTime())*alpha;
			//	coresPerApp=(((fabsf(arrivalRate-throughput))+1)/app->getTargetResponseTime())/alpha;
				//coresPerApp=(((fabsf(arrivalRate/controlInterval-throughput/controlInterval))+1)/app->getTargetResponseTime())*alpha;
				//coresPerApp=(arrivals+1/app->getTargetResponseTime())/alpha;
			// coresPerApp=((app->getMeasuredPerformance()/controlInterval)+1/(app->getTargetResponseTime()))/(app->getAlpha()); //coresPerApp=((app->getMeasuredPerformance()/controlInterval)+1/app->getTargetResponseTime())*(app->getAlpha());
				//coresPerApp=app->getAlpha()*(((app->getMeasuredPerformance()/_controlInterval)*app->getTargetResponseTime()+1)/(app->getTargetResponseTime()));// coresPerApp=1/(app->getAlpha()*app->getTargetResponseTime());
				//coresPerApp=1/(app->getTargetResponseTime()*app->getAlpha());	//coresPerApp=((app->getMeasuredPerformance()/_controlInterval)+1/(app->getTargetResponseTime()))*(app->getAlpha());
				//coresPerApp=1/(app->getAlpha()*app->getTargetResponseTime());
				//coresPerApp=(app->getAlpha()/app->getTargetResponseTime());
			} //			coresPerApp=((app->getMeasuredPerformance()/_controlInterval)+1/app->getTargetResponseTime())*(app->getAlpha());//coresPerApp=1/(app->getAlpha()*app->getTargetResponseTime());///
		app->setCapacity(coresPerApp,controlInterval,true);
		totalCoresNeeded+=coresPerApp;
		
	}
	
	return _coresToDistribute-totalCoresNeeded;
}

//-------------------------------------------------------------------------------------------------
//allocate capacity to applications. The steps of capacity allocations are:
// 1)Online Model Estimation-estimate the value of alpha for each application using RLS
//2)check if the capacity is sufficient for all applications 
//3) Optimization-if the capcity is not sufficient then perform optimization 
//-------------------------------------------------------------------------------------------------
void PriorityManager::allocate(double controlInterval, bool _usingRelativePenalty)
{
	//Model estimation using RLS for each applications
	estimate(controlInterval);
	
	if(_usingRelativePenalty)//across class optimization
	{
		
		
		
		if(checkCapacity(controlInterval)<0)
		{
			Optimizer opt;
			opt.optimizeBasedOnRelativePenalty(this, controlInterval);
		}
	}
	else//per class optimization(strict priority)
	{
		double extra=setMinimumCapacity(controlInterval);//allocate minimum capacity to each app and return the surpless.
		int p=1;//the first priority class
		//distribute the extra capacity to each application down the class hierarchy
		double remainingBalance=0,capToBeDistributed=0;
		for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
		{
			
			PriorityClass *pClass =getClass(p);
			
			
			double totalMinCapPerClass=getPerClassMinCap(pClass,controlInterval);
			//per class model estimation using RLS
			//estimate(pClass,controlInterval);
			
			if(p==1)
				capToBeDistributed=extra+totalMinCapPerClass;
			else
				capToBeDistributed=remainingBalance+totalMinCapPerClass;
			
			remainingBalance=checkCapacity(pClass, capToBeDistributed, controlInterval);
			
			if(remainingBalance<0)
			{
				Optimizer opt;
				opt.optimisePerPriority(pClass, controlInterval,capToBeDistributed);
				
				return;
			}
			
			p++;//advance to the next priority class
		}
	}
}

//-------------------------------------------------------------------------------------------------
void  PriorityManager::estimate(double controlInterval)//non-strict estimation
{
	
	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		PriorityClass::Applications *applications =apps->getApplications();
		for ( PriorityClass::Applications::iterator it = applications->begin(); it != applications->end(); it++)
		{
			Application *app = (Application*)it->second;
			//double currentPerformance =app->getMeasuredPerformance();
			double newAlpha=app->estimateAlpha(controlInterval);
			app->setAlpha(newAlpha);
			//double coresPerApp=app->getTargetPerformance()/newAlpha;
			//(app->getModel()).setInput((coresPerApp,getMeasuredPerformance()/_controlInterval);// store cores and througput for computing the next interval alpha estimation
			//app->setAlpha(newAlpha);
			
			
		}
		
	}
}
	//-------------------------------------------------------------------------------------------------
	/*void  PriorityManager::estimate(PriorityClass *_obj,double controlInterval)//per class estimation
	{
		
			
		PriorityClass::Applications *applications =_obj->getApplications();
		for ( PriorityClass::Applications::iterator iter = applications->begin(); iter != applications->end(); iter++)
			{
				Application *app = (Application*)iter->second;
				double currentPerformance =app->getMeasuredPerformance()/controlInterval;
				double newAlpha=app->estimateAlpha(currentPerformance);
				app->setAlpha(newAlpha);
				
			}
	}
*/
	
	//-------------------------------------------------------------------------------------------------
	double  PriorityManager::setMinimumCapacity(double controlInterval)//set minimum capacity and return the extra  
	{
		
		double capacityUsed=0;
		for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
		{
			PriorityClass *apps = (PriorityClass*)iter->second;
			PriorityClass::Applications *applications =apps->getApplications();
			for ( PriorityClass::Applications::iterator it = applications->begin(); it != applications->end(); it++)
			{
				Application *app = (Application*)it->second;
				double minCap=app->getBaseCapacity();
				app->setCapacity(minCap,controlInterval,false);
				capacityUsed+=minCap;
			}
		}
		
		return  totalCores-capacityUsed;			
	}


//_________________________________________________________________________________
double PriorityManager::getPerClassMinCap(PriorityClass *_obj, double controlInterval)
{
	double totalMinCap=0;
	
	
	PriorityClass::Applications *applications =_obj->getApplications();
	for ( PriorityClass::Applications::iterator iter = applications->begin(); iter != applications->end(); iter++)
	{
		Application *app = (Application*)iter->second;
		double minCap=app->getBaseCapacity();
		app->setCapacity(minCap,controlInterval,false);
		totalMinCap+=minCap;
		
	}
	
	return  totalMinCap;
	
}


//-------------------------------------------------------------------------------

void PriorityManager::setCapToVM(VirtualManager & _vmMgr)

{
	
	for (Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		PriorityClass::Applications *applications =apps->getApplications();
		for ( PriorityClass::Applications::iterator iter = applications->begin(); iter != applications->end(); iter++)
		{
			Application *app = (Application*)iter->second;
			std::string vm=app->getName();
			int cap=static_cast< int>(app->getCapacity()*100);
			_vmMgr.setVmCap(vm,cap );
			//(app->getModel()).setInput((app->getCapacity(),(app->getMeasuredPerformance())/_controlInterval);// store cores and througput for computing the next interval alpha estimation
			
		}
		
	}
	
	
}