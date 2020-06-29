#include "ZmqSocket.h"
#include "logger.h"
#include <errno.h>
#include <iostream>
#include <string.h>
#include <unistd.h> //usleep in some machines
#include <vector>
#include <sstream>
#include <zmq.h>
#include "network_utils.h" //ip 

using namespace rapidjson;
ZmqSocket::ZmqSocket(const char *const hostname_or_ip,
                     const uint32_t portnumber)
    : portno(portnumber), sockfd(false)
{
    // Extra check that throws if conversion fails, could be removed
    auto ipstr = sls::HostnameToIp(hostname_or_ip).str(); 
    std::ostringstream oss;
    oss << "tcp://" << ipstr << ":" << portno;
    sockfd.serverAddress = oss.str();
    LOG(logDEBUG) << "zmq address: " << sockfd.serverAddress;

    // create context
    sockfd.contextDescriptor = zmq_ctx_new();
    if (sockfd.contextDescriptor == nullptr)
        throw sls::ZmqSocketError("Could not create contextDescriptor");

    // create publisher
    sockfd.socketDescriptor = zmq_socket(sockfd.contextDescriptor, ZMQ_SUB);
    if (sockfd.socketDescriptor == nullptr) {
        PrintError();
        throw sls::ZmqSocketError("Could not create socket");
    }

    // Socket Options provided above
    // an empty string implies receiving any messages
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_SUBSCRIBE, "", 0)) {
        PrintError();
        throw sls::ZmqSocketError("Could set socket opt");
    }
    // ZMQ_LINGER default is already -1 means no messages discarded. use this
    // options if optimizing required ZMQ_SNDHWM default is 0 means no limit.
    // use this to optimize if optimizing required eg. int value = -1;
    const int value = 0;
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_LINGER, &value,
                       sizeof(value))) {
        PrintError();
        throw sls::ZmqSocketError("Could not set ZMQ_LINGER");
    }
}

ZmqSocket::ZmqSocket(const uint32_t portnumber, const char *ethip)
    :portno(portnumber), sockfd(true)
{
    // create context
    sockfd.contextDescriptor = zmq_ctx_new();
    if (sockfd.contextDescriptor == nullptr)
        throw sls::ZmqSocketError("Could not create contextDescriptor");

    // create publisher
    sockfd.socketDescriptor = zmq_socket(sockfd.contextDescriptor, ZMQ_PUB);
    if (sockfd.socketDescriptor == nullptr) {
        PrintError();
        throw sls::ZmqSocketError("Could not create socket");
    }

    // construct address, can be refactored with libfmt
    std::ostringstream oss;
    oss << "tcp://" << ethip << ":" << portno;
    sockfd.serverAddress = oss.str();
    LOG(logDEBUG) << "zmq address: " << sockfd.serverAddress;

    // bind address
    if (zmq_bind(sockfd.socketDescriptor, sockfd.serverAddress.c_str())) {
        PrintError();
        throw sls::ZmqSocketError("Could not bind socket");
    }
    // sleep for a few milliseconds to allow a slow-joiner
    usleep(200 * 1000);
};

int ZmqSocket::Connect() {
    if (zmq_connect(sockfd.socketDescriptor, sockfd.serverAddress.c_str())) {
        PrintError();
        return 1;
    }
    return 0;
}

int ZmqSocket::SendHeader(int index, zmqHeader header) {

    /** Json Header Format */
    const char jsonHeaderFormat[] = "{"
                                    "\"jsonversion\":%u, "
                                    "\"bitmode\":%u, "
                                    "\"fileIndex\":%lu, "
                                    "\"detshape\":[%u, %u], "
                                    "\"shape\":[%u, %u], "
                                    "\"size\":%u, "
                                    "\"acqIndex\":%lu, "
                                    "\"frameIndex\":%lu, "
                                    "\"progress\":%u, "
                                    "\"fname\":\"%s\", "
                                    "\"data\": %d, "
                                    "\"completeImage\": %d, "

                                    "\"frameNumber\":%lu, "
                                    "\"expLength\":%u, "
                                    "\"packetNumber\":%u, "
                                    "\"bunchId\":%lu, "
                                    "\"timestamp\":%lu, "
                                    "\"modId\":%u, "
                                    "\"row\":%u, "
                                    "\"column\":%u, "
                                    "\"reserved\":%u, "
                                    "\"debug\":%u, "
                                    "\"roundRNumber\":%u, "
                                    "\"detType\":%u, "
                                    "\"version\":%u, "

                                    // additional stuff
                                    "\"flippedDataX\":%u, "
                                    "\"quad\":%u"

        ; //"}\n";
    memset(header_buffer.get(),'\0',MAX_STR_LENGTH); //TODO! Do we need this
    sprintf(header_buffer.get(), jsonHeaderFormat, header.jsonversion, header.dynamicRange,
            header.fileIndex, header.ndetx, header.ndety, header.npixelsx,
            header.npixelsy, header.imageSize, header.acqIndex,
            header.frameIndex, header.progress, header.fname.c_str(),
            header.data ? 1 : 0, header.completeImage ? 1 : 0,

            header.frameNumber, header.expLength, header.packetNumber,
            header.bunchId, header.timestamp, header.modId, header.row,
            header.column, header.reserved, header.debug, header.roundRNumber,
            header.detType, header.version,

            // additional stuff
            header.flippedDataX, header.quad);

    if (!header.addJsonHeader.empty()) {
        strcat(header_buffer.get(), ", ");
        strcat(header_buffer.get(), "\"addJsonHeader\": {");
        for (auto it = header.addJsonHeader.begin();
             it != header.addJsonHeader.end(); ++it) {
            if (it != header.addJsonHeader.begin()) {
                strcat(header_buffer.get(), ", ");
            }
            strcat(header_buffer.get(), "\"");
            strcat(header_buffer.get(), it->first.c_str());
            strcat(header_buffer.get(), "\":\"");
            strcat(header_buffer.get(), it->second.c_str());
            strcat(header_buffer.get(), "\"");
        }
        strcat(header_buffer.get(), " } ");
    }

    strcat(header_buffer.get(), "}\n");
    int length = strlen(header_buffer.get());

#ifdef VERBOSE
    // if(!index)
    cprintf(BLUE, "%d : Streamer: buf: %s\n", index, buf);
#endif

    if (zmq_send(sockfd.socketDescriptor, header_buffer.get(), length,
                 header.data ? ZMQ_SNDMORE : 0) < 0) {
        PrintError();
        return 0;
    }
#ifdef VERBOSE
    cprintf(GREEN, "[%u] send header data\n", portno);
#endif
    return 1;
}

