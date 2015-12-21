#include "interprocess/listener_factory.hpp"
#include "interprocess/sender_factory.hpp"
#include <chrono>
int main(){


    auto listener = interproc::make_listener<>("ipc://test.sock");
    auto sender = interproc::make_sender<>("ipc://test.sock");

    listener->start();
    sender->connect();
    for (int i=0; i< 10000; i++) {
        sender->send(interproc::buffer("test"));
    }
    sender->close();
    listener->stop();
    listener->wait_until_stopped();
    return 0;
}