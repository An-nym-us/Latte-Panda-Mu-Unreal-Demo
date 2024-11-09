// MIT License
// Copyright (c) 2024 Jonathan Lee Gunderson


#include "TestDoingCoolStuff.h"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstdint>
#include <csignal>
#include <string>


void LazyLog(const FString& Message)
{
    if (GEngine)
    {
        // Log to screen
        GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Yellow, Message);
    }

    // Log to Output Log
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}



// Sets default values
ADoingCoolStuff::ADoingCoolStuff()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}


void ADoingCoolStuff::TogglePin(const int pin)
{
    uint8_t orginal_state;
    if(read(i2c_fd, &orginal_state,1) != 1)
    {
        LazyLog("Error Reading Bytes");
        return;
    }


    orginal_state ^= (1 << pin);
    if(write(i2c_fd, &orginal_state, 1) != 1)
    {
        LazyLog("Error Writing Bytes");
        return;
    }

}


void ADoingCoolStuff::SetPinState(const int pin, const bool bState)
{
    uint8_t original_state;

    // Read the current state of the device, so we don't overwrite other pins
    if (read(i2c_fd, &original_state, 1) != 1)
    {
        LazyLog("Error Reading Current State");
        return;
    }

    // Modify the specific pin's bit
    if (bState)
    {
        original_state |= (1 << pin);  // Set high
    }
    else
    {
        original_state &= ~(1 << pin); // Set low
    }

    // Write the modified state back to the device
    if (write(i2c_fd, &original_state, 1) != 1)
    {
        LazyLog("Error Writing Bytes");
    }

}

// Called when the game starts or when spawned
void ADoingCoolStuff::BeginPlay()
{

	isRunning.store(true);

    InitITwoC();
	Super::BeginPlay();
	
}

void ADoingCoolStuff::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // used to force kill the thread.
    isRunning.store(false); 

    LazyLog("Closing Relays.");

    uint8_t orginal_t = 0xFF;

    if(write(i2c_fd, &orginal_t, 1) != 1)
    {
        LazyLog("Error Writing Bytes To Close Relay.");
    }

    // Close I2c .
    close(i2c_fd);

    Super::EndPlay(EndPlayReason);
}


// Called every frame
void ADoingCoolStuff::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ADoingCoolStuff::singal_handler(int signmun)
{
    uint8_t resetData = 0xFF;

    if(write(i2c_fd, &resetData, 1) != 1)
    {
        LazyLog("Failed to reset relays.");
    }
    LazyLog("I2C Stopped, all realys reset to high");

    close(i2c_fd);
    exit(signmun);
}


void ADoingCoolStuff::WaveEffect()
{
    uint8_t orginal_t = 0xFF;

    while(isRunning.load())
    {
        for (int i = 0; i < 8; ++i)
        {
            //uint8_t dataToWrite = ~(1 << i ) & 0xFF;// Shift 1, invert, keep within 8 bits
            orginal_t ^= (1 << i);
            if(write(i2c_fd, &orginal_t, 1) != 1)
            {
                LazyLog("Error Writing Bytes");
                return;
            }
            usleep(DELAY);
        }
    }
}



bool ADoingCoolStuff::AutoFindPCFAddress()
{

    for (int address = 0x20; address <= 0x27; ++address)
    {
        // Attempt to set the address
        if (ioctl(i2c_fd, I2C_SLAVE, address) < 0)
        {
            LazyLog(FString::Printf(TEXT("Failed to set address 0x%02X\n"), address));
            continue;
        }

        // Try a dummy write to check if the device responds
        uint8_t dummy = 0;
        if (write(i2c_fd, &dummy, 1) == 1)
        {
            PCF_Address = address; // Found the device
            LazyLog(FString::Printf(TEXT("Found I2C address of: 0x%02X\n"), PCF_Address));
            break;
        }
    }



    if (PCF_Address == -1)
    {
        LazyLog("PCF8574 not found on the I2C bus");
        return false;
    }


    return true;
}


int ADoingCoolStuff::InitITwoC()
{
    std::string i2cFileName = "/dev/i2c-" + std::to_string(I2C_BUs);


    i2c_fd = open(i2cFileName.c_str(), O_RDWR);
    if(i2c_fd < 0)
    {
        LazyLog("Failed to open i2c bus");
        return 1;
    }


    // Find Address.
    if(!AutoFindPCFAddress())
    {
        LazyLog("Failed to Find Address");
        return 1;
    }

    if(ioctl(i2c_fd, I2C_SLAVE, PCF_Address) < 0 )
    {
        LazyLog("Failed to quaire bus asscess or talk to PCF8574");
        close(i2c_fd);
        return 1;

    }

    // start all closed.
    uint8_t orginal_t = 0xFF;
    if(write(i2c_fd, &orginal_t, 1) != 1)
    {
        LazyLog("Error Writing Bytes");
        return 1;
    }
    return 0;
}

void ADoingCoolStuff::DoStuff()
{ 
    isRunning.store(true);
    LazyLog("Starting Wave");

    Async(EAsyncExecution::Thread, [this] () mutable 
    {
        WaveEffect();
    });

}

void ADoingCoolStuff::EndWave()
{
    LazyLog("Ending Wave");
    isRunning.store(false);

    // wait a little for the thread to die.
    usleep(DELAY);

    // Close all relays.
    uint8_t orginal_t = 0xFF;
    
    if(write(i2c_fd, &orginal_t, 1) != 1)
    {
        LazyLog("Error Writing Bytes To Close Relays from wave.");
    }
}