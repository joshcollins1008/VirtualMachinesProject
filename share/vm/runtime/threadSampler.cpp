#include "precompiled.hpp"
#include "runtime/task.hpp"

#include "runtime/threadSampler.hpp"

// VM Project - below here
GrowableArray<CountedSymbol*>* HotKlassList::klasses = new (ResourceObj::C_HEAP, mtInternal) GrowableArray<CountedSymbol*>(32, true);

int HotKlassList::add_count = 0;

// ThreadSampler is used by HotMethodSampler in its call to threads_do
class ThreadSampler : public ThreadClosure {
public:
  // Mark access to the top nmethod on "thread's" stack
  void do_thread(Thread* thread) {
    if (thread->is_Java_thread()) {
      JavaThread* jthread = (JavaThread*) thread;

      if (jthread->has_last_Java_frame()) {
        RegisterMap rm(jthread);
        javaVFrame* vf;

        // Look for the highest java vframe on the stack
	for (vf = jthread->last_java_vframe(&rm); vf && !vf->is_java_frame(); vf->sender());

        // Make sure the found frame corresponds to a nmethod
        if (vf->is_java_frame() && vf->cb()->is_nmethod()) {
          nmethod* nm = vf->nm();

          assert(nm != NULL, "Bad NMethod");

          Method* m = nm->method();

          // Get method descriptors
          Symbol* klass = m->klass_name();
          Symbol* method = m->name();
          Symbol* signature = m->signature();

          // Look up the MethodInfo corresponding to this method
          const MethodInfo* mi = MethodInfoManager::getMethodInfo(klass, method, signature);

          // Some nmethods never go through the compiler. I don't think it is possible
          // to get access records for those methods, so I just ignore them.
          // The missed methods seem to be centered around the core java.lang and
          // java.io functions. My guess is that methods that fall into this category
	  // are either implemented in the VM or are C/C++ functions called by Java.
          if (mi)
            mi->getAccesses()->doAccesses(&HotKlassList::addKlass);
        }
      }
    }
  }
};

class HeapInstanceCounter : public ObjectClosure {
private:
  const Symbol* _klass;

  long _count;
  long _size;

public:
  HeapInstanceCounter(const Symbol* klass) : _klass(klass), _count(0), _size(0) {}

  void do_object(oop obj) {
    if (obj->klass()->name() == _klass) {
      _count++;
      _size += obj->size();
    }
  }

  long getCount() const {
    return _count;
  }

  long getSize() const {
    return _size;
  }
};

class HeapOopCounter : public ObjectClosure {
private:
  const Symbol* _klass;

  long _count;
  long _size;

public:
  HeapOopCounter() : _count(0), _size(0) {}

  void do_object(oop obj) {
    long prev_count = _count;
    long prev_size = _size;

    _count++;
    _size += obj->size();

    if (prev_count > _count || prev_size > _size)
      tty->print_cr("Overflow");
  }

  long getCount() const {
    return _count;
  }

  long getSize() const {
    return _size;
  }
};

class HotMethodSamplerTask : public PeriodicTask {
public:
  HotMethodSamplerTask(size_t interval_time) : PeriodicTask(interval_time) {}
  void task() {
    VM_HotMethodSampler vmop;
    VMThread::execute(&vmop);
  }
};

class HotFieldCollectorTask : public PeriodicTask {
private:
  static int _count;

public:
  HotFieldCollectorTask(size_t interval_time) : PeriodicTask(interval_time) {}

  void task() {
    VM_HotFieldCollector vmop;
    tty->print_cr("Collection Interval %d", ++_count);
    VMThread::execute(&vmop);
  }
};

void HotKlassList::addKlass(const Symbol* klass) {
  // Register this addition
  add_count++;

  // Check if "klass" is already in the set
  for (int i = 0; i < klasses->length(); i++) {
    if (klass == klasses->at(i)->getSymbol()) {
      klasses->at(i)->increment();
      return;
    }
  }

  // Add "klass" to the set
  klasses->push(new CountedSymbol(klass));
}

// Call a function on each Symbol in the klasses array
void HotKlassList::doKlasses(void (*f)(const Symbol* klass)) {
  for (int i = 0; i < klasses->length(); i++) {
    (*f)(klasses->at(i)->getSymbol());
  }
}

void HotKlassList::flush() {
  add_count = 0;

  for (int i = 0; i < klasses->length(); i++) {
    delete klasses->at(i);
  }

  klasses->clear();
}

void HotKlassList::sort() {
  klasses->sort(CountedSymbol::compare);
}

