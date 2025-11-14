#include "e32.h"
#include <string.h>

uint8_t LoRa_RX_Buffer[64];
char rx_line[RX_LINE_MAX];
uint8_t rx_idx = 0;

// --- Встановлюємо режим модуля ---
void E32_SetMode(E32_Mode mode)
{
    switch(mode)
    {
        case E32_MODE_NORMAL:
            HAL_GPIO_WritePin(E32_M0_PORT, E32_M0_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(E32_M1_PORT, E32_M1_PIN, GPIO_PIN_RESET);
            break;
        case E32_MODE_WAKEUP:
            HAL_GPIO_WritePin(E32_M0_PORT, E32_M0_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(E32_M1_PORT, E32_M1_PIN, GPIO_PIN_RESET);
            break;
        case E32_MODE_POWERDOWN:
            HAL_GPIO_WritePin(E32_M0_PORT, E32_M0_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(E32_M1_PORT, E32_M1_PIN, GPIO_PIN_SET);
            break;
        case E32_MODE_PROGRAM:
            HAL_GPIO_WritePin(E32_M0_PORT, E32_M0_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(E32_M1_PORT, E32_M1_PIN, GPIO_PIN_SET);
            break;
    }
    HAL_Delay(50); // даємо час на переключення режиму
}

// --- Перевірка готовності через AUX ---
uint8_t E32_IsReady(void)
{
    return (HAL_GPIO_ReadPin(E32_AUX_PORT, E32_AUX_PIN) == GPIO_PIN_SET);
}

// --- Відправка рядка ---
void E32_SendString(char *str)
{
    while(!E32_IsReady()) HAL_Delay(5);
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

// --- Відправка одного байта ---
void E32_SendByte(uint8_t data)
{
    while(!E32_IsReady()) HAL_Delay(5);
    HAL_UART_Transmit(&huart2, &data, 1, HAL_MAX_DELAY);
}
// Callback — викликається при кожному прийнятому байті
uint8_t Packet[64];
uint8_t idx = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {

        uint8_t b = LoRa_RX_Buffer[0];

        if (b == '\n' || rx_idx >= RX_LINE_MAX-1)  // кінець рядка
        {
            rx_line[rx_idx] = 0;  // завершити рядок
            ssd1306_clear();      // очистити дисплей (або частково)
            ssd1306_write_string(0, 0, rx_line);  // вивести прийняті дані
            rx_idx = 0;           // скинути індекс
        }
        else
        {
            rx_line[rx_idx++] = b;  // додати байт в буфер
        }

        HAL_UART_Receive_IT(&huart2, LoRa_RX_Buffer, 1);
    }
}

// -------------------------
// OLED custom functions
// -------------------------
void oled_print_char(char c)
{
    static uint8_t x = 0;
    static uint8_t y = 0;

    // якщо дійшли до кінця рядка
    if (x > 120) {
        x = 0;
        y += 8;
        if (y > 56) {    // дисплей 128x64 → 8 рядків
            y = 0;
            ssd1306_clear();
        }
    }

    char buf[2] = { c, 0 };
    ssd1306_write_string(x, y, buf); // Вивести символ
    x += 6; // ширина символа 5x8 + 1
}
