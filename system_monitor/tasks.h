#ifndef TASKS_H
#define TASKS_H

typedef struct
{
    int tasks;
    int threads;
    int kernel_threads;
    int running;
} TaskInfo;

int read_tasks(TaskInfo *info);

#endif