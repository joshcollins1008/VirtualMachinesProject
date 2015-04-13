#ifndef SHARE_VM_RUNTIME_HEAP_SAMPLER_HPP
#define SHARE_VM_RUNTIME_HEAP_SAMPLER_HPP

#include "precompiled.hpp"
#include "runtime/vm_operations.hpp"

// VM Project - below here
class HotMethodSamplerTask;
class HotFieldCollectorTask;

// Allow for symbol counting
class CountedSymbol : public CHeapObj<mtInternal> {
private:
  const Symbol* _sym;
  int _count;

public:
  CountedSymbol(const Symbol* sym) : _sym(sym), _count(1) {}

  const Symbol* getSymbol() const { return _sym; }
  int getCount() const            { return _count; }

  void increment()                { _count++; }

  static int compare(CountedSymbol** l, CountedSymbol** r) {
    return (*r)->getCount() - (*l)->getCount();
  }
};

// Container for hot classes
class HotKlassList : public AllStatic {
private:
  static GrowableArray<CountedSymbol*>* klasses;
  static int add_count;

public:
  static void doKlasses(void (*f)(const Symbol* klass));

  static void addKlass(const Symbol* klass);
  static void flush();

  static void sort();

  static GrowableArray<const Symbol*> matchHeapOopsWithHotKlasses(size_t cutoff);
  static void print();
};

// This VM operation should collect the top method in all threads
class VM_HotMethodSampler: public VM_Operation {
public:
  VM_HotMethodSampler();
  virtual ~VM_HotMethodSampler();

  // Do the sampling
  virtual void doit();

  // Configuration. Override these appropriatly in subclasses.
  virtual VMOp_Type type() const                  { return VMOp_HotMethodSampler; }
  virtual Mode evaluation_mode() const;
};

class VM_HotFieldCollector: public VM_Operation {
public:
  VM_HotFieldCollector();
  virtual ~VM_HotFieldCollector();

  // Do the collecting
  virtual void doit();

  // Configuration. Override these appropriatly in subclasses.
  virtual VMOp_Type type() const                  { return VMOp_HotFieldCollector; }
  virtual Mode evaluation_mode() const;
};

class HotMethodSamplerTaskManager : public AllStatic {
private:
  static HotMethodSamplerTask* _task;

public:
  static void engage(size_t interval_time);
  static void disengage();
};

class HotFieldCollectorTaskManager : public AllStatic {
private:
  static HotFieldCollectorTask* _task;

public:
  static void engage(size_t interval_time);
  static void disengage();
};

#endif // SHARE_VM_RUNTIME_HEAP_SAMPLER_HPP
