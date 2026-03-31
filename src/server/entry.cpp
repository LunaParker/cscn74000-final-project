#include "main.h"

// NOLINTNEXTLINE(bugprone-exception-escape) — std::string in handlers may throw; acceptable for ATS scaffolding
int main() {
    return CSCN_74000_SERVER::runServer();
}

