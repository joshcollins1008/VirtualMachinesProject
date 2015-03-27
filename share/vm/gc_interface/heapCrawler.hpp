#ifndef SHARE_VM_GC_INTERFACE_HEAPCRAWLER_HPP
#define SHARE_VM_GC_INTERFACE_HEAPCRAWLER_HPP


#include "memory/iterator.hpp"

class OopCrawlerClosure : public OopClosure {
public:
  void do_oop( oop* o );
  void do_oop( narrowOop* o );
};

#endif
