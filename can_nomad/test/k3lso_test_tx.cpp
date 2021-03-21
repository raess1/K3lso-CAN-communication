#include <CAN/PCANDevice.h>

#include <string.h>
#include <iostream>
#include <unistd.h>
#include <math.h>


#define DEVICE "/dev/pcanpcifd0"

PCANDevice can;
char test_string[] = "hello Robin";

int main(int argc, char** argv){

    std::cout << "Starting in device " << DEVICE << std::endl;

    CANDevice::Config_t config;
    config.bitrate = 1e6; //1mbps
    config.d_bitrate = 2e6; //2mbps
    config.sample_point = .875; //87.5% 
    config.d_sample_point = 0.6; //60%
    config.clock_freq = 80e6; // 80mhz // Read from driver?  
    config.mode_fd = 1; // FD Mode

    if(!can.Open(DEVICE, config, false))
    {
        std::cout << "Unable to open CAN Device" << std::endl;
        return 1;
    }
    can.ClearFilters();

    std::cout << "Opened" << std::endl;

    CANDevice::CAN_msg_t msg;
    msg.id = 25;
    msg.length = strlen(test_string) + 1;
    memcpy(msg.data, test_string, msg.length);
    std::cout << "Sending Message" << std::endl;
    can.Send(msg);
    std::cout << "Message Sent" << std::endl;

    can.Close();

    std::cout << "Closed" << std::endl;

    return 0;
}