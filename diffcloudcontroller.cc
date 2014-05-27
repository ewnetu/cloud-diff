#include <arpa/inet.h>
#include <boost/program_options.hpp>
#include <map>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include<pthread.h>
#include "VirtualManager.hh"
//#include "ApplicationManager.hh"
#include "PriorityManager.hh"
#include "Model.hh"

#define MAX_MESSAGE_SIZE 1024 /*< maximum number of bytes accepted through UDP */

///....variables...///
PriorityManager mgr;
std::string config="config.json";

int listenPort;
int init=0;
/* Class to chat with hypervisor */
VirtualManager vmm;

/* Set up some passive data structures */
std::map<uint32_t, std::string> ipToVmCache;
std::map<std::string, double> vmToPerformance;
std::map<std::string, double> vmToVp;
std::string virtualBridge;
//....................//    

double inline now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_usec / 1000000 + tv.tv_sec;
}

bool processMessage(int s,
	VirtualManager &vmm,
	const std::string virtualBridge,
	std::map<uint32_t, std::string> &ipToVmCache,
	std::map<std::string, double> &vmToPerformance)
{
	char buf[MAX_MESSAGE_SIZE + 1 /* for null terminator */];

	struct sockaddr_in sin;
	socklen_t sinlen = sizeof(sin);
	int len = recvfrom(s, buf, 1024, 0, (sockaddr*)&sin, &sinlen);

	if (len <= 0) {
		perror("Error, recvfrom() failed");
		return false;
	}

	/* "Convert" buffer to C string */
	buf[len] = 0;

	/*
	 * Retrieve VM's name
	 */
	std::string vmName;

	/* First, check cache */
	if (ipToVmCache.count(sin.sin_addr.s_addr) == 1) {
		vmName = ipToVmCache[sin.sin_addr.s_addr];
	}
	else {
		/* IP not in cache */
		/* Map IP to MAC address by doing an ARP query */
		struct arpreq arpreq;
		arpreq.arp_flags = 0;
		memcpy(&arpreq.arp_pa, &sin, sizeof(struct sockaddr_in));
		strcpy(arpreq.arp_dev, virtualBridge.c_str());
		if (ioctl(s, SIOCGARP, &arpreq) == -1) {
			perror("Error, unable to retrieve MAC address");
			return false;
		}
		const unsigned char *mac = (unsigned char *)&arpreq.arp_ha.sa_data;

		/* Then map the MAC to a VM using the hypervisor-provided functions */
		vmName = vmm.lookUpVmByMac(mac);

		/* Update the cache */
		ipToVmCache[sin.sin_addr.s_addr] = vmName;
	}

	//fprintf(stderr, "[%f] got message from %s(%s): %s\n", now(), inet_ntoa(sin.sin_addr), vmName.c_str(), buf);

	/* Store performance data */
	
	//double responseTime=std::max(std::min(atof(buf), 1.0), -1.0);//why is min????????
	const char *start = "start\0";
	if( strcmp(buf,start) == 0 )
	{
		//fprintf(stderr, "[%f] uppppppgot message from %s(%s): %s\n", now(), inet_ntoa(sin.sin_addr), vmName.c_str(), buf);
		mgr.updateArrivalRate(vmName);
	}
	else{
		//fprintf(stderr, "[%f] got message from %s(%s): %s\n", now(), inet_ntoa(sin.sin_addr), vmName.c_str(), buf);
	double responseTime=atof(buf);// std::max(std::min(atof(buf), 1.0), -1.0);
	vmToPerformance[vmName] = responseTime;
	
	//this is for throughput(change by ewnetu)
	//std::cout<<"vm nam"<<vmName<<"\n";
	mgr.updateMeasuredPerformance(vmName,1,responseTime);
	}
	return true;
}

