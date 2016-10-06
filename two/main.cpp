#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>
#include <string>
#include <thread>

#define TEST_MODE
#include "hashmap.h"

using namespace std;


TEST_CASE("insert single")
{
	HashMap<int, string> map;
	map.insert(1, "val1");
	map.insert(2, "val2");
	map.insert(3, "val3");

	string value;
	REQUIRE(map.get(1, value) == true);
	REQUIRE(map.get(2, value) == true);
	REQUIRE(map.get(3, value) == true);
	REQUIRE(map.get(4, value) == false);
    REQUIRE(map.getNumItems() == 3);

}

TEST_CASE("get single")
{
	HashMap<int, string> map;
	map.insert(1, "val1");
	map.insert(2, "val2");
	map.insert(3, "val3");

	string value;
	map.get(1, value);
	REQUIRE(value.compare("val1") == 0);
	map.get(2, value);
	REQUIRE(value.compare("val2") == 0);
	map.get(3, value);
	REQUIRE(value.compare("val3") == 0);
}

TEST_CASE("delete single")
{
    HashMap<int, string> map;
    map.insert(1, "val1");
    map.insert(2, "val2");
    map.insert(3, "val3");
    
    map.remove(2);
    map.remove(1);
    
    string value;
    REQUIRE(map.get(1, value) == false);
    REQUIRE(map.get(2, value) == false);
    REQUIRE(map.get(3, value) == true);
    REQUIRE(map.getNumItems() == 1);
}


TEST_CASE("insert concurent")
{
	HashMap<int, string> map;
	vector<thread> workers;
	for (int i = 0; i < 10; ++i)
	{
		workers.push_back(thread([&map, i](){
            for (int j = 1; j <= 10; ++j)
                map.insert(i*10+j, "val" + to_string(i*10+j));
		}));
	}

    for (int i = 0; i < 10; ++i)
        workers[i].join();
    
    string value;
    for (int i = 1; i <= 100; ++i)
        REQUIRE(map.get(i, value) == true);
    REQUIRE(map.get(101, value) == false);
    
    REQUIRE(map.getNumItems() == 100);

}

TEST_CASE("delete concurent")
{
    HashMap<int, string> map;
    for (int i = 1; i <= 100; ++i)
        map.insert(i, "val" + to_string(i));
    map.insert(101, "val101");

    vector<thread> workers;
    for (int i = 0; i < 10; ++i)
    {
        workers.push_back(thread([&map, i](){
            for (int j = 1; j <= 10; ++j)
                map.remove(i*10+j);
        }));
    }
    
    for (int i = 0; i < 10; ++i)
        workers[i].join();
    
    string value;
    for (int i = 1; i <= 100; ++i)
        REQUIRE(map.get(i, value) == false);
    REQUIRE(map.get(101, value) == true);
    REQUIRE(map.getNumItems() == 1);
}

TEST_CASE("all concurent")
{
    HashMap<int, string> map;
    for (int i = 1; i <= 100; ++i)
        map.insert(i, "val" + to_string(i));
    
    
    vector<thread> workers;
    for (int i = 0; i < 10; ++i)
    {
        workers.push_back(thread([&map, i](){
            for (int j = 1; j <= 10; ++j)
                map.insert(i*10+j, "val" + to_string(i*10+j));
        }));
        workers.push_back(thread([&map, i](){
            string value;
            for (int j = 1; j <= 10; ++j)
                map.get(i*10+j, value);
        }));
        workers.push_back(thread([&map, i](){
            for (int j = 1; j <= 10; ++j)
                map.remove(i*10+j);
        }));
    }
    
    for (int i = 0; i < 30; ++i)
        workers[i].join();
    
}
