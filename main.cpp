#include "interprocess/listener_factory.hpp"
#include "interprocess/sender_factory.hpp"

int main(){

    auto listener = interproc::make_listener<>("ipc://test.sock");
    auto sender = interproc::make_sender<>("ipc://test.sock");

    listener->start();
    sender->connect();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    sender->send(interproc::to_buffer("test"));
    sender->send(interproc::to_buffer("test1"));
    sender->send(interproc::to_buffer("test2"));
    sender->close();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    listener->stop();
    listener->wait_until_stopped();
    return 0;
}