int ZmqSocket::SendData(char *buf, int length) {
    if (zmq_send(sockfd.socketDescriptor, buf, length, 0) < 0) {
        PrintError();
        return 0;
    }
    return 1;
}

int ZmqSocket::ReceiveHeader(const int index, zmqHeader &zHeader,
                             uint32_t version) {
    const int bytes_received =
        zmq_recv(sockfd.socketDescriptor, header_buffer.get(), MAX_STR_LENGTH, 0);
    if (bytes_received > 0) {
#ifdef ZMQ_DETAIL
        cprintf(BLUE, "Header %d [%d] Length: %d Header:%s \n", index, portno,
                bytes_received, buffer.data());
#endif
        if (ParseHeader(index, bytes_received, header_buffer.get(), zHeader, version)) {
#ifdef ZMQ_DETAIL
            cprintf(RED, "Parsed Header %d [%d] Length: %d Header:%s \n", index,
                    portno, bytes_received, buffer.data());
#endif
            if (!zHeader.data) {
#ifdef ZMQ_DETAIL
                cprintf(RED, "%d [%d] Received end of acquisition\n", index,
                        portno);
#endif
                return 0;
            }
#ifdef ZMQ_DETAIL
            cprintf(GREEN, "%d [%d] data\n", index, portno);
#endif
            return 1;
        }
    }
    return 0;
};

int ZmqSocket::ParseHeader(const int index, int length, char *buff,
                           zmqHeader &zHeader, uint32_t version) {
    Document document;
    if (document.Parse(buff, length).HasParseError()) {
        LOG(logERROR) << index << " Could not parse. len:" << length
                      << ": Message:" << buff;
        for (int i = 0; i < length; ++i) {
            cprintf(RED, "%02x ", buff[i]);
        }
        std::cout << std::endl;
        return 0;
    }

    // version check
    zHeader.jsonversion = document["jsonversion"].GetUint();
    if (zHeader.jsonversion != version) {
        LOG(logERROR) << "version mismatch. required " << version << ", got "
                      << zHeader.jsonversion;
        return 0;
    }

    // parse
    zHeader.data = ((document["data"].GetUint()) == 0) ? false : true;
    zHeader.dynamicRange = document["bitmode"].GetUint();
    zHeader.fileIndex = document["fileIndex"].GetUint64();
    zHeader.ndetx = document["detshape"][0].GetUint();
    zHeader.ndety = document["detshape"][1].GetUint();
    zHeader.npixelsx = document["shape"][0].GetUint();
    zHeader.npixelsy = document["shape"][1].GetUint();
    zHeader.imageSize = document["size"].GetUint();
    zHeader.acqIndex = document["acqIndex"].GetUint64();
    zHeader.frameIndex = document["frameIndex"].GetUint64();
    zHeader.progress = document["progress"].GetUint();
    zHeader.fname = document["fname"].GetString();

    zHeader.frameNumber = document["frameNumber"].GetUint64();
    zHeader.expLength = document["expLength"].GetUint();
    zHeader.packetNumber = document["packetNumber"].GetUint();
    zHeader.bunchId = document["bunchId"].GetUint64();
    zHeader.timestamp = document["timestamp"].GetUint64();
    zHeader.modId = document["modId"].GetUint();
    zHeader.row = document["row"].GetUint();
    zHeader.column = document["column"].GetUint();
    zHeader.reserved = document["reserved"].GetUint();
    zHeader.debug = document["debug"].GetUint();
    zHeader.roundRNumber = document["roundRNumber"].GetUint();
    zHeader.detType = document["detType"].GetUint();
    zHeader.version = document["version"].GetUint();

    zHeader.flippedDataX = document["flippedDataX"].GetUint();
    zHeader.quad = document["quad"].GetUint();
    zHeader.completeImage = document["completeImage"].GetUint();

    if (document.HasMember("addJsonHeader")) {
        const Value &V = document["addJsonHeader"];
        zHeader.addJsonHeader.clear();
        for (Value::ConstMemberIterator iter = V.MemberBegin();
             iter != V.MemberEnd(); ++iter) {
            zHeader.addJsonHeader[iter->name.GetString()] =
                iter->value.GetString();
        }
    }

    return 1;
}

