/*
 *  HashFunctions.hh
 *  
 *
 *  Created by Ewnetu Bayuh on 11/9/13.
 *  Copyright 2013 Umeå. All rights reserved.
 *
 */

#ifndef _HASHFUNCTIONS_HH
#define _HASHFUNCTIONS_HH

#include <sstream>
#include <string>

#include <hash_map>
//#include<tr1/unordered_map>
class Application;

// extends the std namespace so that == is can be used for Application
/*namespace std {
	bool operator==(const Application& app1, const Application& app2) {
		return app1.getName().compare(app2.getName()) == 0;
	}
}

class MyHasher {
public:
	size_t operator()(const Application &app) const
	{
		return h(app.getName().c_str());
	};
	
private:
	__gnu_cxx::hash<char*> h;
};



class MyComparator
	{
	public:
		bool operator()(const  Application& app1, const  Application& app2) const {
			return app1.getName()==app2.getName();
		}
	};*/

// extends the __gnu_cxx namespace with additional hash functions
 namespace __gnu_cxx
{
	
	template <>
	struct hash<std::string> {
        size_t operator() (const std::string& x) const {
			return hash<const char*>()(x.c_str());
			// hash<const char*> already exists
        }
	};

	/*template<> class hash<Application*>
	{
	public:
		size_t operator() (Application* const value) const
		{
			std::stringstream sstr;
			sstr << value;
			std::string str;
			sstr >> str;
			return hash<const char*>()(str.c_str());
		}
	};*/

	
	
}


	/*
	template<> class hash<Application*>
	{
	public:
		size_t operator() (Application* const value) const
		{
			std::stringstream sstr;
			sstr << value;
			std::string str;
			sstr >> str;
			return hash<const char*>()(str.c_str());
		}
	};*/
	

#endif // _HASHFUNCTIONS_HH
