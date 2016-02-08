#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>

#include "protocol_defs.h"
#include "datum.h"
#include "error.h"

namespace RethinkDB {

class Token;
class Connection;
class ResponseBuffer;

// Used internally to convert a raw response type into an enum
Protocol::Response::ResponseType response_type(double t);

// Contains a response from the server. Use the Cursor class to interact with these responses
class Response {
public:
    Response(Datum&& datum) :
        type(response_type(std::move(datum).extract_field("t").extract_number())),
        result(std::move(datum).extract_field("r").extract_array()) { }
    Error as_error();
    Protocol::Response::ResponseType type;
    Array result;
};

// A connection to a RethinkDB server
// It contains:
//  * A socket
//  * Read and write locks
//  * A cache of responses that have not been read by the corresponding Cursor
class Connection {
public:
    Connection(const std::string& host, int port, const std::string& auth_key);

    Connection(const Connection&) = delete;
    Connection(Connection&&) = default;

    // Used internally by Query::run
    Token start_query(const std::string&);

    void close();

private:
    Response wait_for_response(uint64_t);
    void close_token(uint64_t);
    void ask_for_more(uint64_t);

    friend class ResponseBuffer;
    friend class Token;

    const uint32_t version_magic = static_cast<uint32_t>(Protocol::VersionDummy::Version::V0_4);
    const uint32_t json_magic = static_cast<uint32_t>(Protocol::VersionDummy::Protocol::JSON);

    class ReadLock;
    class WriteLock;
    class CacheLock;

    std::mutex read_lock;
    std::mutex write_lock;
    std::mutex cache_lock;

    struct TokenCache {
        bool closed = false;
        std::condition_variable cond;
        std::queue<Response> responses;
    };

    std::map<uint64_t, TokenCache> guarded_cache;
    uint64_t guarded_next_token;
    int guarded_sockfd;
    bool guarded_loop_active;
};

// $doc(connect)
std::unique_ptr<Connection> connect(std::string host = "localhost", int port = 28015, std::string auth_key = "");

// Each server response is associated with a token, which allows multiplexing streams
// A token can be used to create a cursor
class Token {
public:
    Token(Connection::WriteLock&);

    Token(const Token&) = delete;
    Token(Token&& other) : conn(other.conn), token(other.token) {
        other.conn = NULL;
    }

    void ask_for_more() const {
        conn->ask_for_more(token);
    }

    Response wait_for_response() const {
        return conn->wait_for_response(token);
    }

    void close() const {
        if(conn) {
            conn->close_token(token);
        }
    }

    ~Token() {
        close();
    }
private:
    friend class Connection;
    Connection* conn;
    uint64_t token;
};

}
