// main.cpp : Defines the entry point for the console application.
// My name:
// My student id:

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

// Notes:
//
// I have just learnt, we would use a semaphore on the bridges and taxis to
// control smooth flow of traffic. I have not implemented this as I have not
// learnt it yet.
//
// Copilot, but how would I use a semaphore on the bridges and the taxis?
// Why should I? How should I implement it?
//

Bridge *bridges;
Taxi *taxis;
Island *islands;

class Semaphore {
private:
  int N;
  mutex m;

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
      // we need to wait
      // so we need to unlock the mutex
      // and then wait
      // and then lock the mutex again
      // and then decrement N by nbPeople
      // and then unlock the mutex
      // and then return
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
  int nbPeople;      // People that will take a taxi to travel somewhere
  int peopleDropped; // People that will take a taxi to travel somewhere
public:
  int GetNbPeople() { return nbPeople; }
  int GetNbDroppedPeople() { return peopleDropped; }
  Island() {
    nbPeople = NB_PEOPLE;
    peopleDropped = 0;
  };
  bool GetOnePassenger() {
    // Complete this function. Returns weather a passenger has been picked up
    // (True or false) and reduce the nbPeople count.
    return false;
  }
  void DropOnePassenger() // Complete this function.
  {}
};

class Bridge {
private:
  int origin, dest;

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
    // to be completed
    if (cpt > 0)
      printf("Taxi %d has dropped %d clients on island %d.\n", GetId(), cpt,
             location);
  }

  void CrossBridge() {
    int bridge;
    GetNewLocationAndBridge(location, bridge);
    // Get the right to cross the bridge (to be completed)
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
  // this function creates a thread for each taxi, on our island and then joins
  // them all together, the thread function is TaxiThread, which is defined
  // above the thread function is passed the taxi id, which is used to index
  // into the taxis array
  //
  // the thread function is defined above, and it loops until all the people
  // have been dropped off
  // the thread function calls GetPassengers, CrossBridge, and DropPassengers
  // in that order
  // GetPassengers picks up people from the islands
  // CrossBridge moves the taxi to a new island
  // DropPassengers drops off people on the new islands
  // thank you for coming to my ted talk
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
