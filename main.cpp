#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <ctime>

class Car {
    private:
        int posX, posY, busyTime;
    public:
        Car() {};
        Car(int posX, int posY);
        int getPosX() { return posX; };
        int getPosY() { return posY; };
        void setNewPosX(int posX) { this->posX = posX; };
        void setNewPosY(int posY) { this->posY = posY; };
        void setBusyTime(int busyTime) { this->busyTime = busyTime; };
        void reduceBusyTimeByOne();
        bool isBusy();
};

Car::Car(int posX, int posY) {
    this->posX = posX;
    this->posY = posY;
    this->busyTime = 0;
}

// only to easy maintainability, this is a kind of facade for busyTime higher than zero (i.e., car is busy)
bool Car::isBusy() {
    if (busyTime > 0) {
        return true;
    } else {
        return false;
    }
}

// each time is called, it will reduce busy time by one unit
// then it will return the remaining busy time in units of time
// when busy time returned is 0, the car is available again
void Car::reduceBusyTimeByOne() {
    if (isBusy()) {
        busyTime--;
    }
}

class Request {
    int initPosX, initPosY, destPosX, destPosY, time;
    public:
        Request(int initPosX, int initPosY, int destPosX, int destPosY, int time);
        int getInitPosX() { return initPosX; };
        int getInitPosY() { return initPosY; };
        int getDestPosX() { return destPosX; };
        int getDestPosY() { return destPosY; };
        int getTime() { return time; };
};

Request::Request(int initPosX, int initPosY, int destPosX, int destPosY, int time) {
    this->initPosX = initPosX;
    this->initPosY = initPosY;
    this->destPosX = destPosX;
    this->destPosY = destPosY;
    this->time = time;
}

class RequestsHelper {
    public:
        static std::vector<Request> getRequestsAtTime(std::vector<Request> requests, int time);
};

std::vector<Request> RequestsHelper::getRequestsAtTime(std::vector<Request> requests, int time) {
    std::vector<Request> requestsAtTime;

    for (Request request:requests) {
        // if the request is 'in the present', we add it to the requestAtTime vector to be returned
        if (request.getTime() == time) {
            requestsAtTime.push_back(request);
        }
        // if there is one request 'in the future', we do not need to continue the search for requests at this time
        if (request.getTime() > time) {
            return requestsAtTime;
        }
    }
    return requestsAtTime;
}

class DistanceHelper {
    public:
        // we actually don't need an object, thus the getDistance method is a static one
        static int getDistanceToRequest(Car car, Request request);
        static int getDistanceToDestination(Car car, Request request);
        static int getDistanceFromRequestToDestination(Request request);
        static int getNearestAvailableCarIndex(std::vector<Car> cars, Request request);
        static int getNearestToDestinationAvailableCarIndex(std::vector<Car> cars, Request request);
};

// for now, objects model is accurate. however, when using effort functions this will need an update
// as we cannot get the effort to reach some place yet
int DistanceHelper::getDistanceToRequest(Car car, Request request) {
    return (abs(car.getPosX() - request.getInitPosX()) + abs(car.getPosY() - request.getInitPosY()));
}

int DistanceHelper::getDistanceToDestination(Car car, Request request) {
    return (abs(car.getPosX() - request.getDestPosX()) + abs(car.getPosY() - request.getDestPosY()));
}

int DistanceHelper::getDistanceFromRequestToDestination(Request request) {
    return (abs(request.getDestPosX() - request.getInitPosX()) + abs(request.getDestPosY() - request.getInitPosY()));
}

int DistanceHelper::getNearestAvailableCarIndex(std::vector<Car> cars, Request request) {
    // initial distance to be compared (assumed to be the minimum, at first)
    // we also assume that the first car (element of the vector) is the nearest to a request
    int distance = DistanceHelper::getDistanceToRequest(cars.front(), request);
    int carVectorIndex = 0;
    int nearestCarVectorIndex = 0;

    for (Car car:cars) {
        // it will consider only available cars (i.e., busyTime = 0)
        if ((!car.isBusy()) && (DistanceHelper::getDistanceToRequest(car, request) < distance)) {
            nearestCarVectorIndex = carVectorIndex;
        }
        carVectorIndex++;
    }

    // if any car is nearer, the returned index is 0
    return nearestCarVectorIndex;
}