bool rebalancePlatform(VirtualManager &vmm,
	double platformSize,
	double epsilonRm,
	std::map<std::string, double> &vmToPerformance,
	std::map<std::string, double> &vmToVp)
{
	std::vector<std::string> vms = vmm.listVms();
	vms.erase(vms.begin()); /* leave Dom-0 alone */

	/* Compute sum of matching values */
	double sumFik = 0;
	for (auto vm : vms)
		sumFik += vmToPerformance[vm]; /* zero if vm is not in map, i.e., SL-unaware */

	/* Update caps */
	int numNewVms = 0;
	for (auto vm : vms) {
		if (vmToVp[vm] == 0) /* new VM */ {
			numNewVms++;
			fprintf(stderr, "[%f] vm=%s new\n", now(), vm.c_str());
		}
		else if (vmToPerformance[vm] < 0)
			vmToVp[vm] -= epsilonRm * (vmToPerformance[vm] - vmToVp[vm] * sumFik);
	}

	/* Deal with new VMs */
	for (auto vm : vms) {
		if (vmToVp[vm] == 0) /* new VM */
			vmToVp[vm] = 1.0 / vms.size();
		else
			vmToVp[vm] *= 1.0 * (vms.size() - numNewVms) / vms.size();
	}

	/* Limit vp */
	for (auto vm : vms) {
		if (vmToVp[vm] < 0.125)
			vmToVp[vm] = 0.125;
		else if (vmToVp[vm] > 1)
			vmToVp[vm] = 1;
	}

	/* Rescale to make sure the sum is 1 */
	double sumVp = 0;
	for (auto vm : vms) sumVp += vmToVp[vm];
	for (auto vm : vms) vmToVp[vm] /= sumVp; // XXX: Can sumVp ever be zero?

	/* Apply new caps and report outcome*/
	for (auto vm : vms) {
		double cap = vmToVp[vm] * platformSize;
		fprintf(stderr, "[%f] vm=%s perf=%f vp=%f cap=%f\n",
			now(),
			vm.c_str(),
			vmToPerformance[vm],
			vmToVp[vm],
			cap);
		vmm.setVmCap(vm, cap); /* XXX: We might want to avoid useless changes here */
	}

	return false;
}

//-----------------------------------------------------------------------
void test()
{
	
	std::cout<<"here\n\n";
	mgr.init(config);
	mgr.print();
	Application* app=mgr.getApplication("application5");
	app->print();
	app=mgr.getApplication("application7");
	if(app!=NULL)
		app->print();
}

//UDP for collecting monitoring info
void* monitor(void*arg)
{
	/* Create UDP socket */
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		perror("Fatal error, socket() failed");
		exit(EXIT_FAILURE);
	}
	//increase the buffer size
	int n = 26214400;
	if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n)) == -1) {
		// deal with failure, or ignore if you can live with the default size
		perror("Fatal error, setsockopt() failed");
		exit(EXIT_FAILURE);
	}
	
	/* Start listening */
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(listenPort);
	sa.sin_addr.s_addr = INADDR_ANY;
	if (bind(s, (const sockaddr *)&sa, sizeof(sa)) == -1) {
		perror("Fatal error, bind() failed");
		exit(EXIT_FAILURE);
	}
	
	for (;;) {
	/* Wait for a packet to arrive  */
	int ret = 0; /* as if poll() had timed out */
	struct pollfd fds[1] = {{ s, POLLIN }};
	int waitForMs = (5) * 1000 + 1;
	if (waitForMs > 0)
		ret = poll(fds, 1, waitForMs);
	if (ret == -1) /* why would this ever happen? */
		perror("Error during poll()");
	
	/* Did we receive a packet? If so, process it. */
	if (ret > 0)
		processMessage(s, vmm, virtualBridge, ipToVmCache, vmToPerformance);
	}
}
//-----------------------------------------------------------------------


