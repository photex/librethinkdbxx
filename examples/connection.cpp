#include <iostream>
#include <memory>
#include <rethinkdb.h>

namespace R = RethinkDB;

int main(int argc, char *argv[]) {
    std::unique_ptr<R::Connection> conn = R::connect("localhost", 28015);
    // TODO: Create table first?
    try {
        R::Cursor cursor = R::table("authors").filter(R::row["age"] > 14).run(*conn);
        for (R::Datum& user : cursor) {
            std::cout << R::write_datum(user).c_str() << std::endl;
        }
    } catch (const RethinkDB::Error &err) {
        std::cerr << err.message << std::endl;
        return 1;
    }
    return 0;
}
