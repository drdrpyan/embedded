#ifndef __EMBEDDED_2014_H__
#define __EMBEDDED_2014_H__

#define E2014_NUM_OF_TASK 10
#define E2014_TASK_STK_SIZE 128
//#define E2014_NUM_OF_MBOX_BUFFER 5
//#define E2014_MBOX_BUFFER_SIZE 256
#define E2014_NUM_OF_MSGQ
#define E2014_MSGQ_SIZE
//void E2014_initPeripherals(void);
void E2014_initUART(void);
//void E2014_initADC(void);

void E2014_Task_PrintPeriodicSignal(void *p_arg);
void E2014_Task_PrintEachSensorValue(void *p_arg);


#endif