#include "embedded2014.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include <stdio.h>

int err;
char gp_buffer[128];
OS_EVENT *mutex_gpBuffer;

//char messageBuffer[E2014_NUM_OF_MBOX][E2014_MBOX_SIZE];
//OS_EVENT *msgBox[E2014_NUM_OF_MBOX];

OS_EVENT *msgQ[E2014_NUM_OF_MSGQ];
char *msgBuffer[E2014_NUM_OF_MSGQ][E2014_MSGQ_SIZE];

//void initMemBox()
//{
//    int i = 0;
//    for(i=0; i<E2014_NUM_OF_MBOX; i++)
//        msgBox[i] = OSMBoxCreate(messagebuffer[i]);
//}

void E2014_initAll()
{
    E2014_initMutex();
    E2014_initMsgQ();
    E2014_initUART1();
    E2014_initInterrupt_USART1();
}

void E2014_initMutex()
{
    mutex_gpBuffer = OSMutexCreate(1, &err); 
}

void E2014_initMsgQ()
{
    int i;
    for(i=0; i<E2014_NUM_OF_MSGQ; i++)
        msgQ[i] = OSQCreate(msgBuffer[i], E2014_MSGQ_SIZE);
}

void E2014_initInterrupt_USART1()
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	//EXTI 레지스터 설정
        /*
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource10);
	EXTI_InitStructure.EXTI_Line = EXTI_Line10
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
        */

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//Clear EXTI Line Pending Bit
	EXTI_ClearITPendingBit(EXTI_Line10);

	//Enable the Key EXTI Line Interrupt
	//NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
}


void E2014_initUART1(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    //USART1 Clock 할당
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    // APB2 Clock enable for USART(GPIOA9, A10)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* Configure the USART1_Tx as Alternate function Push-Pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure the USART1_Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure the USART1 */
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl =
                    USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    // Rx Not empty interrupt enable
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    /* Enable the USART1 */
    USART_Cmd(USART1, ENABLE);
}

void USART1_IRQHandler(void)
{
    char receive_data;
    
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        receive_data = USART_ReceiveData(USART1) & 0xFF;
        USART_SendData(USART1, receive_data);
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);

        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
    
    OSMutexPend(mutex_gpBuffer, 1, &err);
    sprintf(gp_buffer, "%c", receive_data);
    OSMutexPost(mutex_gpBuffer);
    OSMboxPost(msgBox[0], gp_buffer);
}

void E2014_Task_PrintPeriodicSignal(void *p_arg)
{
    for( ; ; )
    {
        USART_SendData(USART1, '*');
        OSTimeDlyHMSM(0, 0, 1, 0);
    }    
}

void E2014_Task_PrintUSART1()
{
    
}
