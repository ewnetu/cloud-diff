/*
 *  Optimizer.cc
 *  
 *
 *  Created by Ewnetu Bayuh on 11/14/13.
 *  Copyright 2013 Umeå. All rights reserved.
 *
 */

#include "Optimizer.hh"
#include "flopc.hpp"
#include "OsiCbcSolverInterface.hpp"
#include "OsiClpSolverInterface.hpp"

using namespace flopc;

Optimizer::Optimizer()
{

}


Optimizer::~Optimizer()
{
	
}

void Optimizer::optimizeBasedOnRelativePenalty(PriorityManager* _obj,double controlInterval){
	
	
	MP_model::getDefaultModel().setSolver(new OsiCbcSolverInterface);//OsiClpSolverInterface
	//MP_model &model = MP_model::getDefaultModel();
	//model.setSolver(new OSI_SOLVER_INTERFACE);
	MP_model::getDefaultModel().verbose();
    MP_set APP(_obj->getTotalNApplication());    // size of variable which isthe same as the number of applications 
	
    
    MP_data COST(APP);// (Target/measured)*alpha
	MP_data normalizedTarget(APP);
	MP_data relativePenalty(APP);
	
	//variables
	MP_variable core(APP);
	MP_variable DIFF(APP);
	
	
	//constrains
	MP_constraint  applications;
	MP_constraint abs_x_linctr_a(APP);
	MP_constraint abs_x_linctr_b(APP);
	double predictedCapacity;
	__gnu_cxx::hash_map<int, std::string> *applicationIndex; // an index for extracting the optimized value(number of cores) from the variable for each application
	applicationIndex= new __gnu_cxx::hash_map<int, std::string>();
	int i=0;
	PriorityManager::Priorities *priorityList =_obj->getClasses();
	for (PriorityManager::Priorities::iterator iter = priorityList->begin(); iter != priorityList->end(); iter++)
	{
		PriorityClass *apps = (PriorityClass*)iter->second;
		//double penalty=apps->getRelativePenalty();
		PriorityClass::Applications *applications=apps->getApplications();
		int priority=apps->getPriority();
		

		for(PriorityClass::Applications::iterator appIter=applications->begin(); appIter!=applications->end();appIter++)
		{
			Application *app=appIter->second;
			applicationIndex->insert(std::make_pair(i,app->getName()));
			//COST(i)=app->getAlpha();
			//target(i)=app->getTargetPerformance();
			//double targetPerformance=app->getTargetPerformance();
			//double violations=app->getViolation();
				//normalized value in order to use different performance metrics
			double penalty= apps->getPenalty( app->getTargetPerformance(),  app->getViolation(), priority)+1;
			if(app->getTargetPerformance()!=0){
				COST(i)=app->getAlpha()/app->getTargetPerformance();
				normalizedTarget(i)=1;
				predictedCapacity=app->getTargetPerformance()/app->getAlpha();
			}
			else{
				//COST(i)=1+app->getTargetresponseTime()*app->getMeasuredPerformance()/controlInterval;
				COST(i)=(-1)*app->getTargetResponseTime()/(app->getAlpha()*(fabsf(app->getAverageQueueLength())+1));
				normalizedTarget(i)=-1;
				predictedCapacity=(((fabsf(app->getAverageQueueLength()))+1)/app->getTargetResponseTime())*app->getAlpha();
			}
			
			relativePenalty(i)=penalty;
			//relativePenalty(i)=4;
			core.lowerLimit(i)=app->getBaseCapacity();
			i++;
		}
		
	}
	double cap=_obj->getTotalCores();
    
	DIFF.lowerLimit(APP) = 0.0;//enforcing that the difference should be either positve or zero(i.e., the absolute value)-this can be set to the base capacity
	

	//abs_x_linctr_a(APP) = target(APP)-(COST(APP)*core(APP)) <= DIFF(APP);
	//abs_x_linctr_b(APP) = -1.0*(target(APP)-(COST(APP)*core(APP))) <= DIFF(APP);
	
	
	// absolute value of normalized performance deviation
	abs_x_linctr_a(APP) = normalizedTarget(APP)-(COST(APP)*core(APP)) <= DIFF(APP);
	abs_x_linctr_b(APP) = -1.0*(normalizedTarget(APP)-(COST(APP)*core(APP))) <= DIFF(APP);

	core(APP)<=predictedCapacity;

	
	//the sum of cores assigned to all applications should not be higher than the available core
	applications =sum(APP, core(APP))==cap;
	
	
    
    minimize( sum(APP,relativePenalty(APP)*DIFF(APP)));// minimization
    
    core.display("Optimal solution:");
	
	//extracting the optimal value
	i=0;
	for(__gnu_cxx::hash_map<int, std::string>::iterator it= applicationIndex->begin();it!=applicationIndex->end(); it++)
	{
		double optimalValue=core.level(i);
		std::string name=it->second;
		_obj->setCapacity(optimalValue,name,controlInterval);
	//std::cout<<"VALUE"<<core.level(i);
		i++;
	}
}