int main(int argc, char **argv)
{
       // Model test;
    //test.getNewAlpha();
    //return 0;
	//test();
	//return 0;
	//int flopctest();
	//flopctest();
	//return 0;
	/* Parameters to program */
	mgr.init(config);
	//double epsilonRm;
	double controlInterval;
	int nCpus;
	
	int QoS;

	
	/* Parse command-line for QoS type(1-cost-benefit QoS vs 2-strict QoS ) control interval and platform size */
	namespace po = boost::program_options;
	po::options_description desc("Performance-Based Differentiated QoS for Cloud applications");
	desc.add_options()
	("help", "produce help message")
	("port", po::value<int>(&listenPort)->default_value(2712), "UDP port to listen on for receiving matching values")
	("QoS", po::value<int>(&QoS)->default_value(1), "use this value for QoS type(1-cost-benefit QoS and 2-strict QoS); see paper for its meaning")
	("ncpus", po::value<int>(&nCpus)->default_value(4), "maximum number of processors available for allocating to VMs")
	("interval", po::value<double>(&controlInterval)->default_value(5), "control interval in seconds")
	("bridge", po::value<std::string>(&virtualBridge)->default_value("virbr0"), "use this (virtual) bridge for IP-to-MAC resolution")
	;
	
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	
	if (vm.count("help")) {
		std::cout << desc << "\n";
		return 1;
	}

	

	
	
	//create thread to listen to monitoring information
	pthread_t tid;
	int err = pthread_create(&(tid), NULL, &monitor, NULL);
	if (err != 0)
		printf("\ncan't create thread :[%s]", strerror(err));
	else
		printf("\n Thread created successfully\n");
	
	pthread_mutex_t mutex;
	if (pthread_mutex_init(&mutex, NULL) != 0)
	{
		printf("\n mutex init failed\n");
		return 1;
	}	
	/*  loop */
	double lastControl = now();
	for (;;) {
		/* Wait for a packet to arrive or the next control interval */
		
		
		/* Should we run the controller? If so, run it. */
		if (now() - lastControl > controlInterval) {
			mgr.print();
			/*if(warmUp==0)//ignore the first measurement since it is usually crap!!!
			{
				mgr.resetMeasuredPerformance();
				vmToPerformance.clear();
				warmUp=1;
				
				
			}*/
			//wait for one more control interval so that the apps are heated-up
			if(init<2 )
			{
				init++;
				lastControl = now();
				continue;
			}
			if(QoS==1)
				mgr.allocate( controlInterval,true);//allocate across class- (Cost-benefit QoS)
			else
				mgr.allocate( controlInterval,false);// Strict QoS
			mgr.setCapToVM(vmm);
			
			
			//rebalancePlatform(vmm, 100 * nCpus, epsilonRm, vmToPerformance, vmToVp);
			mgr.save(controlInterval);
			
			/* Require VMs to report performance before next control interval */
			
			 pthread_mutex_lock(&mutex);
			mgr.resetMeasuredPerformance();
			pthread_mutex_unlock(&mutex);
			vmToPerformance.clear();
			lastControl = now();
		}
	}
}


///------testing flopc++____________

#include "flopc.hpp"
#include "OsiCbcSolverInterface.hpp"
#include "OsiClpSolverInterface.hpp"

using namespace flopc;

