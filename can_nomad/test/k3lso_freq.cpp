#include <CAN/PCANDevice.h>

#include <string.h>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <chrono>

#define DEVICE "/dev/pcanpcifd8"

PCANDevice can;

uint8_t pos_00_pkg[] = {0x01, 0x00, 0x0a, 0x0c, 0x05, 0x20, 0x00, 0x00, 0x80, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x42, 0x00, 0x00, 0xa0, 0x40, 0x1f, 0x01, };
uint8_t pos_01_pkg[] = {0x01, 0x00, 0x0a, 0x0c, 0x05, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x42, 0x00, 0x00, 0xa0, 0x40, 0x1f, 0x01, };

int main(int argc, char** argv){
    int can_channel;
    std::string can_device;
    uint32_t can_target;
    int action;
    // CANDevice::CAN_msg_t msg_tx;
    CANDevice::CAN_msg_t msg_rx;
    std::string command_str;
    int command_str_len;

    std::cout << "Starting in device " << DEVICE << std::endl;

    CANDevice::Config_t config;
    config.bitrate = 1e6; //1mbps
    config.d_bitrate = 2e6; //2mbps
    config.sample_point = .875; //87.5% 
    config.d_sample_point = 0.8; //60%
    config.clock_freq = 80e6; // 80mhz // Read from driver?  
    config.mode_fd = 1; // FD Mode

    if(!can.Open(DEVICE, config, false))
    {
        std::cerr << "Unable to open CAN Device" << std::endl;
        return 1;
    }
    can.ClearFilters();

    std::cout << "Opened" << std::endl;

    

    // msg_tx.id = 0x8001;
    // memcpy(msg_tx.data, pos_00_pkg, sizeof(pos_00_pkg));
    // msg_tx.length = sizeof(pos_00_pkg);

    std::cout << "Writing and reading 10000 times" << std::endl;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for(int i=0; i<10000; i++){
        CANDevice::CAN_msg_t msg_tx;
        msg_tx.id = 0x8001;
        if( i<2500 || (i>5000 && i<7500))
            memcpy(msg_tx.data, pos_00_pkg, sizeof(pos_00_pkg));
        else
            memcpy(msg_tx.data, pos_01_pkg, sizeof(pos_01_pkg));
        msg_tx.length = sizeof(pos_00_pkg);
        can.Send(msg_tx);
        while(!can.Receive(msg_rx));
        // std::cout << "Rx size: " << msg_rx.length << std::endl;
        // for(int i=0; i<msg_rx.length; i++){
        //     std::cout << "  " << std::hex << (int)msg_rx.data[i] << std::dec << std::endl;
        // }
        // usleep(30000);
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Finished. Rx size: " << msg_rx.length << std::endl;

    double freq = (double)(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) / 1000000;

    std::cout << "Time for 10000 w/r cycles = " << freq << "[s]" << std::endl;
    freq /= 10000;
    std::cout << "Time for 1 w/r cycle = " << freq << "[s]" << std::endl;
    freq = 1 / freq;
    std::cout << "Frequency = " << freq << "[hz]" << std::endl;

    can.Close();

    std::cout << "Closed" << std::endl;

    return 0;
}
