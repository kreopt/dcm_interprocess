#include "interprocess/listener_factory.hpp"
#include "interprocess/sender_factory.hpp"

int main(){

    auto listener = interproc::make_listener<interproc::buffer>(interproc::conn_type::unix, "test.sock");
    auto sender = interproc::make_sender<interproc::buffer>(interproc::conn_type::unix, "test.sock");

    listener->start();
    sender->connect();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    sender->send("test");
    sender->close();

    listener->wait_until_stopped();
    return 0;
}