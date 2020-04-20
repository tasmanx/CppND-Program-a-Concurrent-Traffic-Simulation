#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // wait for and receive new messages
    std::unique_lock<std::mutex> lck(_mutex);
    _cond.wait(lck, [this] { return !_queue.empty(); });

    // pull new message from the queue
    T msg = std::move(_queue.back());
    _queue.pop_back();

    // return received object
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // add a new message to the queue and send a notification
    std::lock_guard<std::mutex> lck(_mutex);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // start cycleThroughPhases function in a thread using TraficObject class' the thread queue vector
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // set cycleDuration with random number between 4 and 6
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(4, 6);
    int cycleDuration = distr(eng); // seconds

    auto cycleStartTime = std::chrono::system_clock::now();

    while (true)
    {
        // simulate some work to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // measure the time between cycleStartTime and current time
        auto currentCycleDuration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - cycleStartTime).count();

        if (currentCycleDuration >= cycleDuration)
        {
            // toggle the current phase of the traffic light
            if (_currentPhase == TrafficLightPhase::red)
                _currentPhase = TrafficLightPhase::green;
            else
                _currentPhase = TrafficLightPhase::red;

            // send an update to the message queue
            _messageQueue.send(std::move(_currentPhase));

            // traffic light cycle phase ended - reset cycleStartTime
            cycleStartTime = std::chrono::system_clock::now();
        }
    }
}