int minimize()
{
	enum  {seattle, sandiego, numS}; 
    enum  {newyork, chicago, topeka,numD};
	MP_model::getDefaultModel().setSolver(new OsiCbcSolverInterface);
	MP_model::getDefaultModel().verbose();
    MP_set S(numS);          // Sources 
    MP_set D(numD);          // Destinations 
    MP_subset<2> Link(S,D);  // Transportation links (sparse subset of S*D)
	
    Link.insert(seattle,newyork);
    Link.insert(seattle,chicago);
    Link.insert(sandiego,chicago);
    Link.insert(sandiego,topeka);
	
    MP_data SUPPLY(S);
    MP_data DEMAND(D);
	
    SUPPLY(seattle)=350;  SUPPLY(sandiego)=600;
    DEMAND(newyork)=325;  DEMAND(chicago)=300;  DEMAND(topeka)=275;
    
    MP_data COST(Link);
	
    COST(Link(seattle,newyork)) = 2.5;  
    COST(Link(seattle,chicago)) = 1.7;  
    COST(Link(sandiego,chicago))= 1.8;  
    COST(Link(sandiego,topeka)) = 1.4;
	
    COST(Link) = 90 * COST(Link) / 1000.0;
    
    MP_variable x(Link);
	
    MP_constraint supply(S);
    MP_constraint demand(D);  
	
    supply(S) =  sum( Link(S,D), x(Link) ) <= SUPPLY(S);
    demand(D) =  sum( Link(S,D), x(Link) ) >= DEMAND(D);
    
    minimize( sum(Link, COST(Link)*x(Link)) );
    
    x.display("Optimal solution:");
	return 0;
	//MP_model &model = MP_model::getDefaultModel();
	//model.setSolver(new OsiCbcSolverInterface);//OsiCbcSolverInterface);//new OSI_SOLVER_INTERFACE);
	//MP_model::getDefaultModel().setSolver(new OsiCbcSolverInterface);
	//variables
	MP_variable Xb, Xc;
	// Objective - deﬁne it as an expression; later say it should be minimized
	
	//MP_domain d=25*Xb;
	//MP_expression objFunc=sum(25*Xb, 30*Xc);
	
	// Constraints
	MP_constraint c;
	Xb.integer();
	Xc.integer();
	Xc.upperLimit(3)=4000;
	Xb.upperLimit(3)=6000;
	//c=sum((1/200)*Xb +(1/140)*Xc) <=40;
	//MP_constraint MaxTonsXb()=Xb<= 6000;
	//MP_constraint MaxTonsXc()=Xc<= 4000;
	//MP_constraint MaxHours()=(1/200)*Xb +(1/140)*Xc<=40;
	//MP_expression F = 25*Xb +30*Xc;
	//m1.maximize(F);
	// Set the objective; attach the model;
	//model.setObjective(objFunc);
	//model.attach();
	
	// Write MPS ﬁle
	 //model->writeMps("investment");
	
	 // Minimize the objective, ie. solve the model;
	//model.minimize();
	//minimize(25*Xb+ 30*Xc);
	Xc.display("Optimal solution:");
	return 0;
}
void test2()
{
	using namespace std;
		MP_model &model = MP_model::getDefaultModel();
		model.setSolver(new OsiCbcSolverInterface(new OsiCbcSolverInterface));
		model.verbose();
		
		enum {iron, nickel, numRaw};
		enum {nuts, bolts, washers, numPrd};
		const int numT = 4;
		
		MP_set 
		prd(numPrd),    // products        / nuts, bolts, washers /
		raw(numRaw),    // raw materials   / iron, nickel /
		TL(numT+1);     // extended t      / 1 * 5 /
		
		MP_subset<1>
		T(TL),     //  periods     / 1 * 4 /
		first(TL), //          / 1     /
		last(TL);  //           /     5 /
		
		T.insert(0);
		T.insert(1);
		T.insert(2);
		T.insert(3);
		
		first.insert(0);
		last.insert(4);
		
		MP_data istock(raw); // initial stock  /  iron  35.8 , nickel  7.32  /
		MP_data scost(raw); // storage cost   /  iron    .03, nickel   .025 /
		MP_data rvalue(raw); //  residual value /  iron    .02, nickel  -.01  /
		
		istock(iron) = 35.8;   istock(nickel) = 7.32;
		scost(iron) =  0.03;  scost(nickel) = 0.025;
		rvalue(iron) =  0.02;  rvalue(nickel) =-0.01;
		
		double  m = 123; //  maximum production 
		
		MP_data units(raw,prd); // raw material inputs to produce a unit of product
		
		double avalue[numRaw][numPrd] =  {{ .79, .83, .92 }, 
			{ .21, .17, .08 }};
		
		units.value(&avalue[0][0]);
		
		MP_data profit(prd,T); // profit
		
		double cvalue[numPrd][numT] = {{ 1.73,  1.8,  1.6,  2.2 },
			{ 1.82,  1.9,  1.7,   .95},
			{ 1.05,  1.1,   .95, 1.33}};
		
		profit.value(&cvalue[0][0]);
		
		MP_variable  x(prd,TL); // production level
		x.setName("X");
		MP_variable  s(raw,TL); // storage at beginning of period
		
		MP_index p,r,t;
		
		MP_constraint  
		limit(T),       // capacity constraint
		balance(raw,TL);  // raw material balance
		
		
		limit(T) =   sum(prd(p), x(p,T)) <= m;
		
		balance(r,TL+1) = 
		s(r,TL+1) == s(r,TL) - sum(prd(p), units(r,p)*x(p,TL));
		
		s.upperLimit(r,0) = istock(r);
		
		
		// MP_model::default_model.verbose();
		
		model.maximize( 
					   sum(T(t), sum(prd(p), profit(p,t)*x(p,t)) -
						   sum(raw(r), scost(r)*s(r,t) )) +
					   sum(raw(r), rvalue(r)*s(r,numT)) 
					   );
		
		assert(model->getNumRows()==14);
		assert(model->getNumCols()==25);
		assert(model->getNumElements()==52);
		assert(model->getObjValue()>=79.3412 && model->getObjValue()<=79.3414);
		
		x.display();
		s.display();
		cout<<"Test ampl passed."<<endl;
	
}
int flopctest() {
	//test2();
	//minimize();
	//return 0;
    enum  {app1,app2,app3,app4,app5,app6,numA}; 
  //  enum  {newyork, chicago, topeka,numD};
	MP_model::getDefaultModel().setSolver(new OsiCbcSolverInterface);//OsiClpSolverInterface
	//MP_model &model = MP_model::getDefaultModel();
	//model.setSolver(new OSI_SOLVER_INTERFACE);
	MP_model::getDefaultModel().verbose();
    MP_set APP(numA);          // Sources 
   // MP_set CAP(1);          // Destinations 
  //  MP_subset<1> Link(APP);  // Transportation links (sparse subset of S*D)
	
   // APP.insert(app1);
   // APP.insert(app2);
   // APP.insert(app3);
    //Link.insert(sandiego,topeka);
	
    //MP_data APPLICATIONS(APP);
    //MP_data DEMAND(D);
	
   // APPLICATIONS(app1)=10; APPLICATIONS(app2)=10;APPLICATIONS(app3)=10; //SUPPLY(sandiego)=600;
   // DEMAND(newyork)=325;  DEMAND(chicago)=300;  DEMAND(topeka)=275;
    
    MP_data COST(APP);
	MP_data target(APP);
	//MP_data CAPACITY(CAP);
    COST(app1) = 80;  
    COST(app2) = 60;  
    COST(app3)= 70;  
	COST(app4) = 90;  
    COST(app5)= 50;  
	COST(app6)= 40; 
	
	
   // COST(Link(sandiego,topeka)) = 1.4;
	//CAPACITY(1)=10;
   // COST(APP) = 1 * COST(APP) / 1.0;
    
    MP_variable core(APP),DIFF(APP);//CONSTANT(APP), 
	//MP_index p;
	MP_constraint  
	//limit(APP),       // capacity constraint
	applications;//balance(raw,TL);  // raw material balance
	target(app1)=100;
	target(app2)=100;
	target(app3)=100;
	target(app4)=100;
	target(app5)=100;
	target(app6)=100;
	double cap=10;
	
	//limit(APP)=sum(APPLICATIONS(APP))-10;
	/*limit(app1)=x(app1)>=1;
	limit(app2)=x(app2)>=1;
	limit(app3)=x(app3)>=1;
	limit(app4)=x(app4)>=1.5;
	limit(app5)=x(app5)>=2;
	limit(app6)=x(app6)>=1;*/
	//x.lowerLimit(app6)=2;
	/*CONSTANT.lowerLimit(app1)=1;
	CONSTANT.upperLimit(app1)=1;
	CONSTANT.lowerLimit(app2)=1;
	CONSTANT.lowerLimit(app2)=1;
	CONSTANT.upperLimit(app3)=1;
	CONSTANT.lowerLimit(app3)=1;
	CONSTANT.lowerLimit(app4)=1;
	CONSTANT.upperLimit(app4)=1;
	CONSTANT.lowerLimit(app5)=1;
	CONSTANT.lowerLimit(app5)=1;
	CONSTANT.upperLimit(app6)=1;
	CONSTANT.lowerLimit(app6)=1;*/
	
	
	core.lowerLimit(app1)=1;
	core.lowerLimit(app2)=1;
	core.lowerLimit(app3)=1;
	core.lowerLimit(app4)=1;
	core.lowerLimit(app5)=1;
	core.lowerLimit(app6)=2;
    //MP_constraint demand(D);  
	//limit(app1)=APP(app1)>=1;
	//limit(app2)=APP(app2)>=1;
	//limit(app3)=APP(app3)>=1;
	DIFF.lowerLimit(APP) = 0.0;
	MP_constraint abs_x_linctr_a(APP);
	MP_constraint abs_x_linctr_b(APP);
	//abs_x_linctr_a(APP) = target(APP)*CONSTANT(APP)-(COST(APP)*core(APP)) <= DIFF(APP);
	//abs_x_linctr_b(APP) = -1.0*(target(APP)*CONSTANT(APP)-(COST(APP)*core(APP))) <= DIFF(APP);
	abs_x_linctr_a(APP) = target(APP)-(COST(APP)*core(APP)) <= DIFF(APP);
	abs_x_linctr_b(APP) = -1.0*(target(APP)-(COST(APP)*core(APP))) <= DIFF(APP);
   applications =sum(APP, core(APP))<=cap;
	
	//DIFF(APP)= flopc::abs(target(APP)*CONSTANT(APP)-(COST(APP)*core(APP)));
	//applications(APP) =  sum( APP, x(APP) )<=cap;
   // demand(D) =  sum( Link(S,D), x(Link) ) >= DEMAND(D);
    //T-((T/tm)*a*ci)
    minimize( sum(APP,DIFF(APP)));
    
    core.display("Optimal solution:");
	std::cout<<"VALUE"<<core.level(0);
	 //CONSTANT.display("Optimal solution:");
	return 0;
}

