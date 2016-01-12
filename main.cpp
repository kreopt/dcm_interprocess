#include "interprocess/listener_factory.hpp"
#include "interprocess/sender_factory.hpp"
#include <chrono>
int main(){
    using namespace std::chrono_literals;

    auto listener = interproc::make_listener<>("ipc://test.sock");
    auto sender = interproc::make_sender<>("ipc://test1.sock");

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
    std::this_thread::sleep_for(1s);
    for (int i=0; i< 10/*000*/; i++) {
        sender->send(buf);
    }
    return 0;
}