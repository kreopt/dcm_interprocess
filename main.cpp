#include "interprocess/listener_factory.hpp"
#include "interprocess/sender_factory.hpp"
#include <chrono>
int main(){
    using namespace std::chrono_literals;

    auto listener = interproc::make_listener<>("ipc://test.sock");
    auto ep = interproc::make_sender<>("ipc://test.sock");
    auto sender = std::make_shared<interproc::sender<>>();

    auto buf = interproc::buffer(new char[1920*1080*3], 1920*1080*3);

    listener->on_message = [](interproc::buffer &&_buf){
        interproc::Log::d("received");
    };

    listener->start();
    sender->connect();
    auto eps = std::vector<interproc::endpoint<>::ptr>({ep});
    for (int i=0; i< 10/*000*/; i++) {
        sender->send(eps, buf);
    }
    sender->close();
    ep->close();
    listener->stop();
    listener->wait_until_stopped();
    return 0;
}