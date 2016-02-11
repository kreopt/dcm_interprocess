#include "dcm/dcm.hpp"
#include <binelpro/log.hpp>
#include <chrono>

using bp::Log;
int main(){
    using namespace std::chrono_literals;

    auto listener = dcm::make_listener<>("p2pipc://test.sock");
    auto sender = dcm::make_sender<>("p2pipc://test.sock");

    auto buf = dcm::buffer(new char[1920*1080*3], 1920*1080*3);

    listener->on_message.set_default([](const typename dcm::session<dcm::buffer>::ptr &_sess, dcm::buffer &&_buf){
        Log::d("received");
    });

    sender->on_connect.set_default([](){
        Log::d("connected");
    });
    sender->connect();
    for (int i=0; i< 10; i++) {
        sender->send(dcm::buffer(buf));

    }
    listener->start();

    sender->close(true);
    listener->stop();
    listener->wait_until_stopped();
    return 0;
}