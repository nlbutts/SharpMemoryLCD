//*****************************************************************************
//
// Template_Driver.c - Display driver for any LCD Controller. This file serves as
//						a template for creating new LCD driver files
//
//*****************************************************************************
//
//! \addtogroup display_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// READ ME
//
// This template driver is intended to be modified for creating new LCD drivers
// It is setup so that only Template_DriverPixelDraw() and DPYCOLORTRANSLATE()
// and some LCD size configuration settings in the header file Template_Driver.h
// are REQUIRED to be written. These functions are marked with the string
// "TemplateDisplayFix" in the comments so that a search through Template_Driver.c and
// Template_Driver.h can quickly identify the necessary areas of change.
//
// Template_DriverPixelDraw() is the base function to write to the LCD
// display. Functions like WriteData(), WriteCommand(), and SetAddress()
// are suggested to be used to help implement the Template_DriverPixelDraw()
// function, but are not required. SetAddress() should be used by other pixel
// level functions to help optimize them.
//
// This is not an optimized driver however and will significantly impact
// performance. It is highly recommended to first get the prototypes working
// with the single pixel writes, and then go back and optimize the driver.
// Please see application note www.ti.com/lit/pdf/slaa548 for more information
// on how to fully optimize LCD driver files. In short, driver optimizations
// should take advantage of the auto-incrementing of the LCD controller.
// This should be utilized so that a loop of WriteData() can be used instead
// of a loop of Template_DriverPixelDraw(). The pixel draw loop contains both a
// SetAddress() + WriteData() compared to WriteData() alone. This is a big time
// saver especially for the line draws and Template_DriverPixelDrawMultiple.
// More optimization can be done by reducing function calls by writing macros,
// eliminating unnecessary instructions, and of course taking advantage of other
// features offered by the LCD controller. With so many pixels on an LCD screen
// each instruction can have a large impact on total drawing time.
//
//*****************************************************************************

//*****************************************************************************
//
// Include Files
//
//*****************************************************************************
#include <device.h>
#include "grlib.h"
#include "SharpLS013B4DN04.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
/* Global buffer for the display. This is especially useful on 1BPP, 2BPP, and 4BPP
   displays. This allows you to update pixels while reading the neighboring pixels
   from the buffer instead of a read from the LCD controller. A buffer is not required
   as a read followed by a write can be used instead.*/
uint8_t SharpLS013B4DN04_Memory[(LCD_X_SIZE * LCD_Y_SIZE * BPP + 7) / 8];

//*****************************************************************************
//
// Suggested functions to help facilitate writing the required functions below
//
//*****************************************************************************

static uint8_t swap(uint8_t b)
{
    // See http://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious
    return ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
}

// Writes data to the LCD controller
static void
WriteData(uint8_t x, uint8_t y, uint8_t usData)
{
    uint8_t mask = swap(1 << (x % 8));
    if (usData)
    {
        // This just sets or clears pixels in memory
        SharpLS013B4DN04_Memory[(y*LCD_Y_SIZE)/8+x/8] |= mask;
    }
    else
    {
        // This just sets or clears pixels in memory
        SharpLS013B4DN04_Memory[(y*LCD_Y_SIZE)/8+x/8] &= ~mask;
    }
}

// Writes a command to the LCD controller
static void
WriteCommand(uint8_t ucCommand)
{
    /* This function is typically very similar (sometimes the same) as WriteData()
       The difference is that this is for the LCD to interpret commands instead of pixel
       data. For instance in 8080 protocol, this means pulling the DC line low.*/
}

// Sets the pixel address of the LCD driver
void SetAddress(int16_t lX,
                int16_t lY)
{
    /* This function typically writes commands (using WriteCommand()) to the
        LCD to tell it where to move the cursor for the next pixel to be drawn. */
}

// Initializes the pins required for the GPIO-based LCD interface.
// This function configures the GPIO pins used to control the LCD display
// when the basic GPIO interface is in use. On exit, the LCD controller
// has been reset and is ready to receive command and data writes.
static void
InitGPIOLCDInterface(void)
{
    /* Initialize the hardware to configure the ports properly for communication */
}

// Initialize DisplayBuffer.
// This function initializes the display buffer and discards any cached data.
static void
InitLCDDisplayBuffer(uint32_t ulValue)
{
    uint16_t i = 0,j = 0;
    for(i = 0; i < LCD_Y_SIZE; i++)
    {
        for(j = 0; j < (LCD_X_SIZE * BPP + 7) / 8; j++)
        {
            SharpLS013B4DN04_Memory[i * LCD_Y_SIZE + j] = ulValue;
        }
    }
}

