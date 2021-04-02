#include <CAN/PCANDevice.h>

#include <string.h>
#include <iostream>
#include <unistd.h>
#include <math.h>


#define DEVICE "/dev/pcanpcifd"

PCANDevice can;

int main(int argc, char** argv){
    int can_channel;
    std::string can_device;
    uint32_t can_target;
    CANDevice::CAN_msg_t msg;
    std::string command_str;
    int command_str_len;

    std::cout << "Select CAN Channel (1-12)" << std::endl << ">> ";
    std::cout.flush();
    std::cin >> can_channel;
    if(can_channel<1 || can_channel>12){
        std::cerr << "Not valid channel" << std::endl;
        return 1;
    }
    can_device = DEVICE + std::to_string(can_channel-1);
    std::cout << "Starting in device " << can_device << std::endl;

    CANDevice::Config_t config;
    config.bitrate = 1e6; //1mbps
    config.d_bitrate = 2e6; //2mbps
    config.sample_point = .875; //87.5% 
    config.d_sample_point = 0.6; //60%
    config.clock_freq = 80e6; // 80mhz // Read from driver?  
    config.mode_fd = 1; // FD Mode

    if(!can.Open(can_device, config, false))
    {
        std::cout << "Unable to open CAN Device" << std::endl;
        return 1;
    }
    can.ClearFilters();

    std::cout << "Opened" << std::endl;

    std::cout << "Enter target (decimal, NOT hex)" << std::endl << ">> ";
    std::cout.flush();
    std::cin >> can_target;

    std::cout << "Enter command (string, NOT hex)" << std::endl << ">> ";
    std::cout.flush();
    std::cin.ignore();
    std::cin.getline((char*)msg.data, 64);
    command_str_len = strlen((char*)msg.data);
    msg.data[command_str_len] = '\n';
    msg.data[command_str_len+1] = '\0';

    msg.id = can_target;
    msg.length = command_str_len+1;
    std::cout << "Sending Message" << std::endl;
    can.Send(msg);
    std::cout << "Message Sent" << std::endl;

    can.Close();

    std::cout << "Closed" << std::endl;

    return 0;
}