#ifndef SHARE_VM_GC_INTERFACE_METHOD_INFO_HPP
#define SHARE_VM_GC_INTERFACE_METHOD_INFO_HPP

#include "oops/method.hpp"
#include "ci/ciSymbol.hpp"

class MethodInfoManager;

class MethodInfoAccessList : public CHeapObj<mtInternal> {
private:
  GrowableArray<const Symbol*>* _access_list;

  void print_helper(const Symbol* sym) const;

public:
  MethodInfoAccessList();
  virtual ~MethodInfoAccessList();

  void addAccess(const ciSymbol* access);

  void doAccesses(void (*f)(const Symbol*)) const;

  void print() const;
};

class MethodInfo : public CHeapObj<mtInternal> {
  friend MethodInfoManager;

private:
  const Symbol* _klass;
  const Symbol* _method;
  const Symbol* _signature;

  const MethodInfoAccessList* _access_list;

  MethodInfo(const ciSymbol* klass, const ciSymbol* method, const ciSymbol* signature, const MethodInfoAccessList* access_list);
  ~MethodInfo();

public:

  const Symbol* getKlass() const      { return _klass; }
  const Symbol* getMethod() const     { return _method; }
  const Symbol* getSignature() const  { return _signature; }

  const MethodInfoAccessList* getAccesses() const { return _access_list; }

  const Method* getMethodReal() const;

  const void print() const;
  const void print2() const;
};

class MethodInfoManager : public AllStatic {
private:
  static GrowableArray<const MethodInfo*>* _infos;

public:
  /* Construct a new MethodInfo entry and assign it a list of accesses */
  static const MethodInfo* make(const ciSymbol* klass, const ciSymbol* method, const ciSymbol* signature, const MethodInfoAccessList* access_list);

  /* Search based on ciSymbols for a method's MethodInfo entry */
  static const MethodInfo* getMethodInfo(const ciSymbol* klass, const ciSymbol* method, const ciSymbol* signature);

  /* Search based on Symbols for a method's MethodInfo entry */
  static const MethodInfo* getMethodInfo(const Symbol* klass, const Symbol* method, const Symbol* signature);

  /* Get a real Symbol from a ciSymbol */
  static const Symbol* resolve_ciSymbol(const ciSymbol* sym);

  /* DEBUG - print all MethodInfo's in the _infos field */
  static void printAll();
};

#endif // SHARE_VM_GC_INTERFACE_METHOD_INFO_HPP