// Initializes the display driver.
// This function initializes the LCD controller
//
// TemplateDisplayFix
void
SharpLS013B4DN04_DriverInit(void)
{
    uint32_t backGroudValue = 0x0;
    /*Initialize the LCD controller.
       This typically looks like:*/

    InitGPIOLCDInterface();
    InitLCDDisplayBuffer(backGroudValue);

    // Init LCD controller parameters
    // Enable LCD
    // Init Backlight
    // Clear Screen
}

static uint8_t VCOM = 0;

static uint8_t getVCOM()
{
    return VCOM << 6;
}


void SharpLS013B4DN04_FlushBufferToLCD()
{
    uint8_t status;
    uint8_t command;
    uint8_t line = 0;

    Pin_LCD_CS_Write(1);
    CyDelayUs(150);

    // Wire the command
    command = 0x80 | getVCOM();
    SPIM_WriteTxData(command);

    CyDelayUs(100);

    // Write the lines
    while (line < LCD_Y_SIZE)
    {
        SPIM_WriteTxData(swap(line++));
        SPIM_PutArray(&SharpLS013B4DN04_Memory[line*LCD_Y_SIZE], LCD_Y_SIZE/8 + 1);
        do
        {
            status = SPIM_ReadTxStatus();
        } while ((status & SPIM_STS_SPI_DONE) != SPIM_STS_SPI_DONE);
        CyDelayUs(100);
    }

    // One extra byte
    SPIM_WriteTxData(0);
    CyDelayUs(5);
    Pin_LCD_CS_Write(0);
}

void SharpLS013B4DN04_toggleVCOM()
{
    uint8_t lcdData[2];
    uint8_t status;
    lcdData[0] = getVCOM();
    lcdData[1] = 0;
    Pin_LCD_CS_Write(1);
    CyDelayUs(150);
    SPIM_PutArray(lcdData, 2);
    do
    {
        status = SPIM_ReadTxStatus();
    } while ((status & SPIM_STS_SPI_DONE) != SPIM_STS_SPI_DONE);
    CyDelayUs(5);
    Pin_LCD_CS_Write(0);

}

//*****************************************************************************
//
// All the following functions (below) for the LCD driver are required by grlib
//
//*****************************************************************************

//*****************************************************************************
//
//! Draws a pixel on the screen.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param lX is the X coordinate of the pixel.
//! \param lY is the Y coordinate of the pixel.
//! \param ulValue is the color of the pixel.
//!
//! This function sets the given pixel to a particular color.  The coordinates
//! of the pixel are assumed to be within the extents of the display.
//!
//! \return None.
//
//*****************************************************************************
// TemplateDisplayFix
static void
Template_DriverPixelDraw(void *pvDisplayData,
                         int16_t lX,
                         int16_t lY,
                         uint16_t ulValue)
{
    /* This function already has checked that the pixel is within the extents of
       the LCD screen and the color ulValue has already been translated to the LCD.
       This function typically looks like:*/

    // Interpret pixel data (if needed)
    // Update buffer (if applicable)
    // Template_Memory[lY * LCD_Y_SIZE + (lX * BPP / 8)] = , |= , &= ...
    // Template memory must be modified at the bit level for 1/2/4BPP displays

    //SetAddress(MAPPED_X(lX, lY), MAPPED_Y(lX, lY));
    WriteData(MAPPED_X(lX, lY), MAPPED_Y(lX, lY), ulValue);
}

