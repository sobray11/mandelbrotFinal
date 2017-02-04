//
//  TSQ.hpp
//  mandelbrotFinal
//
//  Created by Scott Obray on 2/3/17.
//  Copyright © 2017 Scott Obray. All rights reserved.
//

#ifndef TSQ_hpp
#define TSQ_hpp

#include <stdio.h>
#include <queue>
#include <mutex>

template<typename T>
class TSQ
{

    
public:
    void enqueue(T t)
    {
        std::lock_guard<std::mutex> l(m);
        
        q.push(t);
    }
    bool dequeue(T& res)
    {
        std::lock_guard<std::mutex> l(m);
        if (q.empty()) return false;
        res = q.front();
        q.pop();
        return true;
    }
    bool empty()
    {
        if (q.empty())
        {
            return true;
        }
        return false;
    }
    
private:
    std::queue<T> q;
    std::mutex m;
};
//Start timer
//put worok on queue
//check for done
//stop timer
//Global variable (number of tasks increment and decrement for each task)

#endif /* TSQ_hpp */