int DistanceHelper::getNearestToDestinationAvailableCarIndex(std::vector<Car> cars, Request request) {
    // here we talk about cost, instead of distances
    // the goal here is to minimize the cost and distance to request
    // the cost is indicated by the distance to the destination (i.e., an indicator of how much
    // does the car need to move to attend the request)
    int cost =
            DistanceHelper::getDistanceToDestination(cars.front(), request) +
            DistanceHelper::getDistanceToRequest(cars.front(), request);
    int carVectorIndex = 0;
    int nearestCarToDestinationVectorIndex = 0;

    for (Car car:cars) {
        // it will consider only available cars (i.e., busyTime = 0)
        if ((!car.isBusy()) &&
            (DistanceHelper::getDistanceToDestination(car, request) +
             DistanceHelper::getDistanceToRequest(car, request) < cost)) {
            nearestCarToDestinationVectorIndex = carVectorIndex;
        }
        carVectorIndex++;
    }

    // if any car is nearer, the returned index is 0
    return nearestCarToDestinationVectorIndex;
}

class CarsFileReader {
    public:
        static std::vector<Car> loadCars(std::string file);
};

std::vector<Car> CarsFileReader::loadCars(std::string file) {
    int readPosX, readPosY;
    std::vector<Car> cars;
    std::ifstream inFile;
    inFile.open(file);

    if (inFile.fail()) {
        std::cerr << "Could not open Cars File!" << std::endl;
        inFile.close();
        return cars;
    }

    while (inFile >> readPosX >> readPosY) {
        cars.push_back(Car(readPosX, readPosY));
    }

    inFile.close();
    return cars;
}

class RequestsFileReader {
    public:
        static std::vector<Request> loadRequests(std::string file);
};

std::vector<Request> RequestsFileReader::loadRequests(std::string file) {
    int readInitPosX, readInitPosY, readDestPosX, readDestPosY, time;
    char skip1, skip2;
    std::vector<Request> requests;
    std::ifstream inFile;
    inFile.open(file);

    if (inFile.fail()) {
        std::cerr << "Could not open Requests File!" << std::endl;
        inFile.close();
        return requests;
    }

    // easiest way to skip a character (the '-' in requests file) is to use a dummy variable ('skip')
    while (inFile >> readInitPosX >> readInitPosY >> skip1 >> readDestPosX >> readDestPosY >> skip2 >> time) {
        requests.push_back(Request(readInitPosX, readInitPosY, readDestPosX, readDestPosY, time));
    }

    inFile.close();
    return requests;
}

class HarmonicHelper {
    public:
        static float calculateProbability(Car car, std::vector<Car> cars, Request request);
};

float HarmonicHelper::calculateProbability(Car car, std::vector<Car> cars, Request request) {
    float distanceFromAllCarsToRequest = 0;
    int distanceFromCarToRequest;

    distanceFromCarToRequest = DistanceHelper::getDistanceToRequest(car, request);

    for (Car carElement:cars) {
            distanceFromAllCarsToRequest += (1.0 / (float) DistanceHelper::getDistanceToRequest(carElement, request));
    }

    return (1.0/(float)distanceFromCarToRequest) / (float) distanceFromAllCarsToRequest;

}

class CarAllocation {
    public:
        static void greedy(std::vector<Car> cars, std::vector<Request> requests);
        static void workFunction(std::vector<Car> cars, std::vector<Request> requests);
        static void harmonic(std::vector<Car> cars, std::vector<Request> requests);
};


void CarAllocation::greedy(std::vector<Car> cars, std::vector<Request> requests) {
    int time = 0, distanceTraveledByAllCars = 0;
    std::ofstream myfile ("greedy_results.dat");

    while (time < 12000) {
        // we get all requests arrived at time = time
        std::vector<Request> requestsAtTime = RequestsHelper::getRequestsAtTime(requests, time);

        for (Request requestAtTime:requestsAtTime) {
            int nearestCarIndex = DistanceHelper::getNearestAvailableCarIndex(cars, requestAtTime);
            cars.at(nearestCarIndex).setBusyTime(
                    DistanceHelper::getDistanceToRequest(cars.at(nearestCarIndex), requestAtTime) +
                    DistanceHelper::getDistanceFromRequestToDestination(requestAtTime)
            );
            cars.at(nearestCarIndex).setNewPosX(requestAtTime.getDestPosX());
            cars.at(nearestCarIndex).setNewPosY(requestAtTime.getDestPosY());
        }


        // time advancement: one unit
        time++;

        // IMPORTANT: we need to call a reference (for that reason we use &) because we need to decrease the
        // number of time in the original vector object and not in the copy
        for (Car& car:cars) {
            if (car.isBusy()) {
                distanceTraveledByAllCars++;
                car.reduceBusyTimeByOne();
            }
        }

        if (myfile.is_open())
        {
            myfile << time << " " << distanceTraveledByAllCars << "\n";

        }
    }
}