//*****************************************************************************
//
//! Draws a horizontal sequence of pixels on the screen.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param lX is the X coordinate of the first pixel.
//! \param lY is the Y coordinate of the first pixel.
//! \param lX0 is sub-pixel offset within the pixel data, which is valid for 1
//! or 4 bit per pixel formats.
//! \param lCount is the number of pixels to draw.
//! \param lBPP is the number of bits per pixel; must be 1, 4, or 8.
//! \param pucData is a pointer to the pixel data.  For 1 and 4 bit per pixel
//! formats, the most significant bit(s) represent the left-most pixel.
//! \param pucPalette is a pointer to the palette used to draw the pixels.
//!
//! This function draws a horizontal sequence of pixels on the screen, using
//! the supplied palette.  For 1 bit per pixel format, the palette contains
//! pre-translated colors; for 4 and 8 bit per pixel formats, the palette
//! contains 24-bit RGB values that must be translated before being written to
//! the display.
//!
//! \return None.
//
//*****************************************************************************
static void
Template_DriverPixelDrawMultiple(void *pvDisplayData,
                                 int16_t lX,
                                 int16_t lY,
                                 int16_t lX0,
                                 int16_t lCount,
                                 int16_t lBPP,
                                 const uint8_t *pucData,
                                 const uint32_t *pucPalette)
{
    uint16_t ulByte;

    //
    // Determine how to interpret the pixel data based on the number of bits
    // per pixel.
    //
    switch(lBPP)
    {
    // The pixel data is in 1 bit per pixel format
    case 1:
    {
        // Loop while there are more pixels to draw
        while(lCount > 0)
        {
            // Get the next byte of image data
            ulByte = *pucData++;

            // Loop through the pixels in this byte of image data
            for(; (lX0 < 8) && lCount; lX0++, lCount--)
            {
                // Draw this pixel in the appropriate color
                Template_DriverPixelDraw(pvDisplayData, lX++, lY,
                                         ((uint16_t *)pucPalette)[(ulByte >>
                                                                   (7 -
                                                                    lX0)) & 1]);
            }

            // Start at the beginning of the next byte of image data
            lX0 = 0;
        }
        // The image data has been drawn

        break;
    }

    // The pixel data is in 2 bit per pixel format
    case 2:
    {
        // Loop while there are more pixels to draw
        while(lCount > 0)
        {
            // Get the next byte of image data
            ulByte = *pucData++;

            // Loop through the pixels in this byte of image data
            for(; (lX0 < 4) && lCount; lX0++, lCount--)
            {
                // Draw this pixel in the appropriate color
                Template_DriverPixelDraw(pvDisplayData, lX++, lY,
                                         ((uint16_t *)pucPalette)[(ulByte >>
                                                                   (6 -
                                                                    (lX0 <<
                    1))) & 3]);
            }

            // Start at the beginning of the next byte of image data
            lX0 = 0;
        }
        // The image data has been drawn

        break;
    }
    // The pixel data is in 4 bit per pixel format
    case 4:
    {
        // Loop while there are more pixels to draw.  "Duff's device" is
        // used to jump into the middle of the loop if the first nibble of
        // the pixel data should not be used.  Duff's device makes use of
        // the fact that a case statement is legal anywhere within a
        // sub-block of a switch statement.  See
        // http://en.wikipedia.org/wiki/Duff's_device for detailed
        // information about Duff's device.
        switch(lX0 & 1)
        {
        case 0:

            while(lCount)
            {
                // Get the upper nibble of the next byte of pixel data
                // and extract the corresponding entry from the palette
                ulByte = (*pucData >> 4);
                ulByte = (*(uint16_t *)(pucPalette + ulByte));
                // Write to LCD screen
                Template_DriverPixelDraw(pvDisplayData, lX++, lY, ulByte);

                // Decrement the count of pixels to draw
                lCount--;

                // See if there is another pixel to draw
                if(lCount)
                {
                case 1:
                    // Get the lower nibble of the next byte of pixel
                    // data and extract the corresponding entry from
                    // the palette
                    ulByte = (*pucData++ & 15);
                    ulByte = (*(uint16_t *)(pucPalette + ulByte));
                    // Write to LCD screen
                    Template_DriverPixelDraw(pvDisplayData, lX++, lY, ulByte);

                    // Decrement the count of pixels to draw
                    lCount--;
                }
            }
        }
        // The image data has been drawn.

        break;
    }

    // The pixel data is in 8 bit per pixel format
    case 8:
    {
        // Loop while there are more pixels to draw
        while(lCount--)
        {
            // Get the next byte of pixel data and extract the
            // corresponding entry from the palette
            ulByte = *pucData++;
            ulByte = (*(uint16_t *)(pucPalette + ulByte));
            // Write to LCD screen
            Template_DriverPixelDraw(pvDisplayData, lX++, lY, ulByte);
        }
        // The image data has been drawn
        break;
    }
    }
}

//*****************************************************************************
//
//! Draws a horizontal line.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param lX1 is the X coordinate of the start of the line.
//! \param lX2 is the X coordinate of the end of the line.
//! \param lY is the Y coordinate of the line.
//! \param ulValue is the color of the line.
//!
//! This function draws a horizontal line on the display.  The coordinates of
//! the line are assumed to be within the extents of the display.
//!
//! \return None.
//
//*****************************************************************************
static void
Template_DriverLineDrawH(void *pvDisplayData,
                         int16_t lX1,
                         int16_t lX2,
                         int16_t lY,
                         uint16_t ulValue)
{
    /* Ideally this function shouldn't call pixel draw. It should have it's own
       definition using the built in auto-incrementing of the LCD controller and its
       own calls to SetAddress() and WriteData(). Better yet, SetAddress() and WriteData()
       can be made into macros as well to eliminate function call overhead. */

    do
    {
        Template_DriverPixelDraw(pvDisplayData, lX1, lY, ulValue);
    }
    while(lX1++ < lX2);
}

