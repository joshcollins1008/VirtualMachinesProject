#include "memory/iterator.hpp"

class OopCrawlerClosure : public OopClosure {
public:
  void do_oop( oop* o );
  void do_oop( narrowOop* o );
};
