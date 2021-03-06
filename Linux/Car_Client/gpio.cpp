#include "gpio.h"

//TODO Check which are used for other things and remove these.

#define GPIO_PINS 0, 1, 4, 7, 8, 9, 10, 11, 14, 15, 17, 18, 21, 22, 23, 24, 25
#define STR_LENGTH 64

GPIO::GPIO(QObject *parent)
{

}

GPIO::~GPIO(){

}

/*!
 * \brief GPIO::setGPIO_Out defines a specific GPIO pin to be used as output
 * \param pin number of the specified pin to be set as output
 */
int GPIO::setGPIO_Out(int pin)
{
    int gpio_pins[]={GPIO_PINS};
    int valid = 0;

    //Check if valid GPIO pin
    for (int i=0; i<(sizeof(gpio_pins) / sizeof(int)); i++)
    {
        if(pin == gpio_pins[i]){
            valid = 1;
        }

    }

    if(!valid)
    {
        fprintf(stderr, "ERROR: Tried setting invalid pin to OUT!\nPin %d is not a GPIO pin or already in use\n", pin);
        return -1;
    }

    FILE *sysfsHandle = NULL;

    if ((sysfsHandle = fopen("/sys/class/gpio/export", "w")) == NULL)
    {
        fprintf(stderr, "ERROR: Tried setting pin %d to OUT. Cannot open GPIO export...\n", pin);
        return 1;
    }

    //convert pin number to str
    char strPin[3];
    printf(strPin, (3*sizeof(char)), "%d", pin);

    //Eport pin by writing to the gpio/export file
    if (fwrite(&strPin, sizeof(char), 3, sysfsHandle)!=3)
    {
        fprintf(stderr, "ERROR: Tried setting pin %d to OUT. Unable to export GPIO pin %d\n", pin, pin);
        return 2;
    }
    fclose(sysfsHandle);

    //Set direction in direction file
    char str_direction_file[STR_LENGTH];
    snprintf(str_direction_file, (STR_LENGTH*sizeof(char)), "/sys/class/gpio/gpio%d/direction", pin);
    if ((sysfsHandle = fopen(str_direction_file, "w")) == NULL)
    {
        fprintf(stderr, "ERROR: Tried setting pin %d to OUT. Cannot open direction file...\n", pin);
        return 3;
    }

    //write "out" to the direction file.
    if (fwrite("out", sizeof(char), 4, sysfsHandle) != 4)
    {
        fprintf(stderr, "ERROR: Tried setting pin %d to OUT. Unable to write direction for GPIO %d\n", pin, pin);
        return 4;
    }
    fclose(sysfsHandle);

    fprintf(stdout,"Set Pin %d as OUT\n", pin);

    return 0;
}

int GPIO::GPIO_Write(GPIO::PinOperation_t pinOp)
{
    int pin = pinOp.first, value = pinOp.second;

    if ((value!=0)&&(value!=1))
    {
        fprintf(stderr, "ERROR: Tried writing to pin %d with invalid value %d!\nValue must be 0 or 1\n", pin, value);
        return -1;
    }
    FILE *sysfsHandle = NULL;
    char strValueFile[STR_LENGTH];

    snprintf (strValueFile, (STR_LENGTH*sizeof(char)), "/sys/class/gpio/gpio%d/value", pin);

    if ((sysfsHandle = fopen(strValueFile, "w")) == NULL)
    {
        fprintf(stderr, "ERROR: Tried writing to pin %d. Cannot open value file for pin.\nHas pin %d been exported?\n", pin, pin);
        return 1;
    }

    char strVal[2];
    snprintf (strVal, (2*sizeof(char)), "%d", value);

    if(fwrite(strVal, sizeof(char), 2, sysfsHandle) != 2)
    {
        fprintf(stderr, "ERROR: Tried writing to pin %d. Cannot write value %d to GPIO pin %d\n", value, pin, pin);
        return 2;
    }
    fclose(sysfsHandle);

    fprintf(stdout, "Set Pin %d to %d\n", pin, value);

    return 0;
}

int GPIO::unsetGPIO(int pin)
{
    FILE *sysfsHandle = NULL;
    char strPin[3];
    char strValueFile[STR_LENGTH];

    snprintf (strPin, (3*sizeof(char)), "%d", pin);
    snprintf (strValueFile, (STR_LENGTH*sizeof(char)), "/sys/class/gpio/gpio%d/value", pin);

    if ((sysfsHandle = fopen(strValueFile, "w")) == NULL)
    {
        fprintf(stderr, "ERROR: Tried unsetting pin %d. Unable open value file.", pin);
        return 1;
    }

    if(fwrite("0", sizeof(char), 2, sysfsHandle) != 2)
    {
        fprintf(stderr, "ERROR: Tried unsetting pin %d. Cannot write to GPIO pin\n", pin);
        return 2;
    }
    fclose(sysfsHandle);

    if ((sysfsHandle = fopen("/sys/class/gpio/unexport", "w")) == NULL)
    {
        fprintf(stderr, "ERROR: Tried unsetting pin %d. Cannot open GPIO unexport.\n", pin);
        return 1;
    }

    if (fwrite(&strPin, sizeof(char), 3, sysfsHandle)!=3)
    {
        fprintf(stderr, "ERROR: Tried unsetting pin %d. Unable to unexport GPIO pin.\n", pin);
        return 2;
    }
    fclose(sysfsHandle);

    fprintf(stdout,"Unset Pin %d\n", pin);

    return 0;
}


