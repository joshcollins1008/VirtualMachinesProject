#ifndef SHARE_VM_GC_IMPLEMENTATION_PARALLELSCAVENGE_OOPREORDER_HPP
#define SHARE_VM_GC_IMPLEMENTATION_PARALLELSCAVENGE_OOPREORDER_HPP

#include "precompiled.hpp"

class OopReorder : public AllStatic {
private:
  static void quicksort(GrowableArray<oop*>* collection, int start, int end, int pivot);

  static void sort(GrowableArray<oop*>* collection);
  static void sort_narrow(GrowableArray<narrowOop*>* collection);

public:
  static void oops_do_interceptor1(void (*oops_do)(OopClosure*, bool), OopClosure* mark_and_push);
  static void oops_do_interceptor2(void (*oops_do)(OopClosure*), OopClosure* mark_and_push, OopClosure* follow_class);
  static void oops_do_interceptor3(void (*oops_do)(OopClosure*), OopClosure* mark_and_push, OopClosure* makr_and_push_cld, OopClosure* each_active_code_blob);
};

#endif //SHARE_VM_GC_IMPLEMENTATION_PARALLELSCAVENGE_OOPREORDER_HPP