//*****************************************************************************
//
//! Draws a vertical line.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param lX is the X coordinate of the line.
//! \param lY1 is the Y coordinate of the start of the line.
//! \param lY2 is the Y coordinate of the end of the line.
//! \param ulValue is the color of the line.
//!
//! This function draws a vertical line on the display.  The coordinates of the
//! line are assumed to be within the extents of the display.
//!
//! \return None.
//
//*****************************************************************************
static void
Template_DriverLineDrawV(void *pvDisplayData,
                         int16_t lX,
                         int16_t lY1,
                         int16_t lY2,
                         uint16_t ulValue)
{
    do
    {
        Template_DriverPixelDraw(pvDisplayData, lX, lY1, ulValue);
    }
    while(lY1++ < lY2);
}

//*****************************************************************************
//
//! Fills a rectangle.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param pRect is a pointer to the structure describing the rectangle.
//! \param ulValue is the color of the rectangle.
//!
//! This function fills a rectangle on the display.  The coordinates of the
//! rectangle are assumed to be within the extents of the display, and the
//! rectangle specification is fully inclusive (in other words, both sXMin and
//! sXMax are drawn, along with sYMin and sYMax).
//!
//! \return None.
//
//*****************************************************************************
static void
Template_DriverRectFill(void *pvDisplayData,
                        const Graphics_Rectangle *pRect,
                        uint16_t ulValue)
{
    int16_t x0 = pRect->sXMin;
    int16_t x1 = pRect->sXMax;
    int16_t y0 = pRect->sYMin;
    int16_t y1 = pRect->sYMax;

    while(y0++ <= y1)
    {
        Template_DriverLineDrawH(pvDisplayData, x0, x1, y0, ulValue);
    }
}

//*****************************************************************************
//
//! Translates a 24-bit RGB color to a display driver-specific color.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param ulValue is the 24-bit RGB color.  The least-significant byte is the
//! blue channel, the next byte is the green channel, and the third byte is the
//! red channel.
//!
//! This function translates a 24-bit RGB color into a value that can be
//! written into the display's frame buffer in order to reproduce that color,
//! or the closest possible approximation of that color.
//!
//! \return Returns the display-driver specific color.
//
//*****************************************************************************
static uint32_t
Template_DriverColorTranslate(void *pvDisplayData,
                              uint32_t ulValue)
{
    /* The DPYCOLORTRANSLATE macro should be defined in TemplateDriver.h */

    //
    // Translate from a 24-bit RGB color to a color accepted by the LCD.
    //
    return(DPYCOLORTRANSLATE(ulValue));
}

//*****************************************************************************
//
//! Flushes any cached drawing operations.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//!
//! This functions flushes any cached drawing operations to the display.  This
//! is useful when a local frame buffer is used for drawing operations, and the
//! flush would copy the local frame buffer to the display.
//!
//! \return None.
//
//*****************************************************************************
static void
Template_DriverFlush(void *pvDisplayData)
{
    // Flush Buffer here. This function is not needed if a buffer is not used,
    // or if the buffer is always updated with the screen writes.
    int16_t i = 0,j = 0;
    for(i = 0; i < LCD_Y_SIZE; i++)
    {
        for(j = 0; j < (LCD_X_SIZE * BPP + 7) / 8; j++)
        {
            Template_DriverPixelDraw(pvDisplayData, j, i,
                                     SharpLS013B4DN04_Memory[i * LCD_Y_SIZE + j]);
        }
    }
}

//*****************************************************************************
//
//! Send command to clear screen.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//!
//! This function does a clear screen and the Display Buffer contents
//! are initialized to the current background color.
//!
//! \return None.
//
//*****************************************************************************
static void
Template_DriverClearScreen(void *pvDisplayData,
                           uint16_t ulValue)
{
    // This fills the entire display to clear it
    // Some LCD drivers support a simple command to clear the display
    int16_t y0 = 0;
    while(y0++ <= (LCD_Y_SIZE - 1))
    {
        Template_DriverLineDrawH(pvDisplayData, 0, LCD_X_SIZE - 1, y0, ulValue);
    }
}

//*****************************************************************************
//
//! The display structure that describes the driver for the blank template.
//
//*****************************************************************************
const Graphics_Display g_SharpLS013B4DN04_Driver =
{
    sizeof(tDisplay),
    SharpLS013B4DN04_Memory,
#if defined(PORTRAIT) || defined(PORTRAIT_FLIP)
    LCD_Y_SIZE,
    LCD_X_SIZE,
#else
    LCD_X_SIZE,
    LCD_Y_SIZE,
#endif
    Template_DriverPixelDraw,
    Template_DriverPixelDrawMultiple,
    Template_DriverLineDrawH,
    Template_DriverLineDrawV,
    Template_DriverRectFill,
    Template_DriverColorTranslate,
    Template_DriverFlush,
    Template_DriverClearScreen
};

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
