#pragma once

#include <future>
#include <borealis.hpp>
#include <queue>
#include <vector>


class Timer{
    static std::priority_queue<Timer> timers;

    public:
        Timer(int interval, std::function<void(int64_t)> callback):interval(interval),callback(callback){
            this->update();
        }
        static void setTimeout(Timer time){
            Timer::timers.push(time);
        }
        static void mainLoop(){
            int64_t current = cpu_features_get_time_usec() / 1000;
            while(!Timer::timers.empty()){
                if(current >  Timer::timers.top().end){
                    Timer::timers.top().callback( Timer::timers.top().end);
                    Timer::timers.pop();
                } else {
                    break;
                }
            }
        }
        bool operator > (const Timer time) const {
            return time.end > this->end;
        }
        bool operator < (const Timer time) const {
            return time.end < this->end;
        }
        void update() {
            this->start = cpu_features_get_time_usec() / 1000;
            this->end = this->start + this->interval;
        }
        int64_t start;
        int64_t interval;
        int64_t end;
        std::function<void(int64_t)> callback;
};

std::priority_queue<Timer> Timer::timers;
