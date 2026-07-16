#include "kernel/TaskManager.h"

TaskManager::TaskManager()
{
    taskCount = 0;
}

bool TaskManager::addTask(Task* task)
{
    if(taskCount >= MAX_TASKS)
    {
        return false;
    }

    tasks[taskCount++] = task;

    return true;
}

Task* TaskManager::getTask(uint8_t index)
{
    if(index >= taskCount)
    {
        return nullptr;
    }

    return tasks[index];
}

uint8_t TaskManager::getTaskCount() const
{
    return taskCount;
}