/*
static MP_expression lp_objective_absolute(
					  MP_variable& x,
					  MP_data& weight,
					  MP_model& lp//=MP_model::getDefaultModel()
			 ,
				  const string& prefix//="abs_"
	) {
	if (x.size() > 0) {
		if (x.getName().empty()) {
			string message;
			message += "decision variable(s) or affine expression(s) is/are not named";
			throw BmtException(message.c_str());
		}
		MP_set D(x.size());
		MP_variable abs_x(D);
		abs_x.setName(prefix + x.getName());
		abs_x.lowerLimit(D) = 0.0;
		MP_constraint abs_x_linctr_a(D);
		MP_constraint abs_x_linctr_b(D);
		abs_x_linctr_a(D) = x(D) <= abs_x(D);
		abs_x_linctr_b(D) = -1.0*x(D) <= abs_x(D);
		
		lp.add(abs_x_linctr_a);
		lp.add(abs_x_linctr_b);
		
		return sum(D, weight(D)*abs_x(D));
	} else {
		return MP_expression();
	}
}
	 MP_expression objective;
	 MP_data UNIT_WEIGHT_X(T);
	 UNIT_WEIGHT_X(T) = 1.0;
	 objective = lp_objective_absolute(dispatch_period_volume, UNIT_WEIGHT_X, lp);
	 */

