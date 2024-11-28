#include "uart.h"

/* UART2 Initialization */
void Uart_Init(USART_TypeDef* uart, uint32_t baudrate)
{
    uint8_t frac;
    uint16_t integ;
    float UARTDIV;

    if (uart == USART1)
    {
        // Enable GPIOA clock
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

        // Configure PA9 (TX) as Alternate Function, High Speed, Push-Pull, No Pull-Up/Down
        GPIOA->MODER &= ~(3 << (9 * 2)); // Clear bits
        GPIOA->MODER |= (2 << (9 * 2)); // Set Alternate Function
        GPIOA->OTYPER &= ~(1 << 9);     // Push-Pull
        GPIOA->OSPEEDR |= (3 << (9 * 2)); // High speed
        GPIOA->PUPDR &= ~(3 << (9 * 2)); // No pull-up, pull-down
        GPIOA->AFR[1] |= (7 << (1 * 4)); // AF7 for USART1_TX

        // Configure PA10 (RX) as Alternate Function, High Speed, No Pull-Up/Down
        GPIOA->MODER &= ~(3 << (10 * 2)); // Clear bits
        GPIOA->MODER |= (2 << (10 * 2));  // Set Alternate Function
        GPIOA->PUPDR &= ~(3 << (10 * 2)); // No pull-up, pull-down
        GPIOA->AFR[1] |= (7 << (2 * 4));  // AF7 for USART1_RX

        // Enable USART1 clock
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    }

    // Calculate UARTDIV for baud rate
    UARTDIV = (SystemCoreClock / (16.0 * baudrate));
    integ = (uint16_t)UARTDIV;
    frac = (uint8_t)((UARTDIV - integ) * 16);
    uart->BRR = (integ << 4) | frac;

    // Enable UART, Transmitter, and Receiver
    uart->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

/* Send a single byte */
void Uart_SendByte(USART_TypeDef* uart, uint8_t data)
{
    while (!(uart->ISR & USART_ISR_TXE)); // Wait until TXE (Transmit Data Register Empty) is set
    uart->TDR = data;                    // Transmit data
}

/* Send a string */
void Uart_SendString(USART_TypeDef* uart, uint8_t* str)
{
    uint32_t i = 0;
    while (str[i])
    {
        Uart_SendByte(uart, str[i]);
        i++;
    }
}

/* Receive a single byte */
void Uart_ReceiveByte(USART_TypeDef* uart, uint8_t* pData)
{
    while (!(uart->ISR & USART_ISR_RXNE)); // Wait until RXNE (Receive Data Register Not Empty) is set
    *pData = (uint8_t)uart->RDR;          // Read data
}
