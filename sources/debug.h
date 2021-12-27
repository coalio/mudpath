#define DEBUG_MODE 1
#if DEBUG_MODE
#define LINENUM 1
// print line number
#if LINENUM == 1
    #define DEBUG(msg) std::cerr << "[" << __LINE__ << "] " << msg << std::endl;
#else
    #define DEBUG(msg) std::cerr << msg << std::endl;
#endif
#else
#define DEBUG(msg)
#endif