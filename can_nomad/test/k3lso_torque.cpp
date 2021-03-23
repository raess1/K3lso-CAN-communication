#include <CAN/PCANDevice.h>

#include <string.h>
#include <iostream>
#include <unistd.h>
#include <math.h>


#define DEVICE "/dev/pcanpcifd"

PCANDevice can;
std::string valid_channels = "1234";

uint8_t stop_pkg[] = {0x01, 0x00, 0x00};
uint8_t pos_00_pkg[] = {0x01, 0x00, 0x0a, 0x0e, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x1f, 0x01, 0x13, 0x0d};
uint8_t pos_90_pkg[] = {0x01, 0x00, 0x0a, 0x0e, 0x20, 0x00, 0x00, 0x20, 0x41, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x1f, 0x01, 0x13, 0x0d};

int main(int argc, char** argv){
    int can_channel;
    std::string can_device;
    uint32_t can_target;
    int action;
    CANDevice::CAN_msg_t msg_tx;
    CANDevice::CAN_msg_t msg_rx;
    std::string command_str;
    int command_str_len;

    std::cout << "Select CAN Channel (1, 2, 3 or 4)" << std::endl << ">> ";
    std::cout.flush();
    std::cin >> can_channel;
    if(can_channel<1 || can_channel>4){
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
        std::cerr << "Unable to open CAN Device" << std::endl;
        return 1;
    }
    can.ClearFilters();

    std::cout << "Opened" << std::endl;

    std::cout << "Enter target (decimal, NOT hex)" << std::endl << ">> ";
    std::cout.flush();
    std::cin >> can_target;

    std::cout << "Enter action:" << std::endl;
    std::cout << "1. Stop" << std::endl;
    std::cout << "2. Pos 0" << std::endl;
    std::cout << "3. Pos 90" << std::endl;
    std::cout << ">> ";
    std::cout.flush();
    std::cin >> action;
    if(action<1 or action>3){
        std::cerr << "Not valid action" << std::endl;
        return 1;
    }

    msg_tx.id = can_target | 0x8000;
    if(action==1){
        memcpy(msg_tx.data, stop_pkg, sizeof(stop_pkg));
        msg_tx.length = sizeof(stop_pkg);
    }else if(action==2){
        memcpy(msg_tx.data, pos_00_pkg, sizeof(pos_00_pkg));
        msg_tx.length = sizeof(pos_00_pkg);
    }else if(action==3){
        memcpy(msg_tx.data, pos_90_pkg, sizeof(pos_90_pkg));
        msg_tx.length = sizeof(pos_90_pkg);
    }
    std::cout << "Sending Message" << std::endl;
    can.Send(msg_tx);
    std::cout << "Message Sent" << std::endl;

    std::cout << "Waiting for response" << std::endl;
    while(!can.Receive(msg_rx));
    std::cout << "Reponse" << msg_rx.length << std::endl;

    can.Close();

    std::cout << "Closed" << std::endl;

    return 0;
}