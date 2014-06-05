
/*
 *  Model.hh
 *  
 *
 *  Created by Ewnetu Bayuh on 11/18/13.
 *  Copyright 2013 Umeå. All rights reserved.
 *
 */

#ifndef MODEL_HH
#define MODEL_HH


 //#include "NonLinearModel.hh"

class Model
{
public:
	// ----------------------------------------------------------------------------------------
	
	//typedef dlib::matrix<double,2,1> input_vector;
	//typedef dlib::matrix<double,1,1> parameter_vector;
	//typedef std::vector<std::pair<input_vector, parameter_vector> > HISTORY;
	
	// ----------------------------------------------------------------------------------------
	
	
	Model();
	~Model();
	
	void init(double _taret,double _alpha,double _core,double _performance, double forget_factor,double C);
	void init(double _taret,double _core,double _performance,double _alpha, double _targetRestponseTime);
	double estimateAlpha(double currentperformance);//estimates alpha for throughput using rls
	double estimateAlpha( double currentResponseTime,double averageQueuelength);//estimates alpha for response time using rls
    void setInput(double _core,double _performance,double _alpha);
	//void setAverageResponseTime(double responseTime);
	inline double getCapacity(){return capacity;};
	inline double getPredictedArrivalRate(){return predictedArrivalRate;};
	//double estimateAlpha(double arrivalRate, double responseTime);
	///inline double getCapacity(){return capacity;};
	void print();
    
private:
	double lamda;// forgetting factor
	double p;//the inverse covariance matrix at step n
	double throughput;//measured througput at step n
	double targetThroughput;
	double capacity;//measured capacity at step n
	double targetResponseTime;
	
	double alpha;// the estimated alpha at step n
	double threshold;// a percentile bound for re-estimating alpha-not used
	//NonLinearModel nonLinear;
	double averageArrivalRate;
	double predictedArrivalRate;
	double step;
	double errorValue;
	double regressionCofficient;
	double averageRT;
	
};

#endif /* MODEL_HH */