GrowableArray<const Symbol*> HotKlassList::matchHeapOopsWithHotKlasses(size_t cutoff) {
  GrowableArray<const Symbol*> hot_klasses(32, false);

  long hot_count = 0;
  long live_count = 0;
  long hot_size = 0;
  long live_size = 0;

  HeapOopCounter hoc;

  // Sort the classes so that we may deal with them in a most
  // used class first order.
  if (cutoff > 0)
    sort();

  // Measure the heap
  Universe::heap()->ensure_parsability(false);
  Universe::heap()->object_iterate(&hoc);

  live_count = hoc.getCount();
  live_size = hoc.getSize();

  // Measure the hot objects on the heap
  for (int i = 0; i < klasses->length(); i++) {
    const Symbol* sym = klasses->at(i)->getSymbol();
    HeapInstanceCounter hic(sym);

    // Ensure again and iterate over all objects on the heap
    Universe::heap()->ensure_parsability(false);
    Universe::heap()->object_iterate(&hic);

    // Add classes to the hot classes list ignoring those that don't fit.
    // If zero is specified as the cutoff then we add the class to the list
    // without question.
    if (((size_t)(hot_size + hic.getSize())) <= cutoff || cutoff == 0) {
#if 1
      tty->print_cr("%s --- %d", sym->as_utf8(), klasses->at(i)->getCount());
      tty->print_cr("   count: %7ld, %6.2f%%", hic.getCount(), ((float) hic.getCount())/hoc.getCount()*100);
      tty->print_cr("   size:  %7ld, %6.2f%%", hic.getSize(), ((float) hic.getSize())/hoc.getSize()*100);
#endif

      hot_count += hic.getCount();
      hot_size += hic.getSize();

      // Add the klass to the list
      hot_klasses.push(sym);
    }
  }

  tty->print_cr("      |    Hot    |    Live   | Hot / Live");
  tty->print_cr("-------------------------------------------");
  tty->print_cr("Count | %9ld | %9ld | %8.4f%%", hot_count, live_count, ((float) hot_count)/live_count * 100);
  tty->print_cr("Size  | %9ld | %9ld | %8.4f%%", hot_size, live_size, ((float) hot_size)/live_size * 100);
  tty->cr();

  return hot_klasses;
}

void HotKlassList::print() {
  for (int i = 0; i < klasses->length(); i++) {
    tty->print_cr("\t%s", klasses->at(i)->getSymbol()->as_utf8());
  }
}

int HotFieldCollectorTask::_count = 0;

HotMethodSamplerTask* HotMethodSamplerTaskManager::_task = NULL;
HotFieldCollectorTask* HotFieldCollectorTaskManager::_task = NULL;

// HotMethodSampler vm operation implementation
VM_HotMethodSampler::VM_HotMethodSampler() {}
VM_HotMethodSampler::~VM_HotMethodSampler() {}

VM_HotMethodSampler::Mode VM_HotMethodSampler::evaluation_mode() const {
  return _safepoint;
}

// HotMethodSampler's action - Scans each thread for hot methods
void VM_HotMethodSampler::doit() {
  ThreadSampler ts;
  Threads::threads_do(&ts);
}

// HotFieldCollector vm operation implementation
VM_HotFieldCollector::VM_HotFieldCollector() {}
VM_HotFieldCollector::~VM_HotFieldCollector() {}

VM_HotFieldCollector::Mode VM_HotFieldCollector::evaluation_mode() const {
  return _safepoint;
}

// HotFieldCollector's action - Currently it only prints and then
// empties the array
void VM_HotFieldCollector::doit() {
  HotKlassList::matchHeapOopsWithHotKlasses(500000);
  HotKlassList::flush();
}

// HotMethodSamplerTaskManager - Wrapper for the corresponding
// PeriodicTask subclass
void HotMethodSamplerTaskManager::engage(size_t interval_time) {
  _task = new HotMethodSamplerTask(interval_time);
  _task->enroll();
}

void HotMethodSamplerTaskManager::disengage() {
  _task->disenroll();
  delete _task;
  _task = NULL;
}

// HotMethodSamplerTaskManager - Wrapper for the corresponding
// PeriodicTask subclass
void HotFieldCollectorTaskManager::engage(size_t interval_time) {
  _task = new HotFieldCollectorTask(interval_time);
  _task->enroll();
}

void HotFieldCollectorTaskManager::disengage() {
  _task->disenroll();
  delete _task;
  _task = NULL;
}
