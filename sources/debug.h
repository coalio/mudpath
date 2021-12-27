#define DEBUG_MODE 1
#define PRINT_INIT 1
#if DEBUG_MODE
#define LINENUM 0
// print line number
#if LINENUM
    #define DEBUG(msg) std::cerr << "[" << __LINE__ << "] " << msg << std::endl;
#else
    #define DEBUG(msg) std::cerr << msg << std::endl;
#endif
#else
#define DEBUG(msg)
#endif