void CarAllocation::harmonic(std::vector<Car> cars, std::vector<Request> requests) {
    int time = 0, distanceTraveledByAllCars = 0;
    std::ofstream myfile ("harmonic_results.dat");

    while (time < 12000) {
        // we get all requests arrived at time = time
        std::vector<Request> requestsAtTime = RequestsHelper::getRequestsAtTime(requests, time);

        for (Request requestAtTime:requestsAtTime) {
            int currentCarIndex = 0;
            int chosenCarIndex = 0;

            srand(std::time(NULL));
            for (Car car:cars) {
                if ((!car.isBusy())&& ((rand()%RAND_MAX/(float)(RAND_MAX-1))) - HarmonicHelper::calculateProbability(car, cars, requestAtTime) <= 0) {
                    chosenCarIndex = currentCarIndex;
                    break;
                }
                currentCarIndex++;
            }



            cars.at(chosenCarIndex).setBusyTime(
                    DistanceHelper::getDistanceToRequest(cars.at(chosenCarIndex), requestAtTime) +
                    DistanceHelper::getDistanceFromRequestToDestination(requestAtTime)
            );
            cars.at(chosenCarIndex).setNewPosX(requestAtTime.getDestPosX());
            cars.at(chosenCarIndex).setNewPosY(requestAtTime.getDestPosY());
        }


        // time advancement: one unit
        time++;

        // IMPORTANT: we need to call a reference (for that reason we use &) because we need to decrease the
        // number of time in the original vector object and not in the copy
        for (Car& car:cars) {
            if (car.isBusy()) {
                distanceTraveledByAllCars++;
                car.reduceBusyTimeByOne();
            }
        }

        if (myfile.is_open())
        {
            myfile << time << " " << distanceTraveledByAllCars << "\n";

        }
    }
}

void CarAllocation::workFunction(std::vector<Car> cars, std::vector<Request> requests) {
    int time = 0, distanceTraveledByAllCars = 0;
    std::ofstream myfile ("workfunction_results.dat");

    while (time < 12000) {
        // we get all requests arrived at time = time
        std::vector<Request> requestsAtTime = RequestsHelper::getRequestsAtTime(requests, time);

        for (Request requestAtTime:requestsAtTime) {
            int nearestCarIndex = DistanceHelper::getNearestToDestinationAvailableCarIndex(cars, requestAtTime);
            cars.at(nearestCarIndex).setBusyTime(
                    DistanceHelper::getDistanceToRequest(cars.at(nearestCarIndex), requestAtTime) +
                    DistanceHelper::getDistanceFromRequestToDestination(requestAtTime)
            );
            cars.at(nearestCarIndex).setNewPosX(requestAtTime.getDestPosX());
            cars.at(nearestCarIndex).setNewPosY(requestAtTime.getDestPosY());
        }


        // time advancement: one unit
        time++;

        // IMPORTANT: we need to call a reference (for that reason we use &) because we need to decrease the
        // number of time in the original vector object and not in the copy
        for (Car& car:cars) {
            if (car.isBusy()) {
                distanceTraveledByAllCars++;
                car.reduceBusyTimeByOne();
            }
        }

        if (myfile.is_open())
        {
            myfile << time << " " << distanceTraveledByAllCars << "\n";

        }
    }

}


int main() {
    std::vector<Car> cars = CarsFileReader::loadCars("noche.dat");
    std::vector<Request> requests = RequestsFileReader::loadRequests("requests_1.dat");

    /*
      * set here the algorithm
      * allowable choices are:
      * |---------------|
      * |greedy()       |
      * |workFunction() |
      * |harmonic()     |
      * |---------------|
      *
      *
    */

    CarAllocation::greedy(cars, requests);

    return 0;
}