#include "kernel/Scheduler.h"

Scheduler::Scheduler(){
    count = 0;
    persistentState = nullptr;
}

void Scheduler::addTask(Task* task){
    tasks[count++] = task;
}

void Scheduler::attachState(PersistentState* state){
    persistentState = state;
}

void Scheduler::execute(){
    for(int i=0;i<count;i++){
        Task* task = tasks[i];
        if(task->state != READY){
            continue;
        }
        if((systemTick - task->lastExecution)>= task->period){
            task->state = RUNNING;
            task->run();
            task->lastExecution = systemTick;
            task->executions++;
            if(persistentState != nullptr){
                persistentState->tasks[i] = { static_cast<unsigned char> (i), static_cast<unsigned char> (task->state), task->lastExecution, task->executions };
                /*
                persistentState->tasks[i].id = i;
                persistentState->tasks[i].state = task->state;
                persistentState->tasks[i].lastExecution = task->lastExecution;
                persistentState->tasks[i].executions = task->executions;
                */
            }
            task->state = READY;
        }
    }
}