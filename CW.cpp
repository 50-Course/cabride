// main.cpp : Defines the entry point for the console application.
// My name:
// My student id:

// ------------------------------------------
// Helpful Notes
//
//  - At any given moment, only two taxis can commute on a bridge simultaneously
//    due to its two-track structure
//  - Max. of 4 passengers is allowed on a taxi trip
//
//  - Taxis are located at random initially, and implemented with threads
//  - Taxis pick the next bridge to visit at random
// ------------------------------------------

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

using namespace std;

std::chrono::steady_clock::time_point current_time =
    std::chrono::steady_clock::now();

void InitClock() { current_time = std::chrono::steady_clock::now(); }

void PrintTime_ms(std::string text) {
  std::chrono::steady_clock::time_point new_time =
      std::chrono::steady_clock::now();
  std::cout << text
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   new_time - current_time)
                   .count()
            << " (ms)" << std::endl;
}

#define NB_ISLANDS 50  // number of islands
#define NB_BRIDGES 100 // number of bridges
#define NB_TAXIS 20    // number of taxis
#define NB_PEOPLE 40   // number of people per island

class Bridge;
class Person;
class Taxi;
class Island;
class Semaphore;

Bridge *bridges;
Taxi *taxis;
Island *islands;

class Semaphore {
private:
  int N;
  std::mutex m;

public:
  Semaphore(int nb) { N = nb; };
  void P(int nb = 1) {
    // we want to decrement the semaphore count by nb - that is, we want to
    // take nb from Nj
    // and wait if needed.
    // so we need to lock the mutex, and then decrement N by nbPeople
    // and then unlock the mutex
    std::lock_guard<std::mutex> lock(m);
    N -= nb;
    if (N < 0) {
      m.unlock();
      // wait
      m.lock();
      N -= nb;
      m.unlock();
      return;
    }

  }; // Decrement the semaphore count by nb and wait if needed.
  void V(int nb = 1) {
    // we want to increment the semaphore count by nb - that is, we want to
    // add nb to N
    // so we need to lock the mutex, and then increment N by nbPeople
    // and then unlock the mutex
    std::lock_guard<std::mutex> lock(m);
    N += nb;
    if (N <= 0) {
      m.unlock();
      // signal
      m.lock();
      N += nb;
      m.unlock();
      return;
    }
  }; // Increment the semaphore count by nb.
};

class Island {
private:
  int nbPeople;      // People that will take a taxi to travel somewhere and
                     // have not been dropped on another island
  int peopleDropped; // People that will take a taxi to travel somewhere
                     // and have been dropped on another island
  Semaphore islandMutex;

public:
  int GetNbPeople() { return nbPeople; }
  int GetNbDroppedPeople() { return peopleDropped; }
  Island() {
    nbPeople = NB_PEOPLE;
    peopleDropped = 0;
    islandMutex = Semaphore(1);
  };
  bool GetOnePassenger() {
    // Complete this function. Returns weather a passenger has been picked up
    // (True or false) and reduce the nbPeople count.
    nbPeople -= 1;
    islandMutex.V(1);
    return true;
  }
  void DropOnePassenger() // Complete this function.
  {
    islandMutex.P(1);
    peopleDropped += 1;
    islandMutex.V(1);
  }
};

class Bridge {
private:
  int origin, dest;
  Semaphore bridgeSemaphore;

public:
  Bridge() {
    origin = rand() % NB_ISLANDS;
    do
      dest = rand() % NB_ISLANDS;
    while (dest == origin);
  };
  int GetOrigin() { return origin; };
  int GetDest() { return dest; };
  void SetOrigin(int v) { origin = v; };
  void SetDest(int v) { dest = v; };
};

class Taxi {
private:
  int location; // island location
  int dest[4] = {-1, -1, -1,
                 -1}; // Destination of the people taken; -1 for seat is empty
                      // or the person has been dropped

  int GetId() {
    return this - taxis;
  }; // a hack to get the taxi thread id; Better would be to pass id throught
     // the constructor
public:
  Taxi() { location = rand() % NB_ISLANDS; };

