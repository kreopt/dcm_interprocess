#include "dcm/dcm.hpp"
#include <binelpro/log.hpp>
#include <chrono>

using bp::Log;
int main(){
    using namespace std::chrono_literals;

    auto listener = dcm::make_listener<>("ipc://test.sock");
    auto sender = dcm::make_p2p_sender<>("ipc://test.sock");

    auto buf = dcm::buffer(new char[1920*1080*3], 1920*1080*3);

    listener->on_message = [](dcm::buffer &&_buf){
        Log::d("received");
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