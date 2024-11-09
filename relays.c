#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstdint>
#include <csignal>
#include <string>

const int I2C_BUs = 4;
const uint8_t PCF_Address = 0x27;
const int DELAY  = 100000;

int i2c_fd;

void singal_handler(int signmun)
{
    uint8_t resetData = 0xFF;
    if(write(i2c_fd, &resetData, 1) != 1)
    {
        std::cerr << "FAiled to reset relays." << std::endl;
    }
    std::cout << "\n Wave effect stopped, all realys reset to high" << std::endl;
    close(i2c_fd);
    exit(signmun);
}

void WaveEffect()
{
    while(true)
    {
        for (int i = 0; i < 8; ++i)
        {
            uint8_t dataToWrite = ~(1 << i ) & 0xFF;// Shift 1, invert, keep within 8 bits

            if(write(i2c_fd, &dataToWrite, 1) != 1)
            {
                std::cerr << "Error Writing Bytes" << std::endl;
                return;
            }
            usleep(DELAY);
        }

    }
}

int main()
{
    std::string i2cFileName = "/dev/i2c-" + std::to_string(I2C_BUs);


    i2c_fd = open(i2cFileName.c_str(), O_RDWR);
    if(i2c_fd < 0)
    {
        std::cerr << "Failed to open i2c bus" << std::endl;
        return 1;
    }


    if(ioctl(i2c_fd, I2C_SLAVE, PCF_Address) < 0 )
    {
        std::cerr << "Failed to quaire bus asscess or talk to PCF8574" << std::endl;
        close(i2c_fd);
        return 1;

    }

    WaveEffect();

    return 0;
}