//_____________________________________________________________________
void Optimizer::optimisePerPriority(PriorityClass* _obj,double controlInterval,double _capacityToDistribute)
{
	MP_model::getDefaultModel().setSolver(new OsiCbcSolverInterface);//OsiClpSolverInterface
	//MP_model &model = MP_model::getDefaultModel();
	//model.setSolver(new OSI_SOLVER_INTERFACE);
	MP_model::getDefaultModel().verbose();
    MP_set APP(_obj->size());    // size of variable which is the same as the number of applications 
	
    
    MP_data COST(APP);// (measured/target)*alpha--T or  target/(measured*gamma)--R
	MP_data normalizedTarget(APP);
	MP_data relativePenalty(APP);
	
	//variables
	MP_variable core(APP);
	MP_variable DIFF(APP);
	double predictedCapacity;
	
	//constrains
	MP_constraint  applications;
	MP_constraint abs_x_linctr_a(APP);
	MP_constraint abs_x_linctr_b(APP);
	
	__gnu_cxx::hash_map<int, std::string>* applicationIndex; // an index for extracting the optimized value(number of cores) from the variable for each application
	applicationIndex= new __gnu_cxx::hash_map<int, std::string>();
	int i=0;
	
	PriorityClass::Applications *apps =_obj->getApplications();
	int priority=_obj->getPriority();
	
	for(PriorityClass::Applications::iterator appIter=apps->begin(); appIter!=apps->end();appIter++)
	{
		Application *app=appIter->second;
		applicationIndex->insert(std::make_pair(i,app->getName()));
		//printf("\n\nproblem here:::::::");
		//printf("\n\nproblem here:::::::%s",(app->getName()).c_str());
		
		double penalty= _obj->getPenalty( app->getTargetPerformance(),  app->getViolation(), priority)+1;
		if(app->getTargetPerformance()!=0){
			COST(i)=app->getAlpha()/app->getTargetPerformance();
			normalizedTarget(i)=1;
			predictedCapacity=app->getTargetPerformance()/app->getAlpha();
		}
		else{
			//COST(i)=1+app->getTargetresponseTime()*app->getMeasuredPerformance()/controlInterval;
			COST(i)=app->getAlpha()*app->getTargetResponseTime();
			normalizedTarget(i)=1+app->getTargetResponseTime()*app->getMeasuredPerformance()/controlInterval;
			predictedCapacity=(app->getTargetPerformance()+1/app->getTargetResponseTime())/app->getAlpha();
		}
		
		relativePenalty(i)=penalty;
		//COST(i)=app->getAlpha()/app->getTargetPerformance();
		//target(i)=app->getTargetPerformance();
		
		core.lowerLimit(i)=app->getBaseCapacity();
		i++;
	}
	
	double cap=_capacityToDistribute;
	
	// absolute value of performance difference
	//abs_x_linctr_a(APP) = target(APP)-(COST(APP)*core(APP)) <= DIFF(APP);
	//abs_x_linctr_b(APP) = -1.0*(target(APP)-(COST(APP)*core(APP))) <= DIFF(APP);
	
    
	DIFF.lowerLimit(APP) = 0.0;//enforcing that the difference should be either positve or zero(i.e., the absolute value)
	
		
	//absolute of normalized performance deviation  in order to use different performance metrics
	abs_x_linctr_a(APP) =normalizedTarget(APP) - (COST(APP)*core(APP)) <= DIFF(APP);
	abs_x_linctr_b(APP) = -1.0*(normalizedTarget(APP) - (COST(APP)*core(APP))) <= DIFF(APP);
	
	//optimal value should be less than or equal to the demand predicted in the model
	core(APP)<=predictedCapacity;
	
	//the sum of cores assigned to all applications should not be higher than the available core
	applications =sum(APP, core(APP))==cap;
	

    // minimization
    minimize( sum(APP,relativePenalty(APP)*DIFF(APP)));
    
    core.display("Optimal solution:");
	
	//extracting the optimal value
	i=0;
	for(__gnu_cxx::hash_map<int, std::string>::iterator it= applicationIndex->begin();it!=applicationIndex->end(); it++)
	{
		
		printf("\n\nbalance:%f",cap);
	
		double optimalValue=core.level(i);
		printf("\n\ncore:%f",optimalValue);
		std::string name= it->second;
		printf("\n\nname:%s",name.c_str());
		_obj->setCapacity(optimalValue,name,controlInterval);
		//std::cout<<"VALUE"<<core.level(i);
		i++;
	}
}