#include "precompiled.hpp"
#include <cstring>

#include "utilities/quickSort.hpp"
#include "oops/oop.hpp"

#include "gc_implementation/parallelScavenge/oopReorder.hpp"

class CollectionClosure : public OopClosure {
private:
  GrowableArray<oop*>* _store;
  GrowableArray<narrowOop*>* _narrow_store;

public:
  CollectionClosure(GrowableArray<oop*>* store, GrowableArray<narrowOop*>* narrow_store) : _store(store), _narrow_store(narrow_store) {}

  void do_oop(oop* p) {
    if ((*p)->is_oop())
      _store->push(p);
  }
  
  void do_oop(narrowOop* p) {
    _narrow_store->push(p);
  }
};

class ObjectCollectionClosure : public ObjectClosure {
private:
  GrowableArray<oop>* _store;
  GrowableArray<narrowOop*>* _narrow_store;

public:
  ObjectCollectionClosure(GrowableArray<oop>* store, GrowableArray<narrowOop*>* narrow_store) : _store(store), _narrow_store(narrow_store) {}

  void do_object(oop obj) {
    if (obj->is_oop())
      _store->push(obj);
  }
};

int jrcompare(oop** a, oop** b) {
  const char* a_str = (**a)->klass()->name()->as_utf8();
  const char* b_str = (**b)->klass()->name()->as_utf8();
  
  int cmp = std::strcmp(a_str, b_str);
  
  if (cmp > 0)
    return 1;
  else if (cmp < 0)
    return -1;
  else
    return 0;
}

void OopReorder::sort(GrowableArray<oop*>* collection) {
  ResourceMark rm;

  collection->sort(jrcompare);
}

void OopReorder::oops_do_interceptor0(void (*oops_do)(OopClosure*), OopClosure* mark_and_push) {
  ResourceMark rm;

  GrowableArray<oop*> arr(512);
  GrowableArray<narrowOop*> narr(512);

  CollectionClosure collector(&arr, &narr);

  (*oops_do)(&collector);

  sort(&arr);

  for (int i = 0; i < narr.length(); i++)
    mark_and_push->do_oop(narr.at(i));

  for (int i = 0; i < arr.length(); i++)
    mark_and_push->do_oop(arr.at(i));
}

void OopReorder::oops_do_interceptor1(void (*oops_do)(OopClosure*, bool), OopClosure* mark_and_push) {
  ResourceMark rm;

  GrowableArray<oop*> arr(512);
  GrowableArray<narrowOop*> narr(512);

  CollectionClosure collector(&arr, &narr);

  (*oops_do)(&collector, false);

  sort(&arr);

  for (int i = 0; i < narr.length(); i++)
    mark_and_push->do_oop(narr.at(i));

  for (int i = 0; i < arr.length(); i++)
    mark_and_push->do_oop(arr.at(i));
}

void OopReorder::oops_do_interceptor2(void (*oops_do)(OopClosure*), OopClosure* mark_and_push, OopClosure* follow_class) {

}

void OopReorder::oops_do_interceptor3(void (*oops_do)(OopClosure*), OopClosure* mark_and_push, OopClosure* mark_and_push_cld, OopClosure* each_active_code_blob) {

}

void OopReorder::print_verification0(void (*oops_do)(OopClosure*)) {
  ResourceMark rm;

  GrowableArray<oop*> arr(512);
  GrowableArray<narrowOop*> narr(512);

  CollectionClosure collector(&arr, &narr);
  
  (*oops_do)(&collector);

  for (int i = 0; i < arr.length(); i++)
    tty->print_cr("%4d: %s", i, (*arr.at(i))->klass()->name()->as_utf8());
}

void OopReorder::print_verification1(void (*oops_do)(OopClosure*, bool)) {
  ResourceMark rm;

  GrowableArray<oop*> arr(512);
  GrowableArray<narrowOop*> narr(512);

  CollectionClosure collector(&arr, &narr);
  
  (*oops_do)(&collector, false);

  for (int i = 0; i < arr.length(); i++)
    tty->print_cr("%4d: %s", i, (*arr.at(i))->klass()->name()->as_utf8());
}


void OopReorder::print_verification2() {
  ResourceMark rm;

  GrowableArray<oop> arr(100000);
  GrowableArray<narrowOop*> narr(512);

  ObjectCollectionClosure collector(&arr, &narr);
  
  Universe::heap()->ensure_parsability(false);
  Universe::heap()->object_iterate(&collector);

  tty->print_cr("%d", arr.length());

  //for (int i = 0; i < arr.length(); i++)
  //  tty->print_cr("%4d: %s", i, arr.at(i)->klass()->name()->as_utf8());
}
