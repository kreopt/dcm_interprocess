#include "interprocess/interprocess.hpp"
#include <chrono>
int main(){
    using namespace std::chrono_literals;

    auto listener = interproc::make_listener<>("ipc://test.sock");
    auto sender = std::make_shared<interproc::p2p_sender<>>("ipc://test.sock");

    auto buf = interproc::buffer(new char[1920*1080*3], 1920*1080*3);

    listener->on_message = [](interproc::buffer &&_buf){
        interproc::Log::d("received");
    };

    listener->start();
    sender->connect();
    for (int i=0; i< 10/*000*/; i++) {
        sender->send(buf);
    }
    sender->close();
    listener->stop();
    listener->wait_until_stopped();
    return 0;
}