int ZmqSocket::ReceiveData(const int index, char *buf, const int size) {
    zmq_msg_t message;
    zmq_msg_init(&message);
    int length = ReceiveMessage(index, message);
    if (length == size) {
        memcpy(buf, (char *)zmq_msg_data(&message), size);
    } else if (length < size) {
        memcpy(buf, (char *)zmq_msg_data(&message), length);
        memset(buf + length, 0xFF, size - length);
    } else {
        LOG(logERROR) << "Received weird packet size " << length
                      << " for socket " << index;
        memset(buf, 0xFF, size);
    }

    zmq_msg_close(&message);
    return length;
}

int ZmqSocket::ReceiveMessage(const int index, zmq_msg_t &message) {
    int length = zmq_msg_recv(&message, sockfd.socketDescriptor, 0);
    if (length == -1) {
        PrintError();
        LOG(logERROR) << "Could not read header for socket " << index;
    }
    return length;
}

void ZmqSocket::PrintError() {
    switch (errno) {
    case EINVAL:
        LOG(logERROR) << "The socket type/option or value/endpoint supplied is "
                         "invalid (zmq)";
        break;
    case EAGAIN:
        LOG(logERROR) << "Non-blocking mode was requested and the message "
                         "cannot be sent/available at the moment (zmq)";
        break;
    case ENOTSUP:
        LOG(logERROR) << "The zmq_send()/zmq_msg_recv() operation is not "
                         "supported by this socket type (zmq)";
        break;
    case EFSM:
        LOG(logERROR) << "The zmq_send()/zmq_msg_recv() unavailable now as "
                         "socket in inappropriate state (eg. ZMQ_REP). Look up "
                         "messaging patterns (zmq)";
        break;
    case EFAULT:
        LOG(logERROR) << "The provided context/message is invalid (zmq)";
        break;
    case EMFILE:
        LOG(logERROR) << "The limit on the total number of open ØMQ sockets "
                         "has been reached (zmq)";
        break;
    case EPROTONOSUPPORT:
        LOG(logERROR)
            << "The requested transport protocol is not supported (zmq)";
        break;
    case ENOCOMPATPROTO:
        LOG(logERROR) << "The requested transport protocol is not compatible "
                         "with the socket type (zmq)";
        break;
    case EADDRINUSE:
        LOG(logERROR) << "The requested address is already in use (zmq)";
        break;
    case EADDRNOTAVAIL:
        LOG(logERROR) << "The requested address was not local (zmq)";
        break;
    case ENODEV:
        LOG(logERROR)
            << "The requested address specifies a nonexistent interface (zmq)";
        break;
    case ETERM:
        LOG(logERROR) << "The ØMQ context associated with the specified socket "
                         "was terminated (zmq)";
        break;
    case ENOTSOCK:
        LOG(logERROR) << "The provided socket was invalid (zmq)";
        break;
    case EINTR:
        LOG(logERROR)
            << "The operation was interrupted by delivery of a signal (zmq)";
        break;
    case EMTHREAD:
        LOG(logERROR)
            << "No I/O thread is available to accomplish the task (zmq)";
        break;
    default:
        LOG(logERROR) << "Unknown socket error (zmq)";
        break;
    }
}

// Nested class to do RAII handling of socket descriptors
ZmqSocket::mySocketDescriptors::mySocketDescriptors(bool server)
    : server(server), contextDescriptor(nullptr), socketDescriptor(nullptr){};
ZmqSocket::mySocketDescriptors::~mySocketDescriptors() {
    Disconnect();
    Close();
}
void ZmqSocket::mySocketDescriptors::Disconnect() {
    if (server)
        zmq_unbind(socketDescriptor, serverAddress.c_str());
    else
        zmq_disconnect(socketDescriptor, serverAddress.c_str());
};
void ZmqSocket::mySocketDescriptors::Close() {
    if (socketDescriptor != nullptr) {
        zmq_close(socketDescriptor);
        socketDescriptor = nullptr;
    }

    if (contextDescriptor != nullptr) {
        zmq_ctx_destroy(contextDescriptor);
        contextDescriptor = nullptr;
    }
};
