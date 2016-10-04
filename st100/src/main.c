/****************************usart.c*********************************/
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"

/*******************************************************************/
#define BAUDRATE 9600

/*******************************************************************/
GPIO_InitTypeDef port;
USART_InitTypeDef usart;
//���������� ��� �������� ������������ ������
uint8_t usartData[10];
uint16_t button;
//������� ���������� ����
uint16_t usartCounter = 0;
//� ��� ����� ���������� ����, ������� ����� ��������
uint16_t numOfBytes;

/*******************************************************************/
void initAll()
{
    //�������� ������������
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    //���� PA9 � PA10 � ������ �������������� ������� �
    //Rx � Tx USART��
    GPIO_StructInit(&port);
    port.GPIO_Mode = GPIO_Mode_AF_PP;
    port.GPIO_Pin = GPIO_Pin_9;
    port.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &port);

    port.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    port.GPIO_Pin = GPIO_Pin_10;
    port.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &port);

    //��������� USART, ��� ���� ��������� ����������, ����� �������� ������
    USART_StructInit(&usart);
    usart.USART_BaudRate = BAUDRATE;
    USART_Init(USART1, &usart);

    //����� ����� ������ ���� ������
    port.GPIO_Mode = GPIO_Mode_IPD;
    port.GPIO_Pin = GPIO_Pin_2;
    port.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &port);
}

/*******************************************************************/
//��� ���, �������-�������� ������
void setData()
{
    button = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_2);
    if (button == 0)
    {
        usartData[0] = 'N';
        usartData[1] = 'o';
        usartData[2] = 't';
        usartData[3] = 'P';
        usartData[4] = 'r';
        usartData[5] = 'e';
        usartData[6] = 's';
        usartData[7] = 's';
        usartData[8] = 'e';
        usartData[9] = 'd';
        numOfBytes = 10;
    }
    else
    {
        usartData[0] = 'P';
        usartData[1] = 'r';
        usartData[2] = 'e';
        usartData[3] = 's';
        usartData[4] = 's';
        usartData[5] = 'e';
        usartData[6] = 'd';
        numOfBytes = 7;
    }
    usartCounter = 0;
}

/*******************************************************************/
int main()
{
    __enable_irq ();
    initAll();
    //�������� ���������� �� ������ ����� � �� ��������� ��������
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART1, USART_IT_TC, ENABLE);
    //��������� ��� USART
    USART_Cmd(USART1, ENABLE);
    //������ ��������� ���������, ��� �����
    setData();
    NVIC_EnableIRQ(USART1_IRQn);
    while(1)
    {
        //���������� ��� �� ����, ��������� ����� ��������
        if (usartCounter == numOfBytes)
        {
            setData();
        }
        else __NOP();
    }
}

/*******************************************************************/
//� � ���������� ������ ���� � ����������� �������
void USART1_IRQHandler()
{
    if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
    {
        USART_SendData(USART1, usartData[usartCounter]);
        usartCounter++;
    }
    USART_ClearITPendingBit(USART1, USART_IT_TC);
}

/****************************End of file****************************/