  void GetNewLocationAndBridge(int &location,
                               int &bridge) // find a randomn bridge and returns
                                            // the island on the other side;
  {
    int shift = rand() % NB_BRIDGES;
    for (int i = 0; i < NB_BRIDGES; i++) {
      if (bridges[(i + shift) % NB_BRIDGES].GetOrigin() == location) {
        location = bridges[(i + shift) % NB_BRIDGES].GetDest();
        bridge = (i + shift) % NB_BRIDGES;
        return;
      }
      if (bridges[(i + shift) % NB_BRIDGES].GetDest() == location) {
        location = bridges[(i + shift) % NB_BRIDGES].GetOrigin();
        bridge = (i + shift) % NB_BRIDGES;
        return;
      }
    }
  }

  void GetPassengers() // this function is already completed
  {
    int cpt = 0;
    for (int i = 0; i < 4; i++)
      if ((dest[i] == -1) && (islands[location].GetOnePassenger())) {
        cpt++;
        do
          dest[i] = rand() % NB_ISLANDS; // generating the destination for the
                                         // individual randomly
        while (dest[i] == location);
      }
    if (cpt > 0)
      printf("Taxi %d has picked up %d clients on island %d.\n", GetId(), cpt,
             location);
  }

  void DropPassengers() {
    int cpt = 0;

    // Drop one passenger on the island one at a time
    // If the seat is not empty
    for (int i = 0; i < 4; i++) {
      if (dest[i] != -1) {
        islands[location].DropOnePassenger();
        dest[i] = -1;
        cpt++;
      }
    }

    if (cpt > 0)
      printf("Taxi %d has dropped %d clients on island %d.\n", GetId(), cpt,
             location);
  }

  void CrossBridge() {
    int bridge;
    GetNewLocationAndBridge(location, bridge);

    // To cross the bridge, assuming the seats are not empty
    // Taxi request access to cross the bridge using semaphore
    // release bridge access back to thread pool upon hitting a new destination
    bridges[bridge].bridgeSemaphore.P(2);
    std::this_thread::sleep_for(std::chrono::seconds(
        2)); // assume it takes two seconds to cross the bridge
    printf("Taxi %d has picked up %d clients on island %d.\n", GetId(), cpt,
           location);
    location = bridges[bridge].GetDest();
    bridges[bridges].bridgeSemaphore.V(2);
  }
};

// code for running the taxis
// Comment here on mutual exclusion and the condition
bool NotEnd() // this function is already completed
{
  int sum = 0;
  for (int i = 0; i < NB_ISLANDS; i++)
    sum += islands[i].GetNbDroppedPeople();
  return sum != NB_PEOPLE * NB_ISLANDS;
}

void TaxiThread(int id) // this function is already completed
{
  while (NotEnd()) {
    taxis[id].GetPassengers();
    taxis[id].CrossBridge();
    taxis[id].DropPassengers();
  }
}

void RunTaxisUntilWorkIsDone() // this function is already completed
{
  std::thread taxis[NB_TAXIS];
  for (int i = 0; i < NB_TAXIS; i++)
    taxis[i] = std::thread(TaxiThread, i);
  for (int i = 0; i < NB_TAXIS; i++)
    taxis[i].join();
}

// end of code for running taxis

void Init() {
  bridges = new Bridge[NB_BRIDGES];
  for (int i = 0; i < NB_ISLANDS;
       i++) // Ensuring at least one path to all islands
  {
    bridges[i].SetOrigin(i);
    bridges[i].SetDest((i + 1) % NB_ISLANDS);
  }
  islands = new Island[NB_ISLANDS];
  taxis = new Taxi[NB_TAXIS];
}

void DeleteResources() {
  delete[] bridges;
  delete[] taxis;
  delete[] islands;
}

int main(int argc, char *argv[]) {
  Init();
  InitClock();
  RunTaxisUntilWorkIsDone();
  printf("Taxis have completed!\n ");
  PrintTime_ms("Taxi time multithreaded:");
  DeleteResources();
  return 0;
}
