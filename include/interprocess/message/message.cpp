#include <chrono>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include "message.hpp"


dcm::message::message(const interproc::buffer &_encoded) {
    decode(_encoded);
}

dcm::message::message(interproc::buffer &&_encoded) {
    decode(_encoded);
}

dcm::message::message() {

}

interproc::buffer dcm::message::encode_block(const dcm::message::block_t &_block) const {
    interproc::obufstream ba;
    interproc::write_size(ba, 0);
    interproc::block_size_t block_size = 0;
    for (auto &b: _block){
        interproc::write_size(ba, b.first.size());
        ba << b.first;
        interproc::write_size(ba, b.second.size());
        ba << b.second;
        block_size += b.first.size()+b.second.size()+2*sizeof(interproc::block_size_t);
    }
    auto pos = ba.tellp();
    ba.seekp(0);
    interproc::write_size(ba, block_size);
    ba.seekp(pos);
    return ba.str();
}

interproc::buffer dcm::message::encode() const {
    return encode_block(header)+encode_block(body);
}

void dcm::message::decode_header(const interproc::buffer &_encoded) {
    decode_block(_encoded, header);
}

void dcm::message::decode_body(const interproc::buffer &_encoded) {
    decode_block(_encoded, body);
}

void dcm::message::decode_block(const interproc::buffer &_buf, std::unordered_map<std::string, interproc::buffer> &_container) {
    std::string key;
    interproc::buffer val;
    interproc::ibufstream bs(_buf);
    interproc::block_size_t len;
    int pos = 0;
    while (pos < _buf.size()){
        interproc::read_size(bs, len);
        key.resize(len);
        bs.read(&key[0], len);
        pos+=len;

        interproc::read_size(bs, len);
        val.resize(len);
        bs.read(&val[0], len);
        pos+=len;

        _container[key] = val;
        pos+=2*interproc::BLOCK_SIZE_SIZE;
    }
}

void dcm::message::decode(const interproc::buffer &_encoded) {
    interproc::ibufstream bs(_encoded);
    interproc::block_size_t len = 0;
    interproc::read_size(bs, len);
    decode_header(interproc::buffer(_encoded, interproc::BLOCK_SIZE_SIZE, len));

    interproc::ibufstream bbs(interproc::buffer(_encoded, interproc::BLOCK_SIZE_SIZE+len));
    interproc::block_size_t len1 = 0;
    interproc::read_size(bbs, len1);
    decode_body(interproc::buffer(_encoded.begin()+2*interproc::BLOCK_SIZE_SIZE+len, _encoded.end()));
}

dcm::message::message(dcm::message &&_message) {
    this->header = std::move(_message.header);
    _message.header.clear();
    this->body = std::move(_message.body);
    _message.body.clear();
}

interproc::buffer dcm::message::encode_header() const {
    return encode_block(header);
}

interproc::buffer dcm::message::encode_body() const {
    return encode_block(body);
}

dcm::message::message(dcm::message::block_t &&_header, dcm::message::block_t &&_body) {
    header = _header;
    body = _body;
}
