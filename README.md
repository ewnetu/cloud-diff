

Cloud-Diff Resource Manager
==============================
This project contains source code associated to the following article:

Ewnetu Bayuh Lakew, Cristian Klein, Francisco Hernandez-Rodriguez, and Erik Elmroth, Performance-Based Service Differentiation in Clouds, In Proceedings of the 15th IEEE/ACM International Symposium on Cluster, Cloud, and Grid Computing (CCGrid 2015), IEEE Computer Society, pp. 505-514, 2015.

Abstract
==========
Cloud capacity management is challenging due to the continuous and unpredictable nature of the runtime changes in the services’ capacity demand and the capacity availability. Moreover, some services are more important than others. Capacity needs to be provisioned in a self-adaptive manner to accommodate these changes and services relative importance. However, a self-adaptive system has to be carefully designed in order not to compromise the performance of services as well as not to over-commit capacity.

In this paper we present performance models for two Key Performance Indicators (KPIs) – throughput and response time – so as to allocate capacity that meets individual services’ performance and avoid capacity over-commit. During capacity shortage, we propose two service differentiation schemes that dynamically adjust capacity to individual services with different service levels in a manner that minimizes services’ performance degradation. To evaluate our solution, we have carried out an extensive set of experiments using different services – interactive as well as non-interactive – and workload generators that are based on open and closed system models. The experimental results demonstrate that our solution provides guaranteed and differentiation services depending on the operating environment and the workload dynamics.

Usage
==========
To conduct the experiments, we used Ubuntu 12.04.2 LTS. Other software has been installed from the official repositories, specifically:

Linux 3.2.0

Xen 4.1.2

GNU compiler collection 4.6.3

GNU make 3.81

libvirt 0.9.8

Moreover, the following libraries were used

Boost C++ libraries 1.48

[FLoPC++-1.1.4](https://projects.coin-or.org/FlopC++)

[jsoncpp-src-0.5.0](http://sourceforge.net/projects/jsoncpp/)

You need to configure about each service under config.json file

To run non-strict service differentiation (more setting is possible if the default is not good enough):

./diffcloudcontroller

To run strict service differentiation:

./diffcloudcontroller --QoS 2
