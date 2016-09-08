#include <device.h>
#include <stdio.h>
#include "grlib.h"
#include "SharpLS013B4DN04.h"

#if defined (__GNUC__)
    /* Add an explicit reference to the floating point printf library */
    /* to allow the usage of floating point conversion specifiers. */
    /* This is not linked in by default with the newlib-nano library. */
    asm (".global _printf_float");
#endif


#define RD_BUFFER_LEN           (64u)
#define WR_BUFFER_LEN           (64u)
#define MUX_SIZE                (4u)

/* ASCII value of decimal zero is 48 */
#define ASCII_DECIMAL_ZERO      (48u)

/* Any value above 0x07 is an illegal input for mux channel selection */
#define ERROR_MASK              (0xF8u)

/* Function that encapsulates the process of writing text strings to USBUART */
void PrintToUSBUART(char8 * outText);

uint8_t vCOM = 0;

void fillDisplay(uint8_t line);
void clearDisplay();
uint8_t getVCOM();
void writeLCDLine(uint8_t *data, uint8_t length);
uint8_t swap(uint8_t b);
void toggleVCOM();

const char str[] = "Hello World";

int main()
{
    /* Enable Global interrupts - used for USB communication */
    CyGlobalIntEnable;

    SPIM_Start();

    Pin_LEDS_Write(2);

    CySysTickStart();

    // Turn the LCD on
    Pin_LCD_PWR_Write(1);
    Pin_LCD_GND_Write(0);
    Pin_LCD_EN_Write(1);
    Pin_EXT_COM_I_Write(0);

    SharpLS013B4DN04_DriverInit();
    tContext g;
    Graphics_initContext(&g, &g_SharpLS013B4DN04_Driver);
    Graphics_setForegroundColor(&g, ClrBlack);
    Graphics_setBackgroundColor(&g, ClrWhite);
    Graphics_clearDisplay(&g);
    Graphics_setFont(&g, &g_sFontFixed6x8);


    for(;;)
    {
        CyDelay(34);

        //SharpLS013B4DN04_toggleVCOM();
        SharpLS013B4DN04_FlushBufferToLCD();

        while (SW3_Read() == 0)
        {
            Graphics_Rectangle rect = {40, 40, 50, 50};
            Graphics_fillRectangle(&g, &rect);
            //Graphics_drawString(&g, str, strlen(str), 0, 20, true);
        }
        while (SW2_Read() == 0)
        {
            Graphics_clearDisplay(&g);
        }
    }  /* End of forever loop */
}  /* End of main */

