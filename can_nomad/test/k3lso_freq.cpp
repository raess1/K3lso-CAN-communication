#include <CAN/PCANDevice.h>

#include <string.h>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <chrono>

#define DEVICE "/dev/pcanpcifd0"

PCANDevice can;
std::string valid_channels = "1234";

uint8_t stop_pkg[] = {0x01, 0x00, 0x00};
uint8_t pos_00_pkg[] = {0x01, 0x00, 0x0a, 0x0e, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x1f, 0x01, 0x13, 0x0d};
uint8_t pos_90_pkg[] = {0x01, 0x00, 0x0a, 0x0e, 0x20, 0x00, 0x00, 0xa0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x1f, 0x01, 0x13, 0x0d};

int main(int argc, char** argv){
    int can_channel;
    std::string can_device;
    uint32_t can_target;
    int action;
    CANDevice::CAN_msg_t msg_tx;
    CANDevice::CAN_msg_t msg_rx;
    std::string command_str;
    int command_str_len;

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
        std::cerr << "Unable to open CAN Device" << std::endl;
        return 1;
    }
    can.ClearFilters();

    std::cout << "Opened" << std::endl;

    

    msg_tx.id = 0x8001;
    memcpy(msg_tx.data, pos_00_pkg, sizeof(pos_00_pkg));
    msg_tx.length = sizeof(pos_00_pkg);

    std::cout << "Writing and reading 1000 times" << std::endl;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for(int i=0; i<1000; i++){
        can.Send(msg_tx);
        while(!can.Receive(msg_rx));
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Finished" << std::endl;

    double freq = (double)(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) / 1000000;

    std::cout << "Time for 1000 w/r cycles = " << freq << "[s]" << std::endl;
    freq /= 1000;
    std::cout << "Time for 1 w/r cycle = " << freq << "[s]" << std::endl;
    freq = 1 / freq;
    std::cout << "Frequency = " << freq << "[hz]" << std::endl;

    can.Close();

    std::cout << "Closed" << std::endl;

    return